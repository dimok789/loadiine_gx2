/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "os_functions.h"
#include "socket_functions.h"

EXPORT_DECL(void, socket_lib_init, void);
EXPORT_DECL(int, socket, int domain, int type, int protocol);
EXPORT_DECL(int, socketclose, int s);
EXPORT_DECL(int, connect, int s, void *addr, int addrlen);
EXPORT_DECL(int, send, int s, const void *buffer, int size, int flags);
EXPORT_DECL(int, recv, int s, void *buffer, int size, int flags);
EXPORT_DECL(int, sendto, int s, const void *buffer, int size, int flags, const struct sockaddr *dest, int dest_len);
EXPORT_DECL(int, setsockopt, int s, int level, int optname, void *optval, int optlen);
EXPORT_DECL(char *, inet_ntoa, struct in_addr in);
EXPORT_DECL(int, inet_aton, const char *cp, struct in_addr *inp);




void InitSocketFunctionPointers(void)
{
    unsigned int nsysnet_handle;
    unsigned int *funcPointer = 0;
    OSDynLoad_Acquire("nsysnet.rpl", &nsysnet_handle);

#if defined(LOADIINE_USE_AUTOCONNECT)
    unsigned int nn_ac_handle;
    int(*ACInitialize)();
    int(*ACGetStartupId) (unsigned int *id);
    int(*ACConnectWithConfigId) (unsigned int id);
    OSDynLoad_Acquire("nn_ac.rpl", &nn_ac_handle);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACInitialize", &ACInitialize);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACGetStartupId", &ACGetStartupId);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACConnectWithConfigId",&ACConnectWithConfigId);
#else
     #pragma message "socket_functions: AC (AutoConnect) will not be used!!!"
#endif // LOADIINE_USE_AUTOCONNECT

    OS_FIND_EXPORT(nsysnet_handle, socket_lib_init);
    OS_FIND_EXPORT(nsysnet_handle, socket);
    OS_FIND_EXPORT(nsysnet_handle, socketclose);
    OS_FIND_EXPORT(nsysnet_handle, connect);
    OS_FIND_EXPORT(nsysnet_handle, send);
    OS_FIND_EXPORT(nsysnet_handle, recv);
    OS_FIND_EXPORT(nsysnet_handle, sendto);
    OS_FIND_EXPORT(nsysnet_handle, setsockopt);
    OS_FIND_EXPORT(nsysnet_handle, inet_ntoa);
    OS_FIND_EXPORT(nsysnet_handle, inet_aton);


#if defined(LOADIINE_USE_AUTOCONNECT)
     unsigned int nn_startupid;
     ACInitialize();
     ACGetStartupId(&nn_startupid);
     ACConnectWithConfigId(nn_startupid);
#endif // LOADIINE_USE_AUTOCONNECT
 
     socket_lib_init();
}

