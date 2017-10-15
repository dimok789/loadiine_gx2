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
#include "controller_patcher/ControllerPatcher.hpp"
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

    log_init();
    DEBUG_FUNCTION_LINE("Starting Loadiine GX2 " LOADIINE_VERSION "\n");

    InitFSFunctionPointers();
    InitGX2FunctionPointers();
    InitSysFunctionPointers();
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();
    InitAXFunctionPointers();
    InitCurlFunctionPointers();

    InitAocFunctionPointers();
    InitACPFunctionPointers();

    DEBUG_FUNCTION_LINE("Function exports loaded\n");

    //!*******************************************************************
    //!                Initialize our kernel variables                   *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Setup kernel variables\n");
    SetupKernelCallback();
    //!*******************************************************************
    //!                    Initialize heap memory                        *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Initialize memory management\n");
    memoryInitialize();

    //!*******************************************************************
    //!                        Initialize FS                             *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Mount SD partition\n");
    mount_sd_fat("sd");

    //!*******************************************************************
    //!                       Patch Functions                            *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Patch FS and loader functions\n");
    ApplyPatches();
    PatchSDK();

    //!*******************************************************************
    //!                    Setup exception handler                       *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Setup exception handler\n");
    setup_os_exceptions();

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Start main application\n");
    Application::instance()->exec();
    DEBUG_FUNCTION_LINE("Main application stopped\n");

    Application::destroyInstance();

    DEBUG_FUNCTION_LINE("Unmount SD\n");
    unmount_sd_fat("sd");
    DEBUG_FUNCTION_LINE("Release memory\n");
    memoryRelease();
    DEBUG_FUNCTION_LINE("Loadiine peace out...\n");
    //log_deinit();

    return 0;
}

void ApplyPatches(){
    DEBUG_FUNCTION_LINE("Patching FS functions\n");
    PatchInvidualMethodHooks(method_hooks_fs,                   method_hooks_size_fs,               method_calls_fs);
    DEBUG_FUNCTION_LINE("Patching functions for AOC support\n");
    PatchInvidualMethodHooks(method_hooks_aoc,                  method_hooks_size_aoc,              method_calls_aoc);
    DEBUG_FUNCTION_LINE("Patching more FS functions (SD)\n");
    PatchInvidualMethodHooks(method_hooks_fs_sd,                method_hooks_size_fs_sd,            method_calls_fs_sd);
    DEBUG_FUNCTION_LINE("Patching functions for RPX/RPL loading\n");
    PatchInvidualMethodHooks(method_hooks_rplrpx,               method_hooks_size_rplrpx,           method_calls_rplrpx);
    DEBUG_FUNCTION_LINE("Patching extra log functions\n");
    PatchInvidualMethodHooks(method_hooks_extra_log,            method_hooks_size_extra_log,        method_calls_extra_log);
    DEBUG_FUNCTION_LINE("Patching controller_patcher (HID to VPAD)\n");
    PatchInvidualMethodHooks(method_hooks_hid_controller,       method_hooks_size_hid_controller,   method_calls_hid_controller);
}

void RestoreAllInstructions(){
    DEBUG_FUNCTION_LINE("Restoring FS functions\n");
    RestoreInvidualInstructions(method_hooks_fs,                method_hooks_size_fs);
    DEBUG_FUNCTION_LINE("Restoring functions for AOC support\n");
    RestoreInvidualInstructions(method_hooks_aoc,               method_hooks_size_aoc);
    DEBUG_FUNCTION_LINE("Restoring more FS functions (SD)\n");
    RestoreInvidualInstructions(method_hooks_fs_sd,             method_hooks_size_fs_sd);
    DEBUG_FUNCTION_LINE("Restoring functions for RPX/RPL loading\n");
    RestoreInvidualInstructions(method_hooks_rplrpx,            method_hooks_size_rplrpx);
    DEBUG_FUNCTION_LINE("Restoring extra log functions\n");
    RestoreInvidualInstructions(method_hooks_extra_log,         method_hooks_size_extra_log);
    DEBUG_FUNCTION_LINE("Restoring controller_patcher (HID to VPAD)\n");
    RestoreInvidualInstructions(method_hooks_hid_controller,    method_hooks_size_hid_controller);
}
