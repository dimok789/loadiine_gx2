#include <stdio.h>
#include <string.h>
#include "hid_controller_function_patcher.h"
#include "common/retain_vars.h"
#include "settings/SettingsEnums.h"
#include "video/CursorDrawer.h"
#include "utils/logger.h"
#include "controller_patcher/ControllerPatcher.hpp"

DECL(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){
    if(gHIDCurrentDevice & gHID_LIST_MOUSE && gHID_Mouse_Mode == HID_MOUSE_MODE_TOUCH) {
        HID_Mouse_Data * mouse_data = ControllerPatcher::getMouseData();
        if(mouse_data !=  NULL){
            CursorDrawer::draw(mouse_data->X, mouse_data->Y);
        }
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer,scan_target);
}

DECL(void, __PPCExit, void){
    CursorDrawer::destroyInstance();

    ControllerPatcher::destroyConfigHelper();
    ControllerPatcher::stopNetworkServer();

    memset(gWPADConnectCallback,0,sizeof(gWPADConnectCallback));
    memset(gKPADConnectCallback,0,sizeof(gKPADConnectCallback));
    memset(gExtensionCallback,0,sizeof(gExtensionCallback));
    gSamplingCallback = 0;
    gCallbackCooldown = 0;

    real___PPCExit();
}

DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {
    int result = real_VPADRead(chan, buffer, buffer_size, error);

    if((gHIDPADEnabled == SETTING_ON) && gHIDAttached && buffer_size > 0){
        ControllerPatcher::setRumble(UController_Type_Gamepad,!!VPADBASEGetMotorOnRemainingCount(0));

        if(ControllerPatcher::setControllerDataFromHID(buffer) == CONTROLLER_PATCHER_ERROR_NONE){

            if(buffer[0].btns_h & VPAD_BUTTON_HOME){
                //You can open the home menu this way, but not close it. Need a proper way to close it using the same button...
                //OSSendAppSwitchRequest(5,0,0); //Open the home menu!
            }

            if(error != NULL){
                *error = 0;
            }
            result = 1; // We want the WiiU to ignore everything else.
        }
        if(gButtonRemappingConfigDone){
            ControllerPatcher::buttonRemapping(buffer,result);
        }
    }

    if(gSettingPadconMode == SETTING_ON){
        if(buffer->btns_r&VPAD_BUTTON_STICK_R) {
            s32 mode;
            VPADGetLcdMode(0, &mode);       // Get current display mode
            if(mode != 1) {
                VPADSetLcdMode(0, 1);       // Turn it off
            }
            else {
                VPADSetLcdMode(0, 0xFF);    // Turn it on
            }
        }
    }
    return result;
}

hooks_magic_t method_hooks_hid_controller[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(VPADRead,                            LIB_VPAD,       STATIC_FUNCTION),
    MAKE_MAGIC(GX2CopyColorBufferToScanBuffer,      LIB_GX2,        STATIC_FUNCTION),
    MAKE_MAGIC(__PPCExit,                           LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_hid_controller __attribute__((section(".data"))) = sizeof(method_hooks_hid_controller) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile u32 method_calls_hid_controller[sizeof(method_hooks_hid_controller) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

