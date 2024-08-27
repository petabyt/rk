#if 0
Page 1470
#endif

.global _start
_start:
b entry
.int 0x0
.int 0x4b415352
.int 0x1f8
entry:
	ldr w0, uart_base

#if 1
	mov w1, #0x83
	str w1, [x0, #0xc] // lcr
	mov w1, #0x1 // 15000000
	str w1, [x0, #0x0] // rbr
	mov w1, #0x3
	str w1, [x0, #0xc] // lcr

	mov w1, #0x1
	str w1, [x0, #0x98] // shadow fifo
#endif

	mov w1, #'A'
	str w1, [x0, #0x0] // rbr

	ret

uart_base: .int 0xfeb50000
gpio1_base: .int 0xFEC20000
gpio3_base: .int 0xFEC40000
