#include "input.h"

#define INPUT_QUEUE_SIZE 64

static uint8_t queue[INPUT_QUEUE_SIZE];
static unsigned int read_pos;
static unsigned int write_pos;
static input_poll_fn *poll_fn;

void input_reset(void) {
	read_pos = 0;
	write_pos = 0;
}

void input_set_poller(input_poll_fn *poller) {
	poll_fn = poller;
}

void input_poll(void) {
	if (poll_fn)
		poll_fn();
}

int input_enqueue(uint8_t c) {
	unsigned int next = (write_pos + 1) % INPUT_QUEUE_SIZE;
	if (!c || next == read_pos)
		return 0;
	queue[write_pos] = c;
	write_pos = next;
	return 1;
}

int input_available(void) {
	return read_pos != write_pos;
}

uint8_t input_get_char(void) {
	uint8_t c;
	if (read_pos == write_pos)
		return 0;
	c = queue[read_pos];
	read_pos = (read_pos + 1) % INPUT_QUEUE_SIZE;
	return c;
}
