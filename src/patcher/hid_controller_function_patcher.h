#ifndef _HID_CONTROLLER_FUNCTION_PATCHER_H
#define _HID_CONTROLLER_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t method_hooks_hid_controller[];
extern u32 method_hooks_size_hid_controller;
extern volatile unsigned int method_calls_hid_controller[];

#ifdef __cplusplus
}
#endif

#endif /* _HID_CONTROLLER_FUNCTION_PATCHER_H */
