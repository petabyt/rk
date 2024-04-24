// Example code by Rockchip for building a translation table on compile time

// Put a 64-bit value with little endianness.
.macro PUT_64B high, low
	.word \low
	.word \high
.endm

// Create an entry pointing to a next-level table.
.macro TABLE_ENTRY PA, ATTR
	PUT_64B \ATTR, (\PA) + 0x3
.endm

// Create an entry for a 1GB block.
.macro BLOCK_1GB PA, ATTR_HI, ATTR_LO
PUT_64B \ATTR_HI, ((\PA) & 0xC0000000) | \ATTR_LO | 0x1
.endm

// Create an entry for a 2MB block.
.macro BLOCK_2MB PA, ATTR_HI, ATTR_LO
	PUT_64B \ATTR_HI, ((\PA) & 0xFFE00000) | \ATTR_LO | 0x1
.endm

.align 12 // 12 for 4KB granule.
ttb0_base:
	TABLE_ENTRY level2_pagetable, 0
	BLOCK_1GB 0x40000000, 0, 0x740
	BLOCK_1GB 0x80000000, 0, 0x740
	BLOCK_1GB 0xC0000000, 0, 0x740

.align 12 // 12 for 4KB granule.
level2_pagetable:
	.set ADDR, 0x000 // The current page address.
	.rept 0x200
	BLOCK_2MB (ADDR << 20), 0, 0x74C
	.set ADDR, ADDR+2
	.endr
