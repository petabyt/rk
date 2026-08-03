#include <string.h>
#include "hid_keyboard.h"
#include "input.h"

static uint8_t previous[8];
static int caps_lock;

static int was_pressed(uint8_t usage) {
	for (int i = 2; i < 8; i++)
		if (previous[i] == usage)
			return 1;
	return 0;
}

static uint8_t translate(uint8_t usage, int shift) {
	static const char plain[] = "1234567890-=[]\\;\'`,./";
	static const char shifted[] = "!@#$%^&*()_+{}|:\"~<>?";

	if (usage >= 0x04 && usage <= 0x1d) {
		int upper = shift ^ caps_lock;
		return (uint8_t)((upper ? 'A' : 'a') + usage - 0x04);
	}
	if ((usage >= 0x1e && usage <= 0x27) ||
		(usage >= 0x2d && usage <= 0x38)) {
		unsigned int index;
		if (usage <= 0x27)
			index = usage - 0x1e;
		else
			index = 10 + usage - 0x2d;
		if (index < sizeof(plain) - 1)
			return (uint8_t)(shift ? shifted[index] : plain[index]);
	}
	switch (usage) {
	case 0x28: return '\r';
	case 0x29: return 0x1b;
	case 0x2a: return '\b';
	case 0x2b: return '\t';
	case 0x2c: return ' ';
	default: return 0;
	}
}

void hid_keyboard_reset(void) {
	memset(previous, 0, sizeof(previous));
	caps_lock = 0;
}

void hid_keyboard_report(const uint8_t report[8]) {
	int shift = (report[0] & 0x22) != 0;

	/* ErrorRollOver, POSTFail and ErrorUndefined invalidate the report. */
	for (int i = 2; i < 8; i++)
		if (report[i] >= 1 && report[i] <= 3)
			return;

	/* Ctrl, Alt and GUI-modified input has no printable firmware mapping. */
	if (report[0] & 0xdd) {
		memcpy(previous, report, sizeof(previous));
		return;
	}

	for (int i = 2; i < 8; i++) {
		uint8_t usage = report[i];
		uint8_t c;
		if (!usage || was_pressed(usage))
			continue;
		if (usage == 0x39) {
			caps_lock = !caps_lock;
			continue;
		}
		c = translate(usage, shift);
		if (c)
			input_enqueue(c);
	}
	memcpy(previous, report, sizeof(previous));
}
