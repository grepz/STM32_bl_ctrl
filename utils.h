#ifndef __TCTRL_UTILS_H
#define __TCTRL_UTILS_H

#include <stdint.h>

uint8_t crc8(uint8_t *data, int len);
uint32_t crc32(const uint8_t *data, size_t len, uint32_t crc);

#endif /* __TCTRL_UTILS_H */
