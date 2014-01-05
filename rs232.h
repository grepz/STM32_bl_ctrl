#ifndef __TCTRL_RS232
#define __TCTRL_RS232

#include <termios.h>
#include <stdint.h>

int rs232_open(const char *path, int baud);
int rs232_poll(uint8_t *buf, size_t sz);
int rs232_send(uint8_t *buf, size_t len);
void rs232_close(void);

#endif /* __TCTRL_RS232 */
