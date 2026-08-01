#ifndef RK_INPUT_H
#define RK_INPUT_H

#include <stdint.h>

typedef void input_poll_fn(void);

void input_reset(void);
void input_set_poller(input_poll_fn *poller);
void input_poll(void);
int input_enqueue(uint8_t c);
int input_available(void);
uint8_t input_get_char(void);

#endif
