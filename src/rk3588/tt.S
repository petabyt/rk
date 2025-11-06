#include <mmu_asm.h>

.global mair_reg_base
// attr0: ngnrne device memory
// attr1: ngnre device memory
// attr2: NonCacheable
// attr3: WriteBack_NonTransient_ReadWriteAlloc
mair_reg_base: PUT64(0, 0xff440400)

.align 12
.global ttb0_base
ttb0_base:
	BLOCK_1GB(0x00000000, 3)
	BLOCK_1GB(0x40000000, 3)
	BLOCK_1GB(0x80000000, 3)
	BLOCK_1GB(0xc0000000, 0)
	BLOCK_1GB(0x100000000, 0)
	BLOCK_1GB(0x140000000, 3)
	BLOCK_1GB(0x180000000, 3)
	BLOCK_1GB(0x1c0000000, 3)

