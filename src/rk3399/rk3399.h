#ifndef BOOT_IO_H
#define BOOT_IO_H

#include <stdint.h>

#define CCI500 0xFFB00000
#define SERVICE_NOC3 0xFFA90000
#define SERVICE_NOC2 0xFFA8C000
#define DDRC1 0xFFA88000
#define SERVICE_NOC1 0xFFA84000
#define DDRC0 0xFFA80000
#define SERVICE_NOC0 0xFFA50000

#define USB2_HOST0_START 0xFE380000
#define USB2_HOST1_START 0xFE3C0000
#define USB_OTG0_START 0xFE800000
#define USB_OTG1_START 0xFE900000

// MMC interfaces
#define SDMMC_START 0xFE320000
#define EMMC_START 0xFE330000

// Interrupt controller
#define GICD_BASE 0xfee00000
#define GICR_BASE 0xfef00000
#define GICC_BASE 0xfff00000
#define GICH_BASE 0xfff10000
#define GICV_BASE 0xfff20000
#define GITS_BASE 0xfee20000

// Covers timers 0-5
#define TIMER0_START 0xFF850000
// Covers timers 6-11
#define TIMER6_START 0xFF858000

#define GPIO0_START 0xFF720000
#define GPIO1_START 0xFF730000
#define GPIO2_START 0xFF780000
#define GPIO3_START 0xFF788000
#define GPIO4_START 0xFF790000

#define UART0_START 0xFF180000
#define UART1_START 0xFF190000
#define UART2_START 0xFF1A0000
#define UART3_START 0xFF1B0000

#define I2C0_BASE 0xff3c0000
#define I2C1_BASE 0xff110000
#define I2C2_BASE 0xff120000
#define I2C3_BASE 0xff130000
#define I2C4_BASE 0xff3d0000
#define I2C5_BASE 0xff140000
#define I2C6_BASE 0xff150000
#define I2C7_BASE 0xff160000
#define I2C8_BASE 0xff3e0000

#define PMU_BASE 0xFF310000
#define PMUGRF_BASE 0xFF320000
#define PMUSGRF_BASE 0xFF330000
#define PWM_BASE 0xFF420000

#define PMUCRU_BASE 0xFF750000
#define CRU_BASE 0xFF760000
#define GRF_BASE 0xFF770000

#define EDP_BASE 0xFF970000

#define GRF_IOMUX_BASE	(GRF_BASE + 0xe000)

#define VOP_BIG_BASE 0xFF900000
#define VOP_LIT_BASE 0xFF8F0000

#define HDCP_MMU_START 0xFF930000

#define CIC_BASE 0xFF620000

#define GRF_SOC_CON20	0x6250
#define GRF_SOC_CON25	0x6264

#ifndef __ASM__

void grf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);
void pmugrf_gpio_iomux_set(int gpio, int bit1, int bit2, int func);

void setup_cru(void);
void clock_set_pll(volatile uint32_t *cons, uint32_t refdiv, uint32_t fbdiv, uint32_t postdiv1, uint32_t postdiv2);

void clock_setup_vop(void); // clock.c

int rk3399_init_vop(uintptr_t vop_addr, uint32_t fb_address);

int rk3399_init_edp(uintptr_t edp_addr);
int rk3399_enable_edp(uintptr_t edp_addr);

void reset_timer0(void);
void timer0_msleep(int ms);
void timer0_usleep(int ticks);

int rk3399_cpu_clock_start(void);

void rk_setup_stimer(void);

int sys_soc_setup(void);

// Structs copied from uboot can NOT be packed! They rely on GCC padding!

// Pulse Width Modulation
// page 637
struct Pwm {
	struct PwmChannel {
		uint32_t counter_reg;
		uint32_t period_reg;
		uint32_t duty_reg;
	}channels[3];
	uint32_t int_status_reg;
	uint32_t int_enable_reg;
	uint32_t channel3_fifi_mode_control_reg;
	uint32_t fifo_interrupts_status_reg;
	uint32_t fifo_timeout_threshold_reg;
};

// Page 47
// Power Management Unit Clock & Reset Unit
struct PmuCru {
	uint32_t ppll_conf_regs[6];
	uint32_t clock_select_divide_regs[8];
	uint32_t clock_gating_regs[3];
	
    uint32_t pmucru_softrst_con0;
    uint32_t pmucru_softrst_con1;
    uint32_t pmucru_rstnhold_con0;
    uint32_t pmucru_rstnhold_con1;
    uint32_t pmucru_gatedis_con0;
};

// Page 493
struct Pmu {
	uint32_t pmu_wakeup_cfg[5];
	uint32_t pmu_pwrdn_con;
	uint32_t pmu_pwrdn_st;
	uint32_t pmu_pll_con;
	uint32_t pmu_pwrmode_con;
	uint32_t pmu_sft_con;
	uint32_t pmu_int_con;
	uint32_t pmu_int_st;
	uint32_t pmu_gpio0_pos_int_con;
	uint32_t pmu_gpio0_net_int_con;
	uint32_t pmu_gpio1_pos_int_con;
	uint32_t pmu_gpio1_net_int_con;
	uint32_t pmu_gpio0_pos_int_st;
	uint32_t pmu_gpio0_net_int_st;
	uint32_t pmu_gpio1_pos_int_st;
	uint32_t pmu_gpio1_net_int_st;
	uint32_t pmu_pwrdn_inten;
	uint32_t pmu_pwrdn_status;
	uint32_t pmu_wakeup_status;
	uint32_t pmu_bus_clr;
	uint32_t pmu_bus_idle_req;
	uint32_t pmu_bus_idle_st;
	uint32_t pmu_bus_idle_ack;
	uint32_t pmu_cci500_con;
	uint32_t pmu_adb400_con;
	uint32_t pmu_adb400_st;
	uint32_t pmu_power_st;
	uint32_t pmu_core_pwr_st;
	uint32_t pmu_osc_cnt;
	uint32_t pmu_plllock_cnt;
	uint32_t pmu_pllrst_cnt;
	uint32_t pmu_stable_cnt;
	uint32_t pmu_ddrio_pwron_cnt;
	uint32_t pmu_wakeup_rst_clr_cnt;
	uint32_t pmu_ddr_sref_st;
	uint32_t pmu_scu_l_pwrdn_cnt;
	uint32_t pmu_scu_l_pwrup_cnt;
	uint32_t pmu_scu_b_pwrdn_cnt;
	uint32_t pmu_scu_b_pwrup_cnt;
	uint32_t pmu_gpu_pwrdn_cnt;
	uint32_t pmu_gpu_pwrup_cnt;
	uint32_t pmu_center_pwrdn_cnt;
	uint32_t pmu_center_pwrup_cnt;
	uint32_t pmu_timeout_cnt;
	uint32_t pmu_cpu0apm_con;
	uint32_t pmu_cpu1apm_con;
	uint32_t pmu_cpu2apm_con;
	uint32_t pmu_cpu3apm_con;
	uint32_t pmu_cpu0bpm_con;
	uint32_t pmu_cpu1bpm_con;
	uint32_t pmu_noc_auto_ena;
	uint32_t pmu_pwrdn_con1;
	uint32_t reserved0[0x4];
	uint32_t pmu_sys_reg_reg0;
	uint32_t pmu_sys_reg_reg1;
	uint32_t pmu_sys_reg_reg2;
	uint32_t pmu_sys_reg_reg3;
};

// Clock and reset unit
struct Cru {
	uint32_t apll_l_con[6];
	uint32_t reserved[2];
	uint32_t apll_b_con[6];
	uint32_t reserved1[2];
	uint32_t dpll_con[6];
	uint32_t reserved2[2];
	uint32_t cpll_con[6];
	uint32_t reserved3[2];
	uint32_t gpll_con[6];
	uint32_t reserved4[2];
	uint32_t npll_con[6];
	uint32_t reserved5[2];
	uint32_t vpll_con[6];
	uint32_t reserved6[0x0a];
	uint32_t clksel_con[108];
	uint32_t reserved7[0x14];
	uint32_t clkgate_con[35];
	uint32_t reserved8[0x1d];
	uint32_t softrst_con[21];
	uint32_t reserved9[0x2b];
	uint32_t glb_srst_fst_value;
	uint32_t glb_srst_snd_value;
	uint32_t glb_cnt_th;
	uint32_t misc_con;
	uint32_t glb_rst_con;
	uint32_t glb_rst_st;
	uint32_t reserved10[0x1a];
	uint32_t sdmmc_con[2];
	uint32_t sdio0_con[2];
	uint32_t sdio1_con[2];
};

// General register file
struct Grf {
	// ...
};

#define IOMUX_2A	0
#define IOMUX_2B	1
#define IOMUX_2C	2
#define IOMUX_2D	3
#define IOMUX_3A	4
#define IOMUX_3B	5
#define IOMUX_3C	6
#define IOMUX_3D	7
#define IOMUX_4A	8
#define IOMUX_4B	9
#define IOMUX_4C	10
#define IOMUX_4D	11

// PMUGRF GPIO iomux control
#define PMUGRF_GPIO0A_IOMUX 1
#define PMUGRF_GPIO0B_IOMUX 2
// mysterious omission of PMUGRF_GPIO0C_IOMUX and PMUGRF_GPIO0D_IOMUX (?)
#define PMUGRF_GPIO1A_IOMUX 4
#define PMUGRF_GPIO1B_IOMUX 5
#define PMUGRF_GPIO1C_IOMUX 6
#define PMUGRF_GPIO1D_IOMUX 7

// PMUSGRF / SGRF
// Completely undocumented in TRM
struct PmuSGrfRegs {
	uint32_t ddr_rgn_con[35];
	uint32_t reserved[0x1fe5];
	uint32_t soc_con8;
	uint32_t soc_con9;
	uint32_t soc_con10;
	uint32_t soc_con11;
	uint32_t soc_con12;
	uint32_t soc_con13;
	uint32_t soc_con14;
	uint32_t soc_con15;
	uint32_t reserved1[3];
	uint32_t soc_con19;
	uint32_t soc_con20;
	uint32_t soc_con21;
	uint32_t soc_con22;
	uint32_t reserved2[0x29];
	uint32_t perilp_con[9];
	uint32_t reserved4[7];
	uint32_t perilp_status;
	uint32_t reserved5[0xfaf];
	uint32_t soc_con0;
	uint32_t soc_con1;
	uint32_t reserved6[0x3e];
	uint32_t pmu_con[9];
	uint32_t reserved7[0x17];
	uint32_t fast_boot_addr;
	uint32_t reserved8[0x1f];
	uint32_t efuse_prg_mask;
	uint32_t efuse_read_mask;
	uint32_t reserved9[0x0e];
	uint32_t pmu_slv_con0;
	uint32_t pmu_slv_con1;
	uint32_t reserved10[0x771];
	uint32_t soc_con3;
	uint32_t soc_con4;
	uint32_t soc_con5;
	uint32_t soc_con6;
	uint32_t soc_con7;
	uint32_t reserved11[8];
	uint32_t soc_con16;
	uint32_t soc_con17;
	uint32_t soc_con18;
	uint32_t reserved12[0xdd];
	uint32_t slv_secure_con0;
	uint32_t slv_secure_con1;
	uint32_t reserved13;
	uint32_t slv_secure_con2;
	uint32_t slv_secure_con3;
	uint32_t slv_secure_con4;
};

struct RkPLL {
	uint32_t con0;
	uint32_t con1;
	uint32_t con2;
	uint32_t con3;
	uint32_t con5;
	uint32_t con6;
};

struct MSCHRegs {
	uint32_t coreid;
	uint32_t revisionid;
	uint32_t ddrconf;
	uint32_t ddrsize;
};
#endif

#endif
