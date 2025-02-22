obj-m += led_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

app: 
	gcc -Wall -Wextra led_app.c -o led_app

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f led_app

