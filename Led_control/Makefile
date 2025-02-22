obj-m += led_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CC := gcc
CFLAGS := -Wall -Wextra -O2

all: driver app  

driver:
	make -C $(KDIR) M=$(PWD) modules

app: 
	$(CC) $(CFLAGS) led_app.c -o led_app

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f led_app

.PHONY: all driver app clean
