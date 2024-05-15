#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/input.h>   // 包含输入子系统的定义


static int mpu_mouse_major;
static struct class *mpu_mouse_class = NULL;
static struct cdev mpu_mouse_cdev;


struct mpu_mouse_data {
    struct i2c_client *client;
    unsigned int io_down;
    struct input_dev *input_dev; // 输入设备，用于生成鼠标事件
};

static int mpu_mouse_read_data(struct i2c_client *client, int *ax, int *ay, int *az, int *gx, int *gy, int *gz)
{
    // 读取加速度和陀螺仪数据
    
    // 返回值为0表示成功，非0表示失败
    return 0;
}

static void mpu_mouse_report_event(struct mpu_mouse_data *mouse_data, int ax, int ay, int az, int gx, int gy, int gz)
{
    // 根据读取的数据生成输入事件
    // 比如使用ax, ay值来模拟鼠标移动
}

static int mpu_mouse_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct mpu_mouse_data *mouse_data;
    int ax, ay, az, gx, gy, gz;
    int ret;
	dev_t dev_num;

    mouse_data = devm_kzalloc(&client->dev, sizeof(*mouse_data), GFP_KERNEL);
    if (!mouse_data) {
        dev_err(&client->dev, "Failed to allocate memory\n");
        return -ENOMEM;
    }

    mouse_data->client = client;
    i2c_set_clientdata(client, mouse_data);

    // 设置GPIO
    mouse_data->io_down = of_get_named_gpio(client->dev.of_node, "io_down", 0);

    // 创建输入设备
    mouse_data->input_dev = input_allocate_device();
    if (!mouse_data->input_dev) {
        dev_err(&client->dev, "Failed to allocate input device\n");
        return -ENOMEM;
    }

    mouse_data->input_dev->name = "MPU6050 Mouse";
    mouse_data->input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    mouse_data->input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
    mouse_data->input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT);

    // 注册输入设备
    ret = input_register_device(mouse_data->input_dev);
    if (ret) {
        dev_err(&client->dev, "Failed to register input device\n");
        input_free_device(mouse_data->input_dev);
        return ret;
    }

    // 定期读取数据并报告事件
    while (!kthread_should_stop()) {
        mpu_mouse_read_data(client, &ax, &ay, &az, &gx, &gy, &gz);
        mpu_mouse_report_event(mouse_data, ax, ay, az, gx, gy, gz);
        msleep(10); // 等待10ms
    }

    return 0;
}

static const struct i2c_device_id mpu_id[] = {
    { "mpu6050", 0 },
    { }
};

static struct i2c_driver mpu_mouse_driver = {
    .probe = mpu_mouse_probe,
    .id_table = mpu_id,
    .driver = {
        .name = "mpu6050_mouse",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(mpu_of_match),
    },
};

module_i2c_driver(mpu_mouse_driver);

MODULE_DESCRIPTION("MPU6050 Mouse Driver");
MODULE_LICENSE("GPL");

