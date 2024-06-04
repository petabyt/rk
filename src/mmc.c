int sdmmc_reset_fifo(volatile struct RkMMC *mmc) {
	mmc->ctrl |= (1 << 1); // reset FIFO

	return 0;
}

int sd_setup() {
	volatile struct RkMMC *mmc = (volatile struct RkMMC *)SDMMC_START;

	// SD host controller supports UHS-1

	gpio_set_dir(0, RK_PIN_A1, 1); // SDMMC0_PWR_H
	gpio_set_pin(0, RK_PIN_A1, 1);

	// Quick clock setup (copied from bootrom pretty much)
	struct Cru *cru = (struct Cru *)CRU_BASE;
	cru->clksel_con[13] = 0x9F008300;
	cru->clksel_con[16] = 0x077F051F;
	cru->sdmmc_con[0] = 0x60004;

	// gpio4b5_sel = sdmmc_cmd
	// gpio4b4_sel = sdmmc_clkout
	// gpio4b3_sel = sdmmc_data3
	// gpio4b2_sel = sdmmc_data2
	// gpio4b1_sel = sdmmc_data1
	// gpio4b0_sel = sdmmc_data0
	grf_gpio_iomux_set(IOMUX_4B, 11, 0, 0b10101010101);

	pmugrf_gpio_iomux_set(PMUGRF_GPIO0A_IOMUX, 15, 14, 0b01); // set iomux to sdmmc_dectn
	gpio_set_dir(0, RK_PIN_A7, 0); // make output

	if (gpio_get_pin(0, RK_PIN_A7)) {
		puts("Card is not inserted (detect returned 1)");
		return 1;
	} else {
		puts("Card is present");
	}

	debug("mmc->ctrl: ", mmc->ctrl);

	mmc->ctrl |= 1; // reset mmc
	mmc->pwren = 1; // power up

	msleep(1000);

	mmc->clkena = 1; // clock enabled

	debug("verid: ", mmc->verid);

	debug("sdmmc status: ", mmc->status);
	uint32_t sta = mmc->status;
	debug("command_fsm_states: ", (sta >> 4) & 0b1111);

	return 0;
}
