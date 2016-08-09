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
#include "patcher/fs_patcher.h"
#include "patcher/fs_sd_patcher.h"
#include "patcher/rplrpx_patcher.h"
#include "patcher/extra_log_patcher.h"
#include "patcher/hid_controller_function_patcher.h"
#include "patcher/aoc_patcher.h"
#include "controller_patcher/cp_retain_vars.h"
#include "controller_patcher/config_reader.h"
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
#include "main.h"

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

    InitAocFunctionPointers();
    InitACPFunctionPointers();



    log_printf("Function exports loaded\n");

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
    //!                       Read Configs for HID support               *
    //!*******************************************************************
    if(gConfig_done == HID_INIT_DONE){
        log_print("Reading config files from SD Card\n");
        ConfigReader::getInstance(); //doing the magic automatically
        ConfigReader::destroyInstance();
        log_print("Done with reading config files from SD Card\n");
        gConfig_done = HID_SDCARD_READ;
    }

    //!*******************************************************************
    //!                       Patch Functions                            *
    //!*******************************************************************
    log_printf("Patch FS and loader functions\n");
    ApplyPatches();
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
    //log_deinit();

    return 0;
}

void ApplyPatches(){
    log_print("Patching FS functions\n");
    PatchInvidualMethodHooks(method_hooks_fs,                   method_hooks_size_fs,               method_calls_fs);
    log_print("Patching functions for AOC support\n");
    PatchInvidualMethodHooks(method_hooks_aoc,                  method_hooks_size_aoc,              method_calls_aoc);
    log_print("Patching more FS functions (SD)\n");
    PatchInvidualMethodHooks(method_hooks_fs_sd,                method_hooks_size_fs_sd,            method_calls_fs_sd);
    log_print("Patching functions for RPX/RPL loading\n");
    PatchInvidualMethodHooks(method_hooks_rplrpx,               method_hooks_size_rplrpx,           method_calls_rplrpx);
    log_print("Patching extra log functions\n");
    PatchInvidualMethodHooks(method_hooks_extra_log,            method_hooks_size_extra_log,        method_calls_extra_log);
    log_print("Patching controller_patcher (Hid to VPAD)\n");
    PatchInvidualMethodHooks(method_hooks_hid_controller,       method_hooks_size_hid_controller,   method_calls_hid_controller);
}

void RestoreAllInstructions(){
    log_print("Restoring FS functions\n");
    RestoreInvidualInstructions(method_hooks_fs,                method_hooks_size_fs);
    log_print("Restoring functions for AOC support\n");
    RestoreInvidualInstructions(method_hooks_aoc,               method_hooks_size_aoc);
    log_print("Restoring more FS functions (SD)\n");
    RestoreInvidualInstructions(method_hooks_fs_sd,             method_hooks_size_fs_sd);
    log_print("Restoring functions for RPX/RPL loading\n");
    RestoreInvidualInstructions(method_hooks_rplrpx,            method_hooks_size_rplrpx);
    log_print("Restoring extra log functions\n");
    RestoreInvidualInstructions(method_hooks_extra_log,         method_hooks_size_extra_log);
    log_print("Restoring controller_patcher (Hid to VPAD)\n");
    RestoreInvidualInstructions(method_hooks_hid_controller,    method_hooks_size_hid_controller);
}
