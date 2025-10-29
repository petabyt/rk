#ifndef PINE_BOOT
#define PINE_BOOT

#define DUMMY_ALLOC_BASE 0x20000000
#define STACK_BASE 0x20000000
#define FB_ADDR 0xF7800000

#ifndef __ASM__
#include <stdint.h>

// Base ttbr0 table defined in assembly
extern uint8_t ttb0_base[];

/// Get platform's preferred gpio address
volatile void *plat_get_uart_base(void);
void enable_uart(void);

/// Get address for where framebuffer should be stored (should be setup as noncache memory)
uint32_t *plat_get_framebuffer(void);
void plat_shutdown(void);
void plat_reset(void);
/// Function that implements platform-specific firmware calls
uint64_t plat_process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3);
/// Implements base firmware calls (PSCI and such)
uint64_t process_firmware_call(uint64_t p1, uint64_t p2, uint64_t p3);
/// Hand over control to the payload
void jump_to_payload(void);

/// Set direction (IN/OUT of a pin)
void gpio_set_dir(int gpio, int pin, int bit);
/// Set value (high/low) of a pin
void gpio_set_pin(int gpio, int pin, int bit);
/// Mask interrupt of a pin
void gpio_pin_mask_int(int gpio, int pin);
/// Read value of a pin
int gpio_get_pin(int gpio, int pin);

// Rockchip register design: in order to write bit x, bit x + 16 must be 1.
// If bit x is 1 but x + 16 is 0, the write will be denied.
// In this func: rk_clr_set_bits(&a, 5, 0, 0x0); == sets bits [5:0] of ptr a
void rk_clr_set_bits(volatile void *d, int bit_end, int bit_start, int v);

// asm.S, boot.S
void back_to_bootrom(void);
void asm_enable_ints(void);
void asm_enable_int_groups(int scratch);
void asm_disable_ints(void);
uint64_t asm_get_el_es(void);
uint64_t asm_get_mpidr(void);
uint32_t asm_get_el(void);
void setup_tt_el3(uint64_t tcr, uint64_t mair, uintptr_t ttbr0);
void enable_mmu_el3(void);
void disable_mmu_el3(void);
/// Enable data cache coherency between all CPU cores of all clusters
void asm_enable_smp_cache_coherency(void);

// timer.c
void reset_timer0(void);
void reset_timer(int t, uint64_t start, uint64_t limit);
int timer_check_int(int t);
uint32_t timer_get32(int t);
uint32_t timer0_get_val(void);
void msleep(int ms);
void usleep(int us);

// edp.c
void sys_turn_on_screen(void);
//void edp_shutdown(void);
int edp_init(uintptr_t edp_addr);
int edp_enable(uintptr_t edp_addr, uint32_t link_rate, uint32_t lane_count);

// lib.c
void halt(void);
void blink_bits(uint32_t ptr);
void nop_sleep(void);
void nop_sleep_short(void);
void itoa(uint64_t n, char *buffer, int base);
void *memset32(void *dest, int val, long unsigned int len);
void cheap_memdump(uint8_t *addr, int n);

// uart.c, lib.c
void uart_init(void);
int uart_get(void);
void uart_chr(int c);
int putchar(int c);
void memdump(uint8_t *addr, int n);
void debug(char *str, uint64_t reg);
void sdebug(char *buf, char *str, uint64_t reg);
int puts(const char *str);
void fail(char *reason, int code);

int sys_soc_setup(void);

int setup_ohci(uintptr_t base);

//int sd_setup(void);

//void boot_uboot(void);

#endif

// https://github.com/torvalds/linux/blob/05d3ef8bba77c1b5f98d941d8b2d4aeab8118ef1/include/dt-bindings/pinctrl/rockchip.h
#define RK_PIN_A0 0
#define RK_PIN_A1 1
#define RK_PIN_A2 2
#define RK_PIN_A3 3
#define RK_PIN_A4 4
#define RK_PIN_A5 5
#define RK_PIN_A6 6
#define RK_PIN_A7 7

#define RK_PIN_B0 8
#define RK_PIN_B1 9
#define RK_PIN_B2 10
#define RK_PIN_B3 11
#define RK_PIN_B4 12
#define RK_PIN_B5 13
#define RK_PIN_B6 14
#define RK_PIN_B7 15

#define RK_PIN_C0 16
#define RK_PIN_C1 17
#define RK_PIN_C2 18
#define RK_PIN_C3 19
#define RK_PIN_C4 20
#define RK_PIN_C5 21
#define RK_PIN_C6 22
#define RK_PIN_C7 23

#define RK_PIN_D0 24
#define RK_PIN_D1 25
#define RK_PIN_D2 26
#define RK_PIN_D3 27
#define RK_PIN_D4 28
#define RK_PIN_D5 29
#define RK_PIN_D6 30
#define RK_PIN_D7 31

#endif
