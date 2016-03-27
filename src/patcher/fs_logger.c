#include <malloc.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "function_hooks.h"
#include "fs_logger.h"
#include "utils/utils.h"

#define CHECK_ERROR(cond) if (cond) { goto error; }

static int sendwait(int sock, const unsigned char *buffer, int len) {
    while (bss.lock)
        usleep(5000);
    bss.lock = 1;

    int ret;
    while (len > 0) {
        ret = send(sock, buffer, len, 0);
        if(ret < 0)
        {
            len = ret;
            break;
        }

        len -= ret;
        buffer += ret;
    }
    bss.lock = 0;
    return len;
}


int fs_logger_connect(int *psock) {
    struct sockaddr_in addr;
    int sock, ret;

    // No ip means that we don't have any server running, so no logs
    if (SERVER_IP == 0) {
        *psock = -1;
        return 0;
    }

    socket_lib_init();

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK_ERROR(sock == -1);

    addr.sin_family = AF_INET;
    addr.sin_port = 7332;
    addr.sin_addr.s_addr = SERVER_IP;

    ret = connect(sock, (void *)&addr, sizeof(addr));
    CHECK_ERROR(ret < 0);

    int enable = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

    unsigned char buffer[2];
    ret = recv(sock, buffer, 1, 0);
    CHECK_ERROR(ret < 0);

    *psock = sock;

    bss.lock = 0;
    return 0;

error:
    if (sock != -1)
        socketclose(sock);

    *psock = -1;
    SERVER_IP = 0;
    return -1;
}

void fs_logger_disconnect(int sock) {
    CHECK_ERROR(sock == -1);

    unsigned char buffer[2];
    buffer[0] = BYTE_DISCONNECT;
    buffer[1] = 0;
    sendwait(sock, buffer, 1);

    socketclose(sock);
error:
    return;
}

void fs_log_string(int sock, const char* str, unsigned char flag_byte) {
    if(sock == -1) {
		return;
	}
    int i;
    int len_str = 0;
    while (str[len_str])
        len_str++;

    //
    {
        unsigned char *buffer = memalign(0x40, ROUNDUP((1 + 4 + len_str + 1), 0x40));
        if(!buffer)
            return;

        buffer[0] = flag_byte;
        *(int *)(buffer + 1) = len_str + 1;
        for (i = 0; i < len_str; i++)
            buffer[5 + i] = str[i];

        buffer[5 + i] = 0;

        sendwait(sock, buffer, 1 + 4 + len_str + 1);

        free(buffer);
    }

}

void fs_log_byte(int sock, unsigned char byte) {
    if(sock != -1) {
        unsigned char *buffer = memalign(0x40, 0x40);
        if(!buffer)
            return;

        buffer[0] = byte;
        buffer[1] = 0;

        sendwait(sock, buffer, 1);

        free(buffer);
    }
}
