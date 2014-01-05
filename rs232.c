#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/epoll.h>

#include <errno.h>

#include "rs232.h"

#define EPOLL_TIMEOUT 0
#define EPOLL_SIZE    4

static int __fd, __epfd = -1;

static struct epoll_event __ev;
static struct epoll_event __events[EPOLL_SIZE];

static void __dump_message(uint8_t *buf, size_t sz);

int rs232_open(const char *path, int baud)
{
    struct termios new_tio;

    if (__epfd == -1) {
        __epfd = epoll_create(EPOLL_SIZE);
        if (__epfd < 0)
            return -errno;
    }

    __ev.events = EPOLLIN;
    __fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (__fd == -1)
        return -errno;

    memset(&new_tio, 0, sizeof(struct termios));
    new_tio.c_cflag = baud | CS8 | CREAD | CLOCAL;

    tcflush(__fd, TCIFLUSH);
    tcsetattr(__fd, TCSAFLUSH, &new_tio);

    __ev.data.fd = __fd;
    if (epoll_ctl(__epfd, EPOLL_CTL_ADD, __fd, &__ev) < 0) {
        close(__fd);
        return -errno;
    }

    return 0;
}

int rs232_poll(uint8_t *buf, size_t sz)
{
    int nfds, ret = 0, i;

    nfds = epoll_wait(__epfd, __events, EPOLL_SIZE, EPOLL_TIMEOUT);
    if (nfds < 0)
        return -1;

    for (i = 0; i < nfds; i++)
        if (__events[i].data.fd == __fd) {
            ret = read(__fd, buf, sz);
            if (ret == -1)
                return -errno;
        }

    if (ret != 0) {
        printf("Received. ");
        __dump_message(buf, ret);
    }

    return ret;
}

void rs232_close(void)
{
    close(__fd);
}

int rs232_send(uint8_t *buf, size_t len)
{
    int ret;

    ret = write(__fd, buf, len);
    if (ret == -1)
        return -errno;

    printf("Sent. ");
    __dump_message(buf, len);

    return ret;
}

static void __dump_message(uint8_t *buf, size_t sz)
{
    printf("<");
    while (sz --) {
        printf("%02X;", *buf);
        buf++;
    }
    printf(">\n");
}
