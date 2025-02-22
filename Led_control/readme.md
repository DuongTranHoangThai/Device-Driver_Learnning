Control the led user 1 on BeagleBoneBlack
Build
**make**
Run
**sudo insmod led_driver.ko
sudo ./led_app <on|off|blink ...(ms)>**
Note: Need to turn off the user led 01 before 
**sudo bash -c 'echo 0 > /sys/class/leds/beaglebone:green:usr1/brightness'**
