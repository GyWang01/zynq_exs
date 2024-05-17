#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

struct mpu_mouse_data {
	struct i2c_client *client;
	unsigned int io_down;
	struct input_dev *input_dev;
	struct timer_list poll_timer;
	struct work_struct poll_work;
	int prev_accel_x;
	int prev_accel_y;
	int prev_button_state;
};

#define FILTER_ALPHA 70  //  0.7 * FILTER_SCALE
#define FILTER_SCALE 100
#define ACCEL_SCALE (1 << 14) // 16384 for Q15 format
#define AMPLIFY 30

static int low_pass_filter(int prev_value, int current_value)
{
	//y[n]=α⋅x[n]+(1−α)⋅y[n−1]
	return (FILTER_ALPHA * current_value + (FILTER_SCALE - FILTER_ALPHA) * prev_value) / FILTER_SCALE;
}

static void mpu_mouse_poll_work(struct work_struct *work)
{
	struct mpu_mouse_data *mydevice = container_of(work, struct mpu_mouse_data, poll_work);
	int accel_x, accel_y;
	int16_t raw_accel_x, raw_accel_y;	// 保留符号
	int filtered_x, filtered_y;
	int button_state;
	uint8_t buf[6];
	
    // 读陀螺仪数据
    i2c_smbus_read_i2c_block_data(mydevice->client, 0x3D, 2, buf);
    raw_accel_x = (int16_t)((buf[0] << 8) | buf[1]);  
    i2c_smbus_read_i2c_block_data(mydevice->client, 0x3B, 2, buf);
    raw_accel_y = (int16_t)((buf[0] << 8) | buf[1]);

	accel_x = -raw_accel_x * AMPLIFY / ACCEL_SCALE; 
    accel_y = raw_accel_y * AMPLIFY / ACCEL_SCALE; 

	// 低通滤波
	filtered_x = low_pass_filter(mydevice->prev_accel_x, accel_x);
	filtered_y = low_pass_filter(mydevice->prev_accel_y, accel_y);

	mydevice->prev_accel_x = filtered_x;
	mydevice->prev_accel_y = filtered_y;
	
	// 上报鼠标位移事件
	input_report_rel(mydevice->input_dev, REL_X, filtered_x); 
	input_report_rel(mydevice->input_dev, REL_Y, filtered_y); 
	
	// 读按键、上报按键
	button_state = !gpio_get_value(mydevice->io_down);	// gpio按键的逻辑是反的
	if (button_state != mydevice->prev_button_state) {
		input_report_key(mydevice->input_dev, BTN_LEFT, button_state);
		mydevice->prev_button_state = button_state;
	}
	// 同步事件
	input_sync(mydevice->input_dev);
}

static void mpu_mouse_timer_callback(struct timer_list *t)
{
	struct mpu_mouse_data *mydevice = from_timer(mydevice, t, poll_timer);
	schedule_work(&mydevice->poll_work);
	mod_timer(&mydevice->poll_timer, jiffies + msecs_to_jiffies(20)); // 20ms回调
}

static int mpu_mouse_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int error;
	struct device_node *np = client->dev.of_node;
	struct mpu_mouse_data *mydevice;

	mydevice = devm_kzalloc(&client->dev, sizeof(*mydevice), GFP_KERNEL);
	if (!mydevice) {
		dev_err(&client->dev, "No memory\n");
		return -ENOMEM;
	}
	
	mydevice->client = client;
	i2c_set_clientdata(client, mydevice);//将mydevice与client关联起来，以便后续可以通过client访问mydevice
	
	dev_alert(&client->dev, "MPU Address: 0x%02x\n", client->addr);
	mydevice->io_down = of_get_named_gpio(np, "io_down", 0);
	pr_err("gpio=%d\n", mydevice->io_down);
	if (gpio_is_valid(mydevice->io_down)) {
		error = devm_gpio_request_one(&client->dev, mydevice->io_down, GPIOF_IN, "mpu_gpio");
		if (error) {
			dev_err(&client->dev, "Failed to request GPIO\n");
			return error;
		}
	}
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
		dev_err(&client->dev, "I2C check functionality failed\n");
		return -ENXIO;
	}
	
	error = i2c_smbus_read_byte_data(client, 0x75);
	if (error != 0x68) {
		dev_err(&client->dev, "ID check failed\n");
		return -ENXIO;
	}
	dev_alert(&client->dev, "ID = %x\n", error);
	
	mydevice->input_dev = devm_input_allocate_device(&client->dev);
	if (!mydevice->input_dev) {
		dev_err(&client->dev, "Failed to allocate input device\n");
		return -ENOMEM;
	}
	
	mydevice->input_dev->name = "MPU6050 Mouse";
	mydevice->input_dev->id.bustype = BUS_I2C;
	input_set_capability(mydevice->input_dev, EV_REL, REL_X);
	input_set_capability(mydevice->input_dev, EV_REL, REL_Y);
	input_set_capability(mydevice->input_dev, EV_KEY, BTN_LEFT);
	input_set_capability(mydevice->input_dev, EV_KEY, BTN_RIGHT);
	
	error = input_register_device(mydevice->input_dev);
	if (error) {
		dev_err(&client->dev, "Failed to register input device\n");
		return error;
	}
	
	mydevice->prev_accel_x = 0;
	mydevice->prev_accel_y = 0;
	mydevice->prev_button_state = -1;  // 初始化为一个不可能的状态
	
	timer_setup(&mydevice->poll_timer, mpu_mouse_timer_callback, 0);
	INIT_WORK(&mydevice->poll_work, mpu_mouse_poll_work);
	mod_timer(&mydevice->poll_timer, jiffies + msecs_to_jiffies(10));
	
	return 0;
}

static int mpu_mouse_remove(struct i2c_client *client)
{
	struct mpu_mouse_data *mydevice = i2c_get_clientdata(client);
	del_timer_sync(&mydevice->poll_timer);
	cancel_work_sync(&mydevice->poll_work);
	input_unregister_device(mydevice->input_dev);
	return 0;
}

static const struct of_device_id mpu_of_match[] = {
	{ .compatible = "inv,mpu6050" },
	{ }
};
MODULE_DEVICE_TABLE(of, mpu_of_match);

static struct i2c_driver mpu_mouse_driver = {
	.probe = mpu_mouse_probe,
	.remove = mpu_mouse_remove,
	.driver = {
		.name = "inv-mpu6050",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mpu_of_match),
	},
};
		
module_i2c_driver(mpu_mouse_driver);
MODULE_DESCRIPTION("MPU6050 mouse driver");
MODULE_LICENSE("GPL");

