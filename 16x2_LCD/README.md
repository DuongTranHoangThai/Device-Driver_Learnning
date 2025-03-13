# LCD 16x2 HD44780 Control Program

Using BeagleBone Black board from TI

## Hardware Connections

Data from LCD is directly connected to the board as below:

```
===========================================================================================================
BBB_expansion_header_pins       GPIO number     16x2 LCD pin      Purpose 
===========================================================================================================
P8.7                              gpio2.2          4(RS)           Register selection (Character vs. Command)
P8.46                             gpio2.7          5(RW)           Read/write 
P8.43                             gpio2.8          6(EN)           Enable
P8.44                             gpio2.9          11(D4)          Data line 4
P8.41                             gpio2.10         12(D5)          Data line 5
P8.42                             gpio2.11         13(D6)          Data line 6
P8.39                             gpio2.12         14(D7)          Data line 7 
P9.7(GND)                                          15(BKLTA)       Backlight anode(+)
P9.1(sys_5V supply)                                16(BKLTK)       Backlight cathode(-)
P9.1 (GND)                        ----             1(VSS/GND)      Ground
P9.7(sys_5V supply)               ----             2(VCC)          +5V supply 
===========================================================================================================
```

## 1. Modify Device Tree

### Create Device Tree Include File

Create a new file named `lcd16x2.dtsi` with the following content:

```dts
/ {
       bone_gpio_devs {
	  
		     compatible = "org,bone-gpio-sysfs";
		     pinctrl-single,names = "default";
		     pinctrl-0 = <&p8_gpios>;
		     status = "disable";
		
		        gpio1 {
			        label = "gpio2.2";
			        bone-gpios = <&gpio2 2 GPIO_ACTIVE_HIGH>;
				        
		        };

		        gpio2 {
			        label = "gpio2.7";
			        bone-gpios = <&gpio2 7 GPIO_ACTIVE_HIGH>;
		        };
		        

		        gpio3 {
			        label = "gpio2.8";
			        bone-gpios = <&gpio2 8 GPIO_ACTIVE_HIGH>;
		        };
		        
		        
		        gpio4 {
			        label = "gpio2.9";
			        bone-gpios = <&gpio2 9 GPIO_ACTIVE_HIGH>;
		        };

		        gpio5{
			        label = "gpio2.10";
			        bone-gpios = <&gpio2 10 GPIO_ACTIVE_HIGH>;
		        };

		        gpio6{
			        label = "gpio2.11";
			        bone-gpios = <&gpio2 11 GPIO_ACTIVE_HIGH>;
		        };

		        gpio7{
			        label = "gpio2.12";
			        bone-gpios = <&gpio2 12 GPIO_ACTIVE_HIGH>;
		        };

		        led1 {
			        label = "usrled1:gpio1.22";
			        bone-gpios = <&gpio1 22 GPIO_ACTIVE_HIGH>;
		        };

		        led2 {
			        label = "usrled2:gpio1.23";
			        bone-gpios = <&gpio1 23 GPIO_ACTIVE_HIGH>;
		        };

	};//bone_gpio_devs


    lcd16x2 {
            compatible = "org,lcd16x2";
            pictrl-names = "default";
            pinctrl-0 = <&p8_gpios>;
            status = "okay";
            rs-gpios = <&gpio2 2  GPIO_ACTIVE_HIGH>;
            rw-gpios = <&gpio2 7  GPIO_ACTIVE_HIGH>;
            en-gpios = <&gpio2 8  GPIO_ACTIVE_HIGH>;
            d4-gpios = <&gpio2 9  GPIO_ACTIVE_HIGH>;
            d5-gpios = <&gpio2 10  GPIO_ACTIVE_HIGH>;
            d6-gpios = <&gpio2 11  GPIO_ACTIVE_HIGH>;
            d7-gpios = <&gpio2 12  GPIO_ACTIVE_HIGH>;
    };


}; //root node

&tda19988 {
	status = "disabled";
};
&lcdc {
    status = "disabled";
};


&am33xx_pinmux {
	p8_gpios: bone_p8_gpios {
		pinctrl-single,pins = < 
                    AM33XX_PADCONF(AM335X_PIN_GPMC_ADVN_ALE,PIN_OUTPUT,MUX_MODE7) // P8.7 is used alter
					/* AM33XX_PADCONF(AM335X_PIN_LCD_DATA0,PIN_OUTPUT,MUX_MODE7) */ //not use due to affect boot mode 
                    AM33XX_PADCONF(AM335X_PIN_LCD_DATA1,PIN_OUTPUT,MUX_MODE7) 
					AM33XX_PADCONF(AM335X_PIN_LCD_DATA2,PIN_OUTPUT,MUX_MODE7) 
					AM33XX_PADCONF(AM335X_PIN_LCD_DATA3,PIN_OUTPUT,MUX_MODE7) 
					AM33XX_PADCONF(AM335X_PIN_LCD_DATA4,PIN_OUTPUT,MUX_MODE7) 
					AM33XX_PADCONF(AM335X_PIN_LCD_DATA5,PIN_OUTPUT,MUX_MODE7) 
					AM33XX_PADCONF(AM335X_PIN_LCD_DATA6,PIN_OUTPUT,MUX_MODE7) 
		>;

	};

};
```
### Include the `.dtsi` File in the Main Device Tree

Edit the `am335x-boneblack.dts` file and add the following line at an appropriate place:

```dts
#include "lcd16x2.dtsi"
```

## 2. Rebuild and Apply Device Tree

### Rebuild Device Tree
```sh
sudo dtc -O dtb -o am335x-boneblack.dtb -b 0 -@ am335x-boneblack.dts
```

### Apply New Device Tree at Boot
Copy the generated `am335x-boneblack.dtb` to the `/boot/dtbs/` directory:
```sh
sudo cp am335x-boneblack.dtb /boot/dtbs/
```

Then update the `uEnv.txt` file to use the new device tree:
```sh
sudo nano /boot/uEnv.txt
```
Modify the following line:
```sh
dtb=am335x-boneblack.dtb
```
Save and exit, then reboot the board:
```sh
sudo reboot
```

## 3. Build the Kernel Module

```sh
make
sudo insmod lcd_driver.ko
```

## 4. Control the LCD

After inserting the module, the following files will appear under `/sys/devices/platform/lcd16x2/LCD1602_class/LCD16x2_dev`:

- `lcdcmd`  (write-only)
- `lcdscroll`  (write/read)
- `lcdtext`  (write-only)
- `lcdxy` (write-only)

### Writing an LCD Command
```sh
echo [command] > lcdcmd
```
**Common commands:**
- `0X01` : Clear LCD
- `0x02` : Set cursor to home
- (Check the HD44780 datasheet for other commands)

### Setting Cursor Position
```sh
echo "xy" > lcdxy
```
- `x`: LCD row (1~2)
- `y`: LCD column (1~9)

### Writing Text
```sh
echo -n "HelloWorld" > lcdtext
```

### Scrolling Text Left by One Character
```sh
echo "on" > lcdscroll
```

**Note:** The `echo` commands may require root permissions.

## TODO
- Change the control pins, as the current pin configuration causes boot failures.

