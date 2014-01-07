#ifndef __BL_PROTO_H
#define __BL_PROTO_H

#include "protocol.h"

enum {
    ERR_NO = 0,
    ERR_OVRFLW,
    ERR_MSG,
    ERR_CRC,
    ERR_STATUS,
    ERR_DATA,
    ERR_TIMEOUT
};

int bl_send_msg(int proto_cmd, uint8_t *data, size_t sz, unsigned int timeout);
int bl_send_data(int fd, size_t sz);
int bl_send_app_param(uint32_t addr, uint32_t sz);
int bl_boot_app(uint32_t addr);
int bl_check_crc(unsigned int as, unsigned int ae);
void bl_end_session(void);

const char *bl_err_str(unsigned int code);

#endif /* __BL_PROTO_H */
