#include <string.h>
#include <malloc.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "kernel/syscalls.h"

struct pygecko_bss_t {
	int error, line;
	void* thread;
	unsigned char stack[0x8000];
};

#define CHECK_ERROR(cond) if (cond) { bss->line = __LINE__; goto error; }
#define errno (*__gh_errno_ptr())
#define EWOULDBLOCK     6

static int recvwait(struct pygecko_bss_t *bss, int sock, void *buffer, int len) {
	int ret;
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return 0;
error:
	bss->error = ret;
	return ret;
}

static int recvbyte(struct pygecko_bss_t *bss, int sock) {
	unsigned char buffer[1];
	int ret;

	ret = recvwait(bss, sock, buffer, 1);
	if (ret < 0) return ret;
	return buffer[0];
}

static int checkbyte(struct pygecko_bss_t *bss, int sock) {
	unsigned char buffer[1];
	int ret;

	ret = recv(sock, buffer, 1, MSG_DONTWAIT);
	if (ret < 0) return ret;
	if (ret == 0) return -1;
	return buffer[0];
}

static int sendwait(struct pygecko_bss_t *bss, int sock, const void *buffer, int len) {
	int ret;
	while (len > 0) {
		ret = send(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return 0;
error:
	bss->error = ret;
	return ret;
}

static int sendbyte(struct pygecko_bss_t *bss, int sock, unsigned char byte) {
	unsigned char buffer[1];

	buffer[0] = byte;
	return sendwait(bss, sock, buffer, 1);
}

static int rungecko(struct pygecko_bss_t *bss, int clientfd) {
	int ret;
	unsigned char buffer[0x401];

	while (1) {
		ret = checkbyte(bss, clientfd);

		if (ret < 0) {
			CHECK_ERROR(errno != EWOULDBLOCK);
			GX2WaitForVsync();
			continue;
		}

		switch (ret) {
		case 0x01: { /* cmd_poke08 */
			char *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((char **)buffer)[0];
			*ptr = buffer[7];
			DCFlushRange(ptr, 1);
			break;
		}
		case 0x02: { /* cmd_poke16 */
			short *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((short **)buffer)[0];
			*ptr = ((short *)buffer)[3];
			DCFlushRange(ptr, 2);
			break;
		}
		case 0x03: { /* cmd_pokemem */
			int *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((int **)buffer)[0];
			*ptr = ((int *)buffer)[1];
			DCFlushRange(ptr, 4);
			break;
		}
		case 0x04: { /* cmd_readmem */
			const unsigned char *ptr, *end;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			ptr = ((const unsigned char **)buffer)[0];
			end = ((const unsigned char **)buffer)[1];

			while (ptr != end) {
				int len, i;

				len = end - ptr;
				if (len > 0x400)
					len = 0x400;
				for (i = 0; i < len; i++)
					if (ptr[i] != 0) break;

				if (i == len) { // all zero!
					ret = sendbyte(bss, clientfd, 0xb0);
					CHECK_ERROR(ret < 0);
				} else {
					memcpy(buffer + 1, ptr, len);
					buffer[0] = 0xbd;
					ret = sendwait(bss, clientfd, buffer, len + 1);
					CHECK_ERROR(ret < 0);
				}

				ret = checkbyte(bss, clientfd);
				if (ret == 0xcc) /* GCFAIL */
					goto next_cmd;
				ptr += len;
			}
			break;
		}
		case 0x0b: { /* cmd_writekern */
			void *ptr, * value;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((void **)buffer)[0];
			value = ((void **)buffer)[1];

			kern_write(ptr, (uint32_t)value);
			break;
		}
		case 0x0c: { /* cmd_readkern */
			void *ptr, *value;
			ret = recvwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);

			ptr = ((void **)buffer)[0];

			value = (void*)kern_read(ptr);

			*(void **)buffer = value;
			sendwait(bss, clientfd, buffer, 4);
			break;
		}
		case 0x41: { /* cmd_upload */
			unsigned char *ptr, *end, *dst;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			ptr = ((unsigned char **)buffer)[0];
			end = ((unsigned char **)buffer)[1];

			while (ptr != end) {
				int len;

				len = end - ptr;
				if (len > 0x400)
					len = 0x400;
				if ((int)ptr >= 0x10000000 && (int)ptr <= 0x50000000) {
					dst = ptr;
				} else {
					dst = buffer;
				}
				ret = recvwait(bss, clientfd, dst, len);
				CHECK_ERROR(ret < 0);
				if (dst == buffer) {
					memcpy(ptr, buffer, len);
				}

				ptr += len;
			}

			sendbyte(bss, clientfd, 0xaa); /* GCACK */
			break;
		}
		case 0x50: { /* cmd_status */
			ret = sendbyte(bss, clientfd, 1); /* running */
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x70: { /* cmd_rpc */
			long long (*fun)(int, int, int, int, int, int, int, int);
			int r3, r4, r5, r6, r7, r8, r9, r10;
			long long result;

			ret = recvwait(bss, clientfd, buffer, 4 + 8 * 4);
			CHECK_ERROR(ret < 0);

			fun = ((void **)buffer)[0];
			r3 = ((int *)buffer)[1];
			r4 = ((int *)buffer)[2];
			r5 = ((int *)buffer)[3];
			r6 = ((int *)buffer)[4];
			r7 = ((int *)buffer)[5];
			r8 = ((int *)buffer)[6];
			r9 = ((int *)buffer)[7];
			r10 = ((int *)buffer)[8];

			result = fun(r3, r4, r5, r6, r7, r8, r9, r10);

			((long long *)buffer)[0] = result;
			ret = sendwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x71: { /* cmd_getsymbol */
			int size = recvbyte(bss, clientfd);
			CHECK_ERROR(size < 0);
			ret = recvwait(bss, clientfd, buffer, size);
			CHECK_ERROR(ret < 0);

			/* Identify the RPL name and symbol name */
			char *rplname = (char*) &((int*)buffer)[2];
			char *symname = (char*) (&buffer[0] + ((int*)buffer)[1]);

			/* Get the symbol and store it in the buffer */
			unsigned int module_handle, function_address;
			OSDynLoad_Acquire(rplname, &module_handle);

			char data = recvbyte(bss, clientfd);
			OSDynLoad_FindExport(module_handle, data, symname, &function_address);

			((int*)buffer)[0] = (int)function_address;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x72: { /* cmd_search32 */
			ret = recvwait(bss, clientfd, buffer, 12);
			CHECK_ERROR(ret < 0);
			int addr = ((int *) buffer)[0];
			int val = ((int  *) buffer)[1];
			int size = ((int *) buffer)[2];
			int i;
			int resaddr = 0;
			for(i = addr; i < (addr+size); i+=4)
			{
				if(*(int*)i == val)
				{
					resaddr = i;
					break;
				}
			}
			((int *)buffer)[0] = resaddr;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x80: { /* cmd_rpc_big */
			long long (*fun)(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
			int r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18;
			long long result;

			ret = recvwait(bss, clientfd, buffer, 4 + 16 * 4);
			CHECK_ERROR(ret < 0);

			fun = ((void **)buffer)[0];
			r3  = ((int *)buffer)[1];
			r4  = ((int *)buffer)[2];
			r5  = ((int *)buffer)[3];
			r6  = ((int *)buffer)[4];
			r7  = ((int *)buffer)[5];
			r8  = ((int *)buffer)[6];
			r9  = ((int *)buffer)[7];
			r10 = ((int *)buffer)[8];
			r11 = ((int *)buffer)[9];
			r12 = ((int *)buffer)[10];
			r13 = ((int *)buffer)[11];
			r14 = ((int *)buffer)[12];
			r15 = ((int *)buffer)[13];
			r16 = ((int *)buffer)[14];
			r17 = ((int *)buffer)[15];
			r18 = ((int *)buffer)[16];

			result = fun(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18);

			((long long *)buffer)[0] = result;
			ret = sendwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x99: { /* cmd_version */
			ret = sendbyte(bss, clientfd, 0x82); /* WiiU */
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x9A: { /* cmd_os_version */
			((int *)buffer)[0] = (int)OS_FIRMWARE;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0xcc: { /* GCFAIL */
			break;
		}
		default:
			ret = -1;
			CHECK_ERROR(0);
			break;
		}
next_cmd:
		continue;
	}
	return 0;
error:
	bss->error = ret;
	return 0;
}

static int pygecko_main(int argc, void *argv) {
	int sockfd = -1, clientfd = -1, ret, len;
	struct sockaddr_in addr;
	struct pygecko_bss_t *bss = argv;

	socket_lib_init();

	while (1) {
		addr.sin_family = AF_INET;
		addr.sin_port = 7331;
		addr.sin_addr.s_addr = 0;

		sockfd = ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHECK_ERROR(ret == -1);
        int enable = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
		ret = bind(sockfd, (void *)&addr, 16);
		CHECK_ERROR(ret < 0);
		ret = listen(sockfd, 1);
		CHECK_ERROR(ret < 0);

		while(1)
        {
            len = 16;
            clientfd = ret = accept(sockfd, (void *)&addr, &len);
            CHECK_ERROR(ret == -1);
            rungecko(bss, clientfd);
            socketclose(clientfd);
            clientfd = -1;
        }
		continue;
error:
		if (clientfd != -1)
			socketclose(clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
        clientfd = -1;
        sockfd = -1;
		bss->error = ret;
		usleep(100000);
	}
	return 0;
}

void start_pygecko(void)
{
	struct pygecko_bss_t *bss;

	bss = memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss == 0)
		return;
	memset(bss, 0, sizeof(struct pygecko_bss_t));

	if(OSCreateThread(&bss->thread, pygecko_main, 1, bss, (u32)bss->stack + sizeof(bss->stack), sizeof(bss->stack), 0, 0x1C) == 1)
    {
        OSResumeThread(&bss->thread);
    }
    else
    {
        free(bss);
    }
}
