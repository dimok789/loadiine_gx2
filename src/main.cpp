//#include <string>
#include "Application.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/curl_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "patcher/function_hooks.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "kernel/kernel_functions.h"
#include "settings/CSettings.h"
#include "system/exception_handler.h"
#include "system/memory.h"
#include "utils/strings.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "utils/xml.h"
#include "common/common.h"

/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    InitOSFunctionPointers();
    InitSocketFunctionPointers();

    log_init(LOADIINE_LOGGER_IP);
    log_print("Starting Loadiine GX2 " LOADIINE_VERSION "\n");

    InitFSFunctionPointers();
    InitGX2FunctionPointers();
    InitSysFunctionPointers();
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();
    InitAXFunctionPointers();
    InitCurlFunctionPointers();

    log_print("Function exports loaded\n");

    //!*******************************************************************
    //!                Initialize our kernel variables                   *
    //!*******************************************************************
    log_printf("Setup kernel variables\n");
    SetupKernelCallback();
    //!*******************************************************************
    //!                    Initialize heap memory                        *
    //!*******************************************************************
    log_print("Initialize memory management\n");
    memoryInitialize();

    //!*******************************************************************
    //!                        Initialize FS                             *
    //!*******************************************************************
    log_printf("Mount SD partition\n");
    mount_sd_fat("sd");

    //!*******************************************************************
    //!                       Patch Functions                            *
    //!*******************************************************************
    log_printf("Patch FS and loader functions\n");

    PatchMethodHooks();
    PatchSDK();

    //!*******************************************************************
    //!                    Setup exception handler                       *
    //!*******************************************************************
    log_printf("Setup exception handler\n");
    setup_os_exceptions();

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************
    log_printf("Start main application\n");
    Application::instance()->exec();
    log_printf("Main application stopped\n");

    Application::destroyInstance();



    log_printf("Unmount SD\n");
    unmount_sd_fat("sd");
    log_printf("Release memory\n");
    memoryRelease();
    log_printf("Loadiine peace out...\n");
    log_deinit();

    return 0;
}
