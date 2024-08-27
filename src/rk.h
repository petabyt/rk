volatile void *get_uart_base();
void enable_uart();
struct Uart {
	uint32_t rbr; // recieve buffer reg
	uint32_t ier; // interrupt enable register
	uint32_t fcr; // FIFO control register
	uint32_t lcr; // line control register
	uint32_t mcr; // modem control register
	uint32_t lsr; // line status register
	uint32_t msr; // modem status register
	uint32_t scr; // scratchpad register
	uint32_t reserved1[(0x30 - 0x20) / 4];
	uint32_t srbr[(0x70 - 0x30) / 4];
	uint32_t far;
	uint32_t tfr;
	uint32_t rfw;
	uint32_t usr; // uart status register
	uint32_t tfl;
	uint32_t rfl;
	uint32_t srr; // software reset register
	uint32_t srts;
	uint32_t sbcr;
	uint32_t sdmam;
	uint32_t sfe;
	uint32_t srt;
	uint32_t stet;
	uint32_t htx;
	uint32_t dmasa;
	uint32_t reserved2[(0xf4 - 0xac) / 4];
	uint32_t cpr;
	uint32_t ucv;
	uint32_t ctr;
};

