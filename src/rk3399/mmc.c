#include <rk.h>
#include <os.h>
#include "rk3399.h"

struct SDMMC {
    uint32_t ctrl;
    uint32_t pwren;
    uint32_t clkdiv;
    uint32_t clksrc;
    uint32_t clkena;
    uint32_t tmout;
    uint32_t ctype;
    uint32_t blksiz;
    uint32_t bytcnt;
    uint32_t intmask;
    uint32_t cmdarg;
    uint32_t cmd;
    uint32_t resp0;
    uint32_t resp1;
    uint32_t resp2;
    uint32_t resp3;
    uint32_t mintsts;
    uint32_t rintsts;
    uint32_t status;
    uint32_t fifoth;
    uint32_t cdetect;
    uint32_t wrtprt;
    uint32_t gpio;
    uint32_t tcmcnt;
    uint32_t tbbcnt;
    uint32_t debnce;
    uint32_t usrid;
    uint32_t verid;
    uint32_t hcon;
    uint32_t uhs_reg;
    uint32_t rst_1;
    uint32_t rst_2;
    uint32_t bmod;
    uint32_t pldmnd;
    uint32_t dbaddr;
    uint32_t idsts;
    uint32_t idinten;
    uint32_t dscaddr;
    uint32_t bufaddr;
    uint8_t res0[0x64];
    uint32_t cardthrctl;
    uint32_t back_end_power;
	uint8_t res1[0xf8];
    uint32_t data;
};

#define SDMMC_CMD_START				1 << 31
#define SDMMC_CMD_USE_HOLD_REG		1 << 29
#define SDMMC_CMD_VOLT_SWITCH		1 << 28
#define SDMMC_CMD_CCS_EXP			1 << 23
#define SDMMC_CMD_CEATA_RD			1 << 22
#define SDMMC_CMD_UPD_CLK			1 << 21
#define SDMMC_CMD_INIT				1 << 15
#define SDMMC_CMD_STOP				1 << 14
#define SDMMC_CMD_PRV_DAT_WAIT		1 << 13
#define SDMMC_CMD_SEND_STOP			1 << 12
#define SDMMC_CMD_STRM_MODE			1 << 11
#define SDMMC_CMD_DAT_WR			1 << 10
#define SDMMC_CMD_DAT_EXP			1 << 9
#define SDMMC_CMD_RESP_CRC			1 << 8
#define SDMMC_CMD_RESP_LONG			1 << 7
#define SDMMC_CMD_RESP_EXP			1 << 6
#define CMD_IDX_MASK 0b11111

#define SDCMD_GO_IDLE_STATE          0
#define SDCMD_ALL_SEND_CID           2
#define SDCMD_SEND_RELATIVE_ADDR     3
#define SDCMD_SET_DSR                4
#define SDCMD_SELECT_CARD            7
#define SDCMD_SEND_IF_COND           8
#define SDCMD_SEND_CSD               9
#define SDCMD_SEND_CID              10
#define SDCMD_STOP_TRANSMISSION     12
#define SDCMD_SEND_STATUS           13
#define SDCMD_GO_INACTIVE_STATE     15
#define SDCMD_SET_BLOCKLEN          16
#define SDCMD_READ_SINGLE_BLOCK     17
#define SDCMD_READ_MULTIPLE_BLOCK   18
#define SDCMD_WRITE_BLOCK           24
#define SDCMD_WRITE_MULTIPLE_BLOCK  25
#define SDCMD_PROGRAM_CSD           27
#define SDCMD_SET_WRITE_PROT        28
#define SDCMD_CLR_WRITE_PROT        29
#define SDCMD_SEND_WRITE_PROT       30
#define SDCMD_ERASE_WR_BLOCK_START  32
#define SDCMD_ERASE_WR_BLK_END      33
#define SDCMD_ERASE                 38
#define SDCMD_LOCK_UNLOCK           42
#define SDCMD_APP_CMD               55
#define SDCMD_GEN_CMD               56

#define SDMMC_INT_EBE 15 // End-bit error (read)/Write no CRC (EBE)
#define SDMMC_INT_ACD 14 // Auto command done (ACD)
#define SDMMC_INT_SBE 13 // Start-bit error (SBE)
#define SDMMC_INT_HLE 12 // Hardware locked write error (HLE)
#define SDMMC_INT_FRUN 11 // FIFO underrun/overrun error (FRUN)
#define SDMMC_INT_HTO 10 // Data starvation-by-host timeout (HTO) /Volt_switch_int
#define SDMMC_INT_DRTO 9 // Data read timeout (DRTO)
#define SDMMC_INT_RTO 8 // Response timeout (RTO)
#define SDMMC_INT_DCRC 7 // Data CRC error (DCRC)
#define SDMMC_INT_RCRC 6 // Response CRC error (RCRC)
#define SDMMC_INT_RXDR 5 // Receive FIFO data request (RXDR)
#define SDMMC_INT_TXDR 4 // Transmit FIFO data request (TXDR)
#define SDMMC_INT_DTO 3 // Data transfer over (DTO)
#define SDMMC_INT_CMD_DONE 2 // Command done (CD)
#define SDMMC_INT_RE 1 // Response error (RE)
#define SDMMC_INT_CD 0 // Card detect (CD)

#define SDMMC_STATUS_DATA_BUSY 9
#define SDMMC_STATUS_CARD_PRESENT 8
#define SDMMC_STATUS_FSM_STATE_MASK 0x78

void sdmmc_dump_status(volatile struct SDMMC *mmc) {
	if (mmc->status >> SDMMC_STATUS_DATA_BUSY & 1) {
		puts("Data busy");
	}
	if (mmc->status >> SDMMC_STATUS_CARD_PRESENT & 1) {
		puts("Card present");
	}
	//int fsm = mmc->status >> 4 & 0b111;
	//debug("fsm: ", fsm);
}

int sdmmc_reset_fifo(volatile struct SDMMC *mmc) {
	mmc->ctrl |= (1 << 1); // reset FIFO
	while (mmc->ctrl & (1 << 1));
	return 0;
}

void sdmmc_verbose_intsts(volatile struct SDMMC *mmc) {
	if ((mmc->rintsts >> SDMMC_INT_EBE) & 1) {
	puts("End-bit error (read)/Write no CRC (EBE) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_ACD) & 1) {
	puts("Auto command done (ACD) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_SBE) & 1) {
	puts("Start-bit error (SBE) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_HLE) & 1) {
	puts("Hardware locked write error (HLE) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_FRUN) & 1) {
	puts("FIFO underrun/overrun error (FRUN) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_HTO) & 1) {
	puts("Data starvation-by-host timeout (HTO) /Volt_switch_int is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_DRTO) & 1) {
	puts("Data read timeout (DRTO) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_RTO) & 1) {
	puts("Response timeout (RTO) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_DCRC) & 1) {
	puts("Data CRC error (DCRC) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_RCRC) & 1) {
	puts("Response CRC error (RCRC) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_RXDR) & 1) {
	puts("Receive FIFO data request (RXDR) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_TXDR) & 1) {
	puts("Transmit FIFO data request (TXDR) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_DTO) & 1) {
	puts("Data transfer over (DTO) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_CMD_DONE) & 1) {
	puts("Command done (CD) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_CD) & 1) {
	puts("Card detect (CD) is set");
	}

	if ((mmc->rintsts >> SDMMC_INT_RE) & 1) {
	puts("Response error (RE) is set");
	}
}

int sdmmc_set_cmd(volatile struct SDMMC *mmc, uint32_t val) {
	mmc->cmd = val;
	int tmout = 1000;
	while (mmc->cmd >> 31 & 1) {
		tmout--;
		usleep(1);
		if (!tmout) return 1;
	}

	return 0;
}

int sdmmc_poll_int(volatile struct SDMMC *mmc, int type, int tmout) {
	while (tmout) {
		usleep(1);
		tmout--;
		int sts = mmc->rintsts & 0xffff;
		if (sts == 0) continue;
		sdmmc_verbose_intsts(mmc);
		if (mmc->rintsts & (1 << type)) {
			mmc->rintsts = 1 << type;
			return 0;
		}
	}
	debug("sdmmc: Poll timed out, type: ", type);
	return 1;
}

int sdmmc_cmd(volatile struct SDMMC *mmc, uint32_t cmd, uint32_t cmdarg) {
	sdmmc_reset_fifo(mmc);

	mmc->cmdarg = cmdarg;
	
	mmc->cmd = cmd | SDMMC_CMD_START;

	debug("mmc->cmd: ", mmc->cmd);

	sdmmc_poll_int(mmc, SDMMC_INT_CD, 1000);

	return 0;
}

void sdmmc_setup_pins_clock() {
	gpio_set_dir(0, RK_PIN_A1, 1); // SDMMC0_PWR_H
	gpio_set_pin(0, RK_PIN_A1, 1);

	// Quick clock setup (copied from bootrom pretty much)
	struct Cru *cru = (struct Cru *)CRU_BASE;
	cru->clksel_con[13] = 0x9F008300;
	cru->clksel_con[16] = 0x077F051F;
//	cru->clksel_con[16] = 0x077F0500;
	cru->sdmmc_con[0] = 0x60004;

	// gpio4b5_sel = sdmmc_cmd
	// gpio4b4_sel = sdmmc_clkout
	// gpio4b3_sel = sdmmc_data3
	// gpio4b2_sel = sdmmc_data2
	// gpio4b1_sel = sdmmc_data1
	// gpio4b0_sel = sdmmc_data0
	grf_gpio_iomux_set(IOMUX_4B, 11, 0, 0b10101010101);	
}

int sdmmc_init(volatile struct SDMMC *mmc) {
	debug("verid: ", mmc->verid);

	// SD host controller supports UHS-1

//	mmc->rintsts = 0xffff; // ack all int
//	sdmmc_reset_fifo(mmc);
//	sdmmc_set_cmd(mmc, 0xa0000000 | 0x8100);
//	sdmmc_poll_int(mmc, SDMMC_INT_CMD_DONE, 1000000);
//	return 0;

	debug("mmc->ctrl: ", mmc->ctrl);

	mmc->ctrl = 0b111; // reset mmc, fifo
	while (mmc->ctrl & 0b111);
	mmc->pwren = 1;

	mmc->clkdiv = 0;
	mmc->clkena = 1; // clock enabled

	if ((mmc->cdetect & 1) == 1) {
		puts("Card not inserted");
		return 1;
	}

	mmc->rintsts = 0xfffff; // reset all
//	mmc->intmask = 0xffff; // enable all
	mmc->tmout = 0xffffff40;
	mmc->fifoth = 0x307F0080;
//	sdmmc_dump_status(mmc);

	return 0;	
}

int sd_setup() {
	volatile struct SDMMC *mmc = (volatile struct SDMMC *)SDMMC_START;

	sdmmc_setup_pins_clock();

//	debug("sdmmcdata: ", mmc->data);

	mmc->rintsts = 0xfffff;

	mmc->rintsts = 0xffff;
	sdmmc_reset_fifo(mmc);
	mmc->cmdarg = 0;
	mmc->blksiz = 0x200;
	mmc->bytcnt = 0x200;
	sdmmc_set_cmd(mmc, 0xa0000000 | 0x2351);
	sdmmc_poll_int(mmc, SDMMC_INT_CMD_DONE, 1000000);

	debug("sdmmc insts: ", mmc->rintsts);
	debug("sdmmc resp0: ", mmc->resp0);

	while (mmc->status & (1 << 2));
	while (mmc->status & (1 << 10)); // data_state_mc_busy

	debug("fifo_count: ", mmc->status >> 17 & 0x1fff);

	for (int i = 0; i < 10; i++) {
		while (mmc->status & (1 << 2));
		debug("sdmmcdata: ", mmc->data);
	}

	return 0;
}
