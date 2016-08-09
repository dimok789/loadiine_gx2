#include <gctypes.h>
u8 gSettingLaunchPyGecko __attribute__((section(".data"))) = 0;
u8 gSettingUseUpdatepath __attribute__((section(".data"))) = 0;
u8 gSettingPadconMode __attribute__((section(".data"))) = 0;
u8 gCursorInitDone __attribute__((section(".data"))) = 0;
u8 gPatchSDKDone __attribute__((section(".data"))) = 0;
u8 gHIDPADEnabled __attribute__((section(".data"))) = 0;
u8 gEnableDLC __attribute__((section(".data"))) = 0;
u32 gLoaderPhysicalBufferAddr __attribute__((section(".data"))) = 0;
u32 gLogUDP __attribute__((section(".data"))) = 0;
char gServerIP[16] __attribute__((section(".data")));
