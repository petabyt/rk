#ifndef PINE_BOOT
#define PINE_BOOT
#include "stdint.h"

// io.c
void rk_clr_set_bits(uint32_t *d, int bit_end, int bit_start, int v);
void gpio_set_dir(int gpio, int pin, int bit);
void gpio_set_pin(int gpio, int pin, int bit);
int gpio_get_pin(int gpio, int pin);
void grf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);
void pmugrf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);

// test.S, boot.S
void run_32_bit(void *ptr);
void back_to_bootrom();
uint64_t asm_get_el_es();
uint64_t asm_get_vector_base();
uint32_t asm_get_cpu_id();
uint32_t asm_get_el();
void halt();
void uart_init();
void asm_svc();

// timer.c
void reset_timer0();
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

// uart.c
void memdump(uint8_t *addr, int n);
void uart_str(char *x);
void debug(char *str, uint64_t reg);
int puts(const char *str);
void fail(char *reason, int code);

int sys_soc_setup();
void sd_setup();

int setup_ehci(uintptr_t base);
int setup_ohci(uintptr_t base);

int font_print_string(int x, int y, char *string);
int font_print_char(int x, int y, char c);
int bmp_print_char(char c);
void bmp_clear();

void int_disable();
void int_enable();
void gic_init();
void gic_trigger(int n);
void gic_enable_irq(int n);

#endif
