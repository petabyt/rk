// Minimal code to light green LED
// light_led(scratch, scratch)
.global light_led
light_led:
	#define PIN_B3 11
	mov x1, 0xff720000

	// Set GPIO to pin B3
	add x1, x1, #0x4
	ldr w0, [x1]
	orr w0, w0, (1 << PIN_B3)
	str w0, [x1]

	// Turn on pin B3 bit
	mov x1, 0xff720000
	ldr w0, [x1]
	orr w0, w0, (1 << PIN_B3)
	str w0, [x1]

	ret
