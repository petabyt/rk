#ifndef RK_HID_KEYBOARD_H
#define RK_HID_KEYBOARD_H

#include <stdint.h>

void hid_keyboard_reset(void);
void hid_keyboard_report(const uint8_t report[8]);

#endif
