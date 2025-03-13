#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include<linux/slab.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include "lcd_platform_driver.h"
#include "lcd.h"
#include "gpio.h"
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

//Private data structure
struct lcd_private_data lcd_data = 
{
	.lcd_scroll = 0 , 
	.lcdxy = "(1,1)" 
};

//Global driver data
struct private_driver_data
{
	struct class* class;
};
struct private_driver_data prv_drv_data;

//Attribute "lcdcmd"
static ssize_t lcdcmd_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	
	long command;
	int ret = kstrtol(buf,0,&command);
	pr_info("Command : %s\n",buf);
	if(!ret)
	{
		lcd_send_command((u8)command,dev);
	}
	
	return ret ?:size;
}

static DEVICE_ATTR_WO(lcdcmd);

//Attribute "lcdtext"
static ssize_t lcdtext_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	//pr_info("This is store function of lcdtext attribute\n");
	if(buf)
	{
		lcd_print_string((char*)buf,dev);
		dev_info(dev,"Text : %s\n",buf);
	}
	else
	{
		dev_err(dev,"Buffer is NULL\n");
		return -EINVAL;
	}
	return size;

}
static DEVICE_ATTR_WO(lcdtext);

static ssize_t lcdscroll_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;

	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);

	if(lcd_data->lcd_scroll)
	{
		ret = sprintf(buf,"on\n");
	}
	else
	{
		ret = sprintf(buf,"off\n");
	}
	return ret;;
}


static ssize_t lcdscroll_store(struct device *dev,struct device_attribute *attr,\
	       					const char *buf, size_t size)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	if(sysfs_streq(buf,"on"))
	{
		lcd_data->lcd_scroll = 1;
	}
	else if (sysfs_streq(buf,"off"))
	{
		lcd_data->lcd_scroll = 0;
	}
	else
	{
		dev_err(dev,"Scroll should be on or off \n");
		return -EINVAL;
	}

	if(lcd_data->lcd_scroll)
	{
		pr_info("Scroll is on\n");
		SHIFT_LEFT_ON(dev);
	}
	else
	{
		lcd_display_return_home(dev);
		pr_info("Scroll is off\n");
		SHIFT_OFF(dev);
	}
	

	return size;
}	
static DEVICE_ATTR_RW(lcdscroll);

static ssize_t lcdxy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	return sprintf(buf,"%s\n",lcd_data->lcdxy);
}


static ssize_t lcdxy_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	long value;
	int status;
	int x, y;
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	
	status = kstrtol(buf,10,&value);
	if(status)
		return status;
	x = value /10;
	if (x > 2)
	{
		dev_err(dev,"Row should less than 2\n");
		return -EINVAL;
	}
	y = value % 10;
	status = sprintf(lcd_data->lcdxy ,"(%d,%d)",x,y);
	lcd_set_cursor(x,y,dev);
	return status;
}

static DEVICE_ATTR_RW(lcdxy);

//Attribute register
static struct attribute *lcd_attrs[] = {
	&dev_attr_lcdtext.attr,
	&dev_attr_lcdcmd.attr,
	&dev_attr_lcdscroll.attr,
	&dev_attr_lcdxy.attr,
	NULL
};
struct attribute_group lcd_attr_group = {
	.attrs = lcd_attrs
};
static const struct attribute_group *lcd_attr_groups[] = {
	&lcd_attr_group,
	NULL
};

//Platform proble function
int lcd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	dev_info(&pdev->dev,"This is probe function");

	//Set driver data as "lcd_data"
	dev_set_drvdata(dev,&lcd_data);
	
	//Get pin description.
	lcd_data.desc[LCD_RS] = gpiod_get(dev, "rs", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_RW] = gpiod_get(dev, "rw", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_EN] = gpiod_get(dev, "en", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_D4] = gpiod_get(dev, "d4", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_D5] = gpiod_get(dev, "d5", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_D6] = gpiod_get(dev, "d6", GPIOD_OUT_LOW);
	lcd_data.desc[LCD_D7] = gpiod_get(dev, "d7", GPIOD_OUT_LOW);
	if(IS_ERR(lcd_data.desc[LCD_RS]) || \
	   IS_ERR(lcd_data.desc[LCD_RW]) || \
	   IS_ERR(lcd_data.desc[LCD_EN]) || \
	   IS_ERR(lcd_data.desc[LCD_D4]) || \
	   IS_ERR(lcd_data.desc[LCD_D5]) || \
	   IS_ERR(lcd_data.desc[LCD_D6]) || \
	   IS_ERR(lcd_data.desc[LCD_D7])  )
	{
		dev_err(dev,"Gpio error\n");
		return -EINVAL;
	}
	
	//Note :At here we can controll the LCD pin
	
	//Init LCD (use LCD lib)
	lcd_init(dev);
	
	//Create device "LCD16x2_dev "with attribute
	lcd_data.dev = device_create_with_groups(prv_drv_data.class,dev,0,&lcd_data,lcd_attr_groups,"LCD16x2_dev");
	if (IS_ERR(lcd_data.dev)) {
		dev_err(dev,"Error while creating class entry \n");
		return PTR_ERR(lcd_data.dev);
	}
	
	
	//Note : at here everything is completetly set up, shoud print the messange on lcd
	
	
	return 0;
}

//Platform remove function
int lcd_remove(struct platform_device *pdev)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(&pdev->dev);
	lcd_deinit(&pdev->dev);
	dev_info(&pdev->dev,"remove called\n");
	device_unregister(lcd_data->dev);
	return 0;
}


//Platform compatible string 
struct of_device_id lcd_device_match[] = {
        { .compatible = "org,lcd16x2"},
        { }
};

//Platform driver
struct platform_driver lcdsysfs_platform_driver = 
{
	.probe = lcd_probe,
	.remove = lcd_remove,
	.driver = {
		.name = "lcd-sysfs",
		.of_match_table = of_match_ptr(lcd_device_match)
	}

};

static int __init pcd_platform_driver_init(void)
{
	int ret;
	
	/*Create device class under /sys/class */
	prv_drv_data.class = class_create(THIS_MODULE,"LCD1602_class");
	if(IS_ERR(prv_drv_data.class)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(prv_drv_data.class);
		return ret;
	}
	// Register platform driver 
	platform_driver_register(&lcdsysfs_platform_driver);
	
	//Loaded successfully
	pr_info("LCD platform driver loaded\n");
	return 0;
}


static void __exit pcd_platform_driver_cleanup(void)
{
	platform_driver_unregister(&lcdsysfs_platform_driver);
	/*Class destroy */
	class_destroy(prv_drv_data.class);
	//Remove successfully
	pr_info("LCD platform driver unloaded\n");
}


module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Duong Tran Hoang Thai");
MODULE_DESCRIPTION("LCD 16x2 platform driver with sysfs interface");
MODULE_VERSION("1.0.0");