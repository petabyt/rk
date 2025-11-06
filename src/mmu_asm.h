// C Macros to define a translation table in assembly
#define PUT64(high, low) .word low; .word high;
#define ATTR(mair_i) (0x740 | (mair_i << 2))
#define BLOCK_1GB(output_addr, mair_i) PUT64(0, ((output_addr) & 0xc0000000) | ATTR(mair_i) | 1)
#define BLOCK_2MB(output_addr, mair_i) PUT64(0, ((output_addr) & 0xffe00000) | ATTR(mair_i) | 1)
#define TABLE_ENTRY(output_addr) PUT64(0, (output_addr) + 3)
