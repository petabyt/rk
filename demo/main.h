#pragma once
#include <stdint.h>

int puts(const char *s);

uint64_t fw_handler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3);
int bmp_setup(void);
int bmp_print(char *s);
int bmp_print_char(char c);
void bmp_clear(void);
