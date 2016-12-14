#include <string.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "utils/function_patcher.h"
#include "patcher/cpp_to_c_util.h"
#include "controller_patcher/controller_patcher.h"
#include "patcher/pygecko.h"
#include "common/common.h"
#include "common/retain_vars.h"
#include "utils/utils.h"
#include "utils/logger.h"
#include "main.h"


int __entry_menu(int argc, char **argv)
{
    //! Launch PyGecko if requested
    if (GAME_LAUNCHED && gSettingLaunchPyGecko)
    {
        start_pygecko();
    }
    InitOSFunctionPointers();
    InitSocketFunctionPointers();
    InitAocFunctionPointers();
    InitACPFunctionPointers();


    log_init("192.168.0.181");

    //!*******************************************************************
    //!                        Initialize HID Config                     *
    //!*******************************************************************
    init_config_controller();
    init_button_remapping();

    //!*******************************************************************
    //!                        Dynamic Patching                          *
    //!*******************************************************************

    if(GAME_LAUNCHED){
         ApplyPatches();
    }

    //! *******************************************************************
    //! *              Check if our application is started                *
    //! *******************************************************************
    if (OSGetTitleID != 0 &&
        OSGetTitleID() != 0x000500101004A200 && // mii maker eur
        OSGetTitleID() != 0x000500101004A100 && // mii maker usa
        OSGetTitleID() != 0x000500101004A000 && // mii maker jpn
        OSGetTitleID() != 0x0005000013374842)   // hbl channel
    {
        return EXIT_RELAUNCH_ON_LOAD;
    }

    //!*******************************************************************
    //!                       Check game launch                          *
    //!*******************************************************************
    // check if game is launched, if yes continue coreinit process
    if ((GAME_LAUNCHED == 1) && (LOADIINE_MODE == LOADIINE_MODE_MII_MAKER))
        return EXIT_RELAUNCH_ON_LOAD;

    //! *******************************************************************
    //! *                     Setup EABI registers                        *
    //! *******************************************************************
    register int old_sdata_start, old_sdata2_start;
    asm volatile(
        "mr %0, 13\n"
        "mr %1, 2\n"
        "lis 2, __sdata2_start@h\n"
        "ori 2, 2,__sdata2_start@l\n"  // Set the Small Data 2 (Read Only) base register.
        "lis 13, __sdata_start@h\n"
        "ori 13, 13, __sdata_start@l\n"// # Set the Small Data (Read\Write) base register.
        : "=r" (old_sdata_start), "=r" (old_sdata2_start)
    );

    //!*******************************************************************
    //!                    Initialize BSS sections                       *
    //!*******************************************************************
    asm volatile(
        "lis 3, __bss_start@h\n"
        "ori 3, 3,__bss_start@l\n"
        "lis 5, __bss_end@h\n"
        "ori 5, 5, __bss_end@l\n"
        "subf 5, 3, 5\n"
        "li 4, 0\n"
        "bl memset\n"
    );

    //! *******************************************************************
    //! *                        Call our Main                            *
    //! *******************************************************************
    Menu_Main();

    //! *******************************************************************
    //! *                    Restore EABI registers                       *
    //! *******************************************************************
    asm volatile("mr 13, %0" : : "r" (old_sdata_start));
    asm volatile("mr 2,  %0" : : "r" (old_sdata2_start));

    if(GAME_LAUNCHED)
    {
        if (LOADIINE_MODE == LOADIINE_MODE_SMASH_BROS)
        {
            if(SYSCheckTitleExists(0x0005000010145000)) { // EUR
                SYSLaunchTitle(0x0005000010145000);
            }
            else if(SYSCheckTitleExists(0x0005000010144F00)) { // USA
                SYSLaunchTitle(0x0005000010144F00);
            }
            else if(SYSCheckTitleExists(0x0005000010110E00)) { // JAP
                SYSLaunchTitle(0x0005000010110E00);
            }
            else {
                // Launch smash bros disk without exiting to menu
                char buf_vol_odd[20];
                strcpy(buf_vol_odd, "/vol/storage_odd03");
                _SYSLaunchTitleByPathFromLauncher(buf_vol_odd, 18, 0);
            }
        }
        else if(LOADIINE_MODE == LOADIINE_MODE_MII_MAKER)
        {
            // Restart mii maker
            SYSRelaunchTitle(0, 0);
            __Exit();
        }
        //! TODO: add auto launch with SYSLaunchTitle for Karaoke and Art Atelier Modes

        //! *******************************************************************
        //! *                 Jump to original application                    *
        //! *******************************************************************
        return EXIT_RELAUNCH_ON_LOAD;
    }

    RestoreAllInstructions();

    deinit_config_controller();

    //! *******************************************************************
    //! *                 Jump to homebrew launcher                       *
    //! *******************************************************************
    return EXIT_SUCCESS;
}
