#ifndef __TCTRL_UTILS_H
#define __TCTRL_UTILS_H

#include <stdint.h>

inline uint8_t crc8(uint8_t *data, int len);
inline uint32_t crc32(const uint8_t *data, size_t len, uint32_t crc);

#endif /* __TCTRL_UTILS_H */
