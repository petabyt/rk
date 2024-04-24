// Bare metal binary loader - meant to be at origin 0
// Load boot.bin to 2mb out
.global _start
_start:
	mov x0, xzr
	mov x2, xzr
	ldr w0, os_bin_size
	mov x1, #0
	ldr w2, orig
	adr x3, os_bin_start
	top:
		ldrb w4, [x3, x1] // load from src
		strb w4, [x2, x1] // store to dest
		add x1, x1, #0x1
		cmp x1, x0 // i != size
	bne top

	// Now jump there
	ldr x2, orig
	blr x2

os_bin_size: .int os_bin_end - os_bin_start
orig: .int 0xf0000000

os_bin_start:
.incbin "os.bin"
os_bin_end:
