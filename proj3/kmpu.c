#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/gpio.h>

struct mpu_mouse_data {
	struct i2c_client *client;
	unsigned int io_down;
};

static int mpu_mouse_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct mpu_mouse_data *mydevice;
	int error;
	mydevice = devm_kzalloc(&client->dev, sizeof(*mydevice), GFP_KERNEL);
	if (!mydevice){
		dev_err(&client->dev, "nomem\n");
		return -ENOMEM;
	}
	mydevice->client = client;
	i2c_set_clientdata(client, mydevice);

	dev_alert(&client->dev, "MPU Address: 0x%02x\n", client->addr);
	mydevice->io_down = of_get_named_gpio(np, "io_down", 0);
	pr_err("gpio=%d\n",mydevice->io_down);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
		dev_err(&client->dev, "I2C check functionality failed.\n");
		return -ENXIO;
	}
	error = i2c_smbus_read_byte_data(client, 0x75);
	if(error != 0x68){
		dev_err(&client->dev, "id check failed.\n");
		return -ENXIO;
	}
	dev_alert(&client->dev, "id=%x.\n",error);
	return 0;
}

static const struct of_device_id mpu_of_match[] = {
	{ .compatible = "inv,mpu6050" },
	{ }
};
MODULE_DEVICE_TABLE(of, mpu_of_match);

static struct i2c_driver mpu_mouse_driver = {
	.probe = mpu_mouse_probe,
	.driver = {
		.name = "inv-mpu6050",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mpu_of_match),
	},
};
module_i2c_driver(mpu_mouse_driver);

MODULE_DESCRIPTION("MPU6050 mouse driver");
MODULE_LICENSE("GPL");
