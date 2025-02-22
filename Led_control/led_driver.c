#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "led_ioctl.h"

#define DEVICE_NAME "led_driver"
#define LED_GPIO 54  // GPIO của User LED 1

static dev_t dev_num;
static struct cdev led_cdev;
static struct class *led_class;

static struct timer_list led_timer;
static int blink_period = 1000; // Nhấp nháy mỗi 1 giây

static void led_timer_callback(struct timer_list *t) {
    static bool led_state = 0;
    led_state = !led_state;
    gpio_set_value(LED_GPIO, led_state);

    mod_timer(&led_timer, jiffies + msecs_to_jiffies(blink_period));
}


// Hàm mở file
static int led_open(struct inode *inode, struct file *file) {
    pr_info("LED Driver: Device opened\n");
    return 0;
}

// Hàm đóng file
static int led_release(struct inode *inode, struct file *file) {
    pr_info("LED Driver: Device closed\n");
    return 0;
}

// Hàm ghi dữ liệu vào file /dev/led_driver
static ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char buffer[2] = {0};

    if (len > 2)
        return -EINVAL;

    if (copy_from_user(buffer, buf, len))
        return -EFAULT;

    if (buffer[0] == '1') {
        gpio_set_value(LED_GPIO, 1);
        pr_info("LED Driver: LED ON\n");
    } else if (buffer[0] == '0') {
        gpio_set_value(LED_GPIO, 0);
        pr_info("LED Driver: LED OFF\n");
    } else {
        pr_err("LED Driver: Invalid input\n");
        return -EINVAL;
    }

    return len;
}

// Hàm đọc trạng thái LED
static ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    char buffer[2];
    int led_state = gpio_get_value(LED_GPIO);

    buffer[0] = led_state ? '1' : '0';
    buffer[1] = '\n';

    if (*offset == 0) {
        if (copy_to_user(buf, buffer, 2))
            return -EFAULT;
        *offset = 2;
        return 2;
    }

    return 0;
}

static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case LED_ON:
            gpio_set_value(LED_GPIO, 1);
            break;
        case LED_OFF:
            gpio_set_value(LED_GPIO, 0);
            break;
        case LED_BLINK:
            blink_period = arg; // arg là thời gian nhấp nháy (ms)
            mod_timer(&led_timer, jiffies + msecs_to_jiffies(blink_period));
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

// Định nghĩa file operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .write = led_write,
    .read = led_read,
    .unlocked_ioctl = led_ioctl
};

// Hàm khởi tạo module
static int __init led_init(void) {
    int ret;

    pr_info("LED Driver: Initializing\n");

    // Cấp phát số major/minor
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("LED Driver: Failed to allocate char device\n");
        return ret;
    }

    // Khởi tạo cdev
    cdev_init(&led_cdev, &fops);
    led_cdev.owner = THIS_MODULE;

    ret = cdev_add(&led_cdev, dev_num, 1);
    if (ret) {
        pr_err("LED Driver: Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // Tạo class và device file
    led_class = class_create(THIS_MODULE, "led_class");
    if (IS_ERR(led_class)) {
        pr_err("LED Driver: Failed to create class\n");
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(led_class);
    }

    device_create(led_class, NULL, dev_num, NULL, DEVICE_NAME);

    // Yêu cầu quyền điều khiển GPIO
    ret = gpio_request(LED_GPIO, "usr1_led");
    if (ret) {
        pr_err("LED Driver: Failed to request GPIO %d\n", LED_GPIO);
        device_destroy(led_class, dev_num);
        class_destroy(led_class);
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    gpio_direction_output(LED_GPIO, 0);
    pr_info("LED Driver: Initialized successfully\n");

    timer_setup(&led_timer, led_timer_callback, 0);


    return 0;
}

// Hàm cleanup module
static void __exit led_exit(void) {
    del_timer_sync(&led_timer);
    gpio_set_value(LED_GPIO, 0);
    gpio_free(LED_GPIO);
    device_destroy(led_class, dev_num);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("LED Driver: Unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Char Device Driver for BeagleBone Black User LED 1");
