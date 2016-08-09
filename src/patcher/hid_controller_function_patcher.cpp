#include "hid_controller_function_patcher.h"
#include "common/retain_vars.h"
#include "settings/SettingsEnums.h"
#include "video/CursorDrawer.h"
#include "controller_patcher/cp_retain_vars.h"
#include "utils/logger.h"

DECL(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){
    if(gHIDCurrentDevice & gHID_LIST_MOUSE && gHID_Mouse_Mode == HID_MOUSE_MODE_TOUCH) {
        CursorDrawer::draw(gHID_Mouse.pad_data[0].data[0].X, gHID_Mouse.pad_data[0].data[0].Y);
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer,scan_target);
}

DECL(void, _Exit, void){
    CursorDrawer::destroyInstance();
    real__Exit();
}

DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {

    int result = real_VPADRead(chan, buffer, buffer_size, error);


    if((gHIDPADEnabled == SETTING_ON) && gHIDAttached){
        setControllerDataFromHID(buffer,HID_ALL_CONNECTED_DEVICES);
    }

    if((gHIDPADEnabled == SETTING_ON) && (gButtonRemappingConfigDone && gConfig_done)){
        buttonRemapping(buffer);
        if (HID_DEBUG) printButtons(buffer);
    }

    if(gSettingPadconMode == SETTING_ON){
        if(buffer->btns_r&VPAD_BUTTON_STICK_R) {
            int mode;
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
    MAKE_MAGIC(_Exit,                               LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_hid_controller __attribute__((section(".data"))) = sizeof(method_hooks_hid_controller) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_hid_controller[sizeof(method_hooks_hid_controller) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

