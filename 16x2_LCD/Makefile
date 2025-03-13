# Kernel source directory (modify this if needed)
KDIR := /lib/modules/$(shell uname -r)/build

# Module name
obj-m := lcd_driver.o

# Object files that make up the module
lcd_driver-objs := lcd.o gpio.o lcd_platform_driver.o

# Default rule to build the module
all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

# Clean rule to remove generated files
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean

