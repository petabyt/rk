#include <stdint.h>
#include "os.h"

// Rockchip register design: in order to write bit x, bit x + 16 must be 1.
// If bit x + 16 is 0, the write will be denied.
// In this func: rk_clr_set_bits(&a, 5, 0, 0x0); == sets bits [5:0] of ptr a
void rk_clr_set_bits(uint32_t *d, int bit_end, int bit_start, int v) {
	uint32_t temp = 0;

	// Write the requested bits at their location
	temp |= (v << (bit_start));

	// Write the corrosponding bits in the write_bit field (31:16)
	// So we generate a mask of 1s, and shift it over to the corrosponding bits
	temp |= ((1 << (bit_end - bit_start + 1)) - 1) << (16 + bit_start);

	d[0] = temp;
}
