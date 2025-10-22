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
volatile void *get_uart_base(void);
void enable_uart(void);

/// Set direction (IN/OUT of a pin)
void gpio_set_dir(int gpio, int pin, int bit);
/// Set value (high/low) of a pin
void gpio_set_pin(int gpio, int pin, int bit);
/// Mask interrupt of a pin
void gpio_pin_mask_int(int gpio, int pin);
/// Read value of a pin
int gpio_get_pin(int gpio, int pin);

// asm.S, boot.S
void back_to_bootrom(void);
void asm_enable_ints(void);
void asm_enable_int_groups(int scratch);
void asm_disable_ints(void);
uint64_t asm_get_el_es(void);
uint64_t asm_get_vector_base(void);
uint64_t asm_get_mpidr(void);
uint32_t asm_get_el(void);
void halt(void);
//void asm_svc(void);
void setup_tt_el3(uint64_t tcr, uint64_t mair, uintptr_t ttbr0);
void enable_mmu_el3(void);
void disable_mmu_el3(void);

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
uint32_t *sys_get_framebuffer(void);

// lib.c
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
void debugf(char *buf, char *str, uint64_t reg);
int puts(const char *str);
void fail(char *reason, int code);

int sys_soc_setup(void);

int setup_ohci(uintptr_t base);

//int sd_setup(void);

//void boot_uboot(void);

#endif

#endif
