static void MmioWrite32(uintptr_t addr, uint32_t v) {
  ((volatile uint32_t *)addr)[0] = v;
}
static uint32_t MmioRead32(uintptr_t addr) {
	return ((volatile uint32_t *)addr)[0];
}
static void NanoSecondDelay(uint32_t ns) {
	usleep(ns / 1000);
}
static void MicroSecondDelay(uint32_t us) {
	usleep(us);
}

#define TRUE 1
#define FALSE 0

#define BIT(x) (1 << (x))
#define BITS_PER_LONG 	(sizeof(unsigned long) * 8)
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#define EFI_TIMEOUT -1
#define EFI_UNSUPPORTED -2
#define EFI_DEVICE_ERROR -3
#define EREMOTEIO       121     /* Remote I/O error */
#define EINVAL          22      /* Invalid argument */
#define ETIMEDOUT       110     /* Connection timed out */

uint32_t HdptxRopllTmdsModeConfig(int id);
