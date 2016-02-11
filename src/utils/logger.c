#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "logger.h"

#define LOADIINE_LOGGER_IP  "192.168.2.156"

static int log_socket = 0;
static volatile int log_lock = 0;


void log_init(void)
{
    if(log_socket > 0)
        return;

	log_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (log_socket < 0)
		return;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = 4405;
	inet_aton(LOADIINE_LOGGER_IP, &connect_addr.sin_addr);

	if(connect(log_socket, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) < 0)
	{
	    socketclose(log_socket);
	    log_socket = -1;
	}
}

void log_print(const char *str)
{
    // socket is always 0 initially as it is in the BSS
    if(log_socket <= 0) {
        log_init();
        if(log_socket <= 0) {
            return;
        }
    }

    while(log_lock)
        usleep(1000);
    log_lock = 1;

    int len = strlen(str);
    int ret;
    while (len > 0) {
        ret = send(log_socket, str, len, 0);
        if(ret < 0)
            break;

        len -= ret;
        str += ret;
    }

    log_lock = 0;
}

void log_printf(const char *format, ...)
{
    if(log_socket <= 0) {
        log_init();
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
