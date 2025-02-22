#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "led_ioctl.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <on|off|blink <ms>>\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/led_driver", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (strcmp(argv[1], "on") == 0) {
        ioctl(fd, LED_ON);
    } else if (strcmp(argv[1], "off") == 0) {
        ioctl(fd, LED_OFF);
    } else if (strcmp(argv[1], "blink") == 0) {
        if (argc < 3) {
            printf("Missing blink period (ms)\n");
            close(fd);
            return -1;
        }
        int period = atoi(argv[2]);
        if (period < 100) {
            printf("Warning: Period too short, setting to 100ms\n");
            period = 100;
        }
        ioctl(fd, LED_BLINK, period);
    } else {
        printf("Invalid command. Use: on | off | blink <ms>\n");
    }

    close(fd);
    return 0;
}
