#ifndef PINE_BOOT
#define PINE_BOOT

#define CRAP_ALLOC_BASE 0x20000000
#define STACK_BASE 0x20000000
#define FB_ADDR 0xF7800000

#ifndef __ASM__
#include <stdint.h>

volatile void *get_uart_base();
void enable_uart();

volatile void *get_gicr_base();
volatile void *get_gicd_base();
volatile void *get_gicc_base();

// io.c
void gpio_set_dir(int gpio, int pin, int bit);
void gpio_set_pin(int gpio, int pin, int bit);
void gpio_pin_mask_int(int gpio, int pin);
int gpio_get_pin(int gpio, int pin);
void grf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);
void pmugrf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);

// asm.S, boot.S
void run_32_bit(void *ptr);
void back_to_bootrom();
void asm_enable_ints();
void asm_enable_int_groups(int scratch);
void asm_disable_ints();
uint64_t asm_get_el_es();
uint64_t asm_get_vector_base();
uint64_t asm_get_mpidr();
uint32_t asm_get_el();
void halt();
void uart_init();
void asm_svc();
void disable_mmu_el3(int scratch);

// gic.c
void gic_enable_irq(int n);
int gic_get_int();
void gic_init();

// timer.c
void reset_timer0();
void reset_timer(int t, uint64_t start, uint64_t limit);
int timer_check_int(int t);
uint32_t timer_get32(int t);
uint32_t timer0_get_val();
void msleep(int ms);
void usleep(int us);

// clock.c
void setup_cru();
void clock_set_pll(uint32_t *cons, uint32_t refdiv, uint32_t fbdiv, uint32_t postdiv1, uint32_t postdiv2);

// edp.c
void sys_turn_on_screen();
void edp_shutdown();
uint32_t *sys_get_framebuffer();

// lib.c
void blink_bits(uint32_t ptr);
void nop_sleep();
void nop_sleep_short();
void itoa(uint64_t n, char *buffer, int base);
void *memset32(void *dest, int val, long unsigned int len);
void cheap_memdump(uint8_t *addr, int n);

// uart.c, lib.c
int putchar(int c);
void memdump(uint8_t *addr, int n);
void uart_str(char *x);
void debug(char *str, uint64_t reg);
void debugf(char *buf, char *str, uint64_t reg);
int puts(const char *str);
void fail(char *reason, int code);

int sys_soc_setup();

int font_print_string(int x, int y, char *string);
int font_print_char(int x, int y, char c);
int bmp_print_char(char c);
void bmp_clear();
void bmp_draw_rect(int x, int y, int w, int h, int col);

int setup_ohci(uintptr_t base);

int sd_setup();

void boot_uboot();

#endif

#endif
