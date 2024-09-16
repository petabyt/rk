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
