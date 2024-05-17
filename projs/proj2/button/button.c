#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h> 
#include <linux/gpio.h>
#include <linux/device.h>  
#include <linux/uaccess.h>

#define GPIO_PL_KEY1 965  // PL_KEY1对应的GPIO编号
#define GPIO_PL_KEY2 966  // PL_KEY1对应的GPIO编号
#define BUTTON_TOTAL 2    // button个数

static int major, i;
static struct class* buttonClass = NULL;
static struct device* buttonDevice = NULL;

int button_open(struct inode *inode, struct file *filp) {
    int gpio = iminor(inode) == 0 ? GPIO_PL_KEY1 : GPIO_PL_KEY2;
    int result = gpio_request(gpio, "GPIO Button");
    if (result) {
        pr_err("GPIO %d request failed: %d\n", gpio, result);
        return result;
    }
    gpio_direction_input(gpio);
    filp->private_data = (void*)(size_t)gpio;  // 存储GPIO号作为私有数据
    return 0;
}


int button_release(struct inode *inode, struct file *file)
{
    int gpio = (int)(size_t)file->private_data;  // 从私有数据获取GPIO号
    gpio_free(gpio);
    return 0;
}


// 读取GPIO值
ssize_t button_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int gpio = (int)(size_t)filp->private_data;  // 从私有数据获取GPIO号
    int value = gpio_get_value(gpio);
    char value_str[2] = {value ? '1' : '0', '\n'};
    if (copy_to_user(buf, value_str, 2)) {
        return -EFAULT;
    }
    return 2; // 返回读取的字节数
}

struct file_operations my_button_ops =
{
	.owner = THIS_MODULE,
	.open = button_open,
	.release = button_release,
    .read = button_read,
};

int button_init(void)
{
    major = register_chrdev(0, "buttons", &my_button_ops);
    printk("devnum: %d\n", major);
	if(major<0)
	{
		pr_err("unable to register leds driver!\n");
		return major;
	}

    // 创建 device class
    buttonClass = class_create(THIS_MODULE, "buttons_class");
    if (IS_ERR(buttonClass)) {
        unregister_chrdev(major, "buttons");
        pr_err("Failed to register device class\n");
        return PTR_ERR(buttonClass);
    }

    // 创建 device
    for (i=0; i<BUTTON_TOTAL; i++) {
    buttonDevice = device_create(buttonClass, NULL, MKDEV(major, i), NULL, "button%d", i);
    if (IS_ERR(buttonDevice)) {
        // 错误处理：销毁之前创建的设备
        while (i-- > 0) {
            device_destroy(buttonClass, MKDEV(major, i));
        }
        class_destroy(buttonClass);
        unregister_chrdev(major, "buttons");
        pr_err("Failed to create the device\n");
        return PTR_ERR(buttonDevice);
    }
    pr_info("leds: device class created correctly\n");
}

	pr_alert("buttons module is installed\n");
	return 0;
}

void buttons_cleanup(void) {
    for (i=0; i<BUTTON_TOTAL; i++)
        device_destroy(buttonClass, MKDEV(major, i));
    class_destroy(buttonClass);
    unregister_chrdev(major, "buttons");
    pr_info("buttons: device class destroyed\n");
}

module_init(button_init);
module_exit(buttons_cleanup);
MODULE_LICENSE("GPL");