#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "logger.h"

#ifdef DEBUG_LOGGER
static int log_socket = -1;
static struct sockaddr_in connect_addr;
static volatile int log_lock = 0;


void log_init()
{
    int broadcastEnable = 1;
	log_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (log_socket < 0)
		return;

    setsockopt(log_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	memset(&connect_addr, 0, sizeof(struct sockaddr_in));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = 4405;
    connect_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
}

void log_print(const char *str)
{
    // socket is always 0 initially as it is in the BSS
    if(log_socket < 0) {
        return;
    }

    while(log_lock)
        os_usleep(1000);
    log_lock = 1;

    int len = strlen(str);
    int ret;
    while (len > 0) {
        int block = len < 1400 ? len : 1400; // take max 1400 bytes per UDP packet
        ret = sendto(log_socket, str, block, 0, (struct sockaddr *)&connect_addr, sizeof(struct sockaddr_in));
        if(ret < 0)
            break;

        len -= ret;
        str += ret;
    }

    log_lock = 0;
}

void log_printf(const char *format, ...)
{
    if(log_socket < 0) {
        return;
    }

	char * tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        log_print(tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}
#endif
