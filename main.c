#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <errno.h>

#include "utils.h"
#include "rs232.h"

#include "bl_proto.h"

static void __usage(const char *str);

int main(int argc, char *argv[])
{
    int ret, fd;
    uint32_t app_addr;
    int bdrate = B115200;
    size_t app_sz;

    if (argc < 4) {
        __usage(argv[0]);
        return 1;
    }

    printf("Opening serial device '%s'\n", argv[1]);
    ret = rs232_open(argv[1], bdrate);
    if(ret) {
        printf("Can't open serial device: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }

    ret = bl_send_msg(BL_PROTO_CMD_HANDSHAKE, NULL, 0, 1000);
    if (ret < 0) {
        printf("Error sending handshake request: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    } else if (ret != 0) {
        printf("Handshake failed: %s\n", bl_err_str(ret));
        return EXIT_FAILURE;
    }

    printf("Connection with bootloader established.\n");

    /* Getting info on a file to flash */
    fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        printf("Error opening binary file: %s\n", strerror(errno));
        bl_end_session();
        return EXIT_FAILURE;
    }

    /* Get application size and check if its thumb friendly */
    app_sz = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if (app_sz % 4)
        return ERR_DATA;

    /* Set application start address */
    sscanf(argv[3], "%x", &app_addr);
    printf("Setting application address: 0x%08X\n", app_addr);
    ret = bl_send_app_param(app_addr, app_sz);
    if (ret) {
        printf("Failed setting application address: %d\n", ret);
        bl_end_session();
        return EXIT_FAILURE;
    }

    ret = bl_send_data(fd, app_sz);
    if (ret != ERR_NO) {
        printf("Failed sending program to STM32 microcontroller: %d\n", ret);
        bl_end_session();
        return EXIT_FAILURE;
    } else
        printf("Finished sending binary data to STM32 microcontroller.\n");

    ret = bl_check_crc(app_addr, app_sz);
    if (ret) {
        printf("Failed checking of flashed data: %d\n", ret);
        bl_end_session();
        return EXIT_FAILURE;
    }

    printf("Booting application at address: 0x%08X\n", app_addr);
    ret = bl_boot_app(app_addr);
    if (ret) {
        printf("Failed booting application: %d\n", ret);
        bl_end_session();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void __usage(const char *str)
{
    printf("%s: [serial_device] [bin file] [addr]\n", str);
}
