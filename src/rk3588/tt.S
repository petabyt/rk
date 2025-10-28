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

#define ATTR(mair_i) (0x740 | (mair_i << 2))

.align 12 // 12 for 4KB granule.
.global ttb0_base
ttb0_base:
	BLOCK_1GB 0x00000000, 0, 0x74C
	BLOCK_1GB 0x40000000, 0, 0x74C
	BLOCK_1GB 0x80000000, 0, 0x74C
	BLOCK_1GB 0xC0000000, 0, 0x740
