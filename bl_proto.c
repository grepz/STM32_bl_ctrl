#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <errno.h>

#include "rs232.h"
#include "utils.h"
#include "bl_proto.h"

static const char *__err_str[] = {
    "Buffer margins error",
    "Wrong message or format",
    "CRC checking error",
    "Status code signals of error",
    "Data error",
    "Timeout"
};

typedef struct __bl_proto
{
    int cmd;
    int (*req)(const uint8_t *buf, size_t n);
    int (*reply)(uint8_t *buf, size_t n);
} bl_proto_t;

static int __handshake_reply(uint8_t *buf, size_t n);
static int __status_reply(uint8_t *buf, size_t n);
static int __crc_reply(uint8_t *buf, size_t n);

bl_proto_t proto_parse[] = {
    {BL_PROTO_CMD_HANDSHAKE,  NULL, __handshake_reply},
    {BL_PROTO_CMD_ERASE,      NULL, __status_reply},
    {BL_PROTO_CMD_FLASH,      NULL, __status_reply},
    {BL_PROTO_CMD_EOS,        NULL, __status_reply},
    {BL_PROTO_CMD_DATA_CRC,   NULL, __crc_reply},
    {BL_PROTO_CMD_BOOT,       NULL, __status_reply},
    {-1,                      NULL, NULL}
};

uint32_t data_crc;

int bl_send_msg(int proto_cmd, uint8_t *data, size_t sz, unsigned int timeout)
{
    int ret;
    unsigned int timed;
    uint8_t buf[4096];
    bl_proto_t *ptr = proto_parse;

    if (sz > 256)
        return ERR_OVRFLW;

    for (ptr = proto_parse; ptr->cmd != -1; ptr++)
        if (ptr->cmd == proto_cmd)
            break;

    /* Unknown command */
    if (ptr->cmd == -1) {
        return 1;
    }
    /* See if we have special case for request */
    if (ptr->req) {
        ret = ptr->req(data, sz);
        if (ret < 0) {
            return ret;
        }
    } else {
        buf[0] = proto_cmd;
        memcpy(buf + 1, data, sz);
        buf[sz + 1] = BL_PROTO_EOM;
        buf[sz + 2] = crc8(buf, sz + 2);
        ret = sz + 3;
    }

    ret = rs232_send(buf, ret);
    if (ret < 0) {
        return ret;
    }

    /* If timeout is specified, wait for reply with ~timeout */
    if (timeout) {
        for (timed = 0; timed < timeout * 1000; timed += 1000) {
            usleep(1000);
            ret = rs232_poll(buf, 4096);
            if (ret == -EAGAIN || !ret)
                continue;
            else if (ret < 0) {
                return ret;
            }

            if (ptr->reply) {
                return ptr->reply(buf, ret);
            } else
                return ERR_NO;
        }
    } else {
        ret = rs232_poll(buf, 4096);
        if (ret < 0)
            return ret;
        if (ptr->reply)
            return ptr->reply(buf, ret);
    }

    return ERR_TIMEOUT;
}

int bl_send_data(int fd, size_t sz)
{
    uint32_t crc = 0;
    uint8_t data[256], buf[4096];
    int ret, tr;
    unsigned int timed, timeout = 10000000;
    uint8_t msg[] = {BL_PROTO_CMD_FLASH_DATA, 0, BL_PROTO_EOM, 0};

    while (sz) {
        tr = (sz >= 256) ? 256 : sz;
        ret = read(fd, data, tr);
        if (ret != tr)
            return ERR_DATA;

        /* Setting data size and recalculating CRC */
        msg[1] = tr - 1;
        msg[3] = crc8(msg, 3);

        /* Sending first part of message */
        ret = rs232_send(msg, 2);
        if (ret < 0) return ret;
        usleep(10000);
        /* Sending binary data */
        ret = rs232_send(data, tr);
        if (ret < 0) return ret;
        usleep(10000);
        /* Sending second part of message */
        ret = rs232_send(msg + 2, 2);
        if (ret < 0) return ret;
        usleep(10000);

        /* Waiting for response, check if write was OK */
        for (timed = 0; timed <= timeout; timed += 1000) {
            usleep(1000);
            ret = rs232_poll(buf, 4096);
            if (ret == -EAGAIN || ret == 0)
                continue;
            else if (ret < 0){
                return ret;
            }

            ret = __status_reply(buf, ret);
            if (ret != ERR_NO) {
                return ret;
            } else
                break;
        }

        if (timed >= timeout)
            return ERR_TIMEOUT;

        /* Adding to data CRC */
        crc = crc32(data, tr, crc);
        sz -= tr;
    }

    data_crc = crc;

    return ERR_NO;
}

void bl_end_session(void)
{
    int ret;

    ret = bl_send_msg(BL_PROTO_CMD_EOS, NULL, 0, 10000);
    if (ret < 0) {
        printf("Error waiting for session end: %s\n", strerror(-ret));
        exit(EXIT_FAILURE);
    } else if (ret != 0) {
        printf("End of session status error: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    printf("Session ended.\n");
}

/* Get data CRC from address 'as' to adress 'ae' */
int bl_check_crc(unsigned int as, unsigned int ae)
{
    uint8_t msg[] = {
        as & 0xFF, (as>>8) & 0xFF, (as>>16) & 0xFF, (as>>24) & 0xFF,
        ae & 0xFF, (ae>>8) & 0xFF, (ae>>16) & 0xFF, (ae>>24) & 0xFF
    };
    return bl_send_msg(BL_PROTO_CMD_DATA_CRC, msg, 8, 100000);
}

int bl_boot_app(uint32_t addr)
{
    uint8_t msg[] = {
        /* Application address */
        addr         & 0xFF, (addr >> 8)  & 0xFF,
        (addr >> 16) & 0xFF, (addr >> 24) & 0xFF,
    };

    return bl_send_msg(BL_PROTO_CMD_BOOT, msg, 4, 10000);
}

/* Send address 'addr' to start flashing with and application size 'sz' */
int bl_send_app_param(uint32_t addr, uint32_t sz)
{
    uint8_t msg[] = {
        /* Application address */
        addr         & 0xFF, (addr >> 8)  & 0xFF,
        (addr >> 16) & 0xFF, (addr >> 24) & 0xFF,
        /* Application size */
        sz         & 0xFF, (sz >> 8)  & 0xFF,
        (sz >> 16) & 0xFF, (sz >> 24) & 0xFF
    };

    return bl_send_msg(BL_PROTO_CMD_FLASH, msg, 8, 100000);
}


const char *bl_err_str(unsigned int code)
{
    return __err_str[code - 1];
}

/* Get handshake data */
static int __handshake_reply(uint8_t *buf, size_t n)
{
    uint32_t flash_sz = 0;

    if (n != 12) {
        return ERR_MSG;
    }
    else if (crc8(buf, 11) != buf[11]) {
        return ERR_CRC;
    }

    flash_sz |= buf[6];
    flash_sz |= buf[7] << 8;
    flash_sz |= buf[8] << 16;
    flash_sz |= buf[9] << 24;

    printf("Protocol ver %d.%d\n"
           "BoardID: %02X:%02X:%02X:%02X\n"
           "Flashable area: %d\n",
           buf[0], buf[1],
           buf[2], buf[3], buf[4], buf[5],
           flash_sz);

    return ERR_NO;
}

static int __status_reply(uint8_t *buf, size_t n)
{
    if (n != 3)
        return ERR_MSG;
    else if (crc8(buf, 2) != buf[2])
        return ERR_CRC;

    if (buf[0] != BL_PROTO_STATUS_OK) {
        return ERR_STATUS;
    }

    return ERR_NO;
}

static int __crc_reply(uint8_t *buf, size_t n)
{
    union {
        uint32_t crc;
        uint8_t b[4];
    } crc_val;

    if (n != 7)
        return ERR_MSG;
    else if (crc8(buf, 6) != buf[6]){
        return ERR_CRC;
    }

    if (buf[0] == BL_PROTO_STATUS_OK) {
        crc_val.b[0] = buf[1];
        crc_val.b[1] = buf[2];
        crc_val.b[2] = buf[3];
        crc_val.b[3] = buf[4];
        printf("CRC %X:%X \n", data_crc, crc_val.crc);
        if (data_crc != crc_val.crc)
            return ERR_CRC;
    } else {
        printf("CRC check failed on MC side: %d\n", buf[0]);
    }

    return ERR_NO;
}
