#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h> 
#include <linux/gpio.h>
#include <linux/device.h>  
#include <linux/uaccess.h>

#define LED_TOTAL 3
static int led_pins[LED_TOTAL] = {906, 960, 961};

static int major, i;
static struct class* ledClass = NULL;
static struct device* ledDevice = NULL;

int light_open(struct inode *inode, struct file *filp)
{
    int lednum = 0;
    switch(iminor(inode))
    {
    case 0:
        lednum = 906;
        break;
    case 1:
        lednum = 960;
        break;
    default:
        lednum = 961;
    }
    filp->private_data = (void *) lednum;
    return 0;
}

int light_release(struct inode *inode, struct file *filp)
{
    return 0;
}

long light_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int lednum;
    lednum = (int) filp->private_data;
    switch(cmd)		// LED逻辑电平是反的
    {
        case 0:
        gpio_set_value(lednum, 0);
        break;
        case 1:
        gpio_set_value(lednum, 1);
        break;
        default:
        return -ENOTTY;
    }
    return 0;
}

ssize_t light_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int lednum = (int) filp->private_data;
    char buffer[2];
    int value = gpio_get_value(lednum);

    buffer[0] = value ? '1' : '0';
    buffer[1] = '\n';

    if (*f_pos > 0)
        return 0; // EOF

    if (count < 2)
        return -EINVAL; // Buffer is too small

    if (copy_to_user(buf, buffer, 2))
        return -EFAULT;

    *f_pos += 2; // Update the file position
    return 2; // Number of bytes written to the buffer
}

ssize_t light_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    char buffer[10];
    int lednum = (int) filp->private_data;
    if (count > sizeof(buffer) - 1)
        return -EINVAL;

    if (copy_from_user(buffer, buf, count))
        return -EFAULT;

    buffer[count] = '\0'; // Null terminate the string

    if (buffer[0] == '1') {
        gpio_set_value(lednum, 0);
    } else if (buffer[0] == '0') {
        gpio_set_value(lednum, 1);	// LED逻辑电平是反的
    } else {
        return -EINVAL;
    }

    return count;
}

struct file_operations light_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = light_ioctl,
    .open = light_open,
    .release = light_release,
    .read = light_read,
    .write = light_write,
};

// 模块加载函数
int light_init(void)
{
    int result;
    major = register_chrdev(0, "leds", &light_fops);
    printk("devnum: %d\n", major);      /* /dev/leds */
    if(major < 0)
    {
        pr_err("unable to register leds driver!\n");
        return major;
    }

    // 请求并初始化GPIO
    for (i = 0; i < LED_TOTAL; i++) {
        result = gpio_request(led_pins[i], "Leds");
        if (result != 0) {
            pr_err("gpio_request failed for pin %d!\n", led_pins[i]);
            goto fail_gpio_request;
        }
        result = gpio_direction_output(led_pins[i], 1);
        if (result != 0) {
            pr_err("gpio_direction_output failed for pin %d!\n", led_pins[i]);
            goto fail_gpio_direction;
        }
    }

    // 创建 device class
    ledClass = class_create(THIS_MODULE, "myled_class");
    if (IS_ERR(ledClass)) {
        unregister_chrdev(major, "leds");
        pr_err("Failed to register device class\n");
        return PTR_ERR(ledClass);
    }

    // 创建 device
    for (i = 0; i < LED_TOTAL; i++) {
        ledDevice = device_create(ledClass, NULL, MKDEV(major, i), NULL, "led%d", i);
        if (IS_ERR(ledDevice)) {
            // 错误处理：销毁之前创建的设备
            while (i-- > 0) {
                device_destroy(ledClass, MKDEV(major, i));
            }
            class_destroy(ledClass);
            unregister_chrdev(major, "leds");
            pr_err("Failed to create the device\n");
            return PTR_ERR(ledDevice);
        }
        pr_info("leds: device class created correctly\n");
    }

    pr_alert("leds module is installed\n");
    return 0;

fail_gpio_direction:
    gpio_free(led_pins[i]);
fail_gpio_request:
    while (i-- > 0) {
        gpio_free(led_pins[i]);
    }
    unregister_chrdev(major, "leds");
    return -1;
}

// 模块卸载函数
void light_cleanup(void)
{
    for (i = 0; i < LED_TOTAL; i++) {
        device_destroy(ledClass, MKDEV(major, i));
        gpio_free(led_pins[i]);
    }
    class_destroy(ledClass);
    unregister_chrdev(major, "leds");
    pr_info("leds: device class destroyed\n");
}

module_init(light_init);
module_exit(light_cleanup);
MODULE_LICENSE("GPL");

