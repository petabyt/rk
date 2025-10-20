#ifndef RK3588_IO_H
#define RK3588_IO_H

#include <stdint.h>

#define RK3588_BASE_CLOCK 25

#define SYS_GRF 0x0fd58c000

#define DEVICE_MEMORY_START 0x0f0000000
#define DEVICE_MEMORY_END 0x0ff100000

#define PCIE_DEVICE_MEMORY_START 0x900000000

#define SDMMC 0x0fe2c0000
#define SDMMC_BUF 0x0fe2f0000

// OHCI regs are at offset 0x40000
#define USB2HOST_0 0x0fc800000
#define USB2HOST_1 0x0fc880000

#define USB2PHY0_GRF 0x0fd5d0000
#define USB2PHY1_GRF 0x0fd5d4000
#define USB2PHY2_GRF 0x0fd5d8000
#define USB2PHY3_GRF 0x0fd5dc000

#define GPIO0 0x0fd8a0000
#define GPIO1 0x0fec20000
#define GPIO2 0x0fec30000
#define GPIO3 0x0fec40000
#define GPIO4 0x0fec50000

#define HPTIMER 0x0fd8c8000
#define TIMER_S_0 0x0fe3d0000
#define TIMER_NS_0 0x0feae0000
#define TIMER_NS_1 0x0feae8000
#define TIMER_PMU 0x0fd8f0000

#define USB3_0 0x0fc000000
#define USB3_1 0x0fc400000
#define USB3_2 0x0fcd00000

#define EDP0 0x0fdec0000
#define EDP1 0x0fded0000

#define UART0 0x0fd890000
#define UART1 0x0feb40000
#define UART2 0x0feb50000
#define UART3 0x0feb60000

#define GIC600_D 0x0fe600000
#define GIC600_R 0x0fe680000

#define PMU 0x0fd8d0000

#define PMU0_SGRF 0x0fd580000
#define PMU1_SGRF 0x0fd582000

#define PMU1_IOC 0x0fd5f0000
#define PMU2_IOC 0x0fd5f4000
#define BUS_IOC 0x0fd5f8000
#define VCCIO3_5_IOC 0x0fd5fa000

#define HDMI_TX0 0x0fde80000
#define HDMI_TX1 0x0fdea0000 // Not available on RK3588S (?)

#define HDPTXPHY0_GRF 0x0fd5e0000
#define HDPTX_COMBO_PHY0 0x0fed60000

#define HDPTXPHY1_GRF 0x0fd5e4000
#define HDPTX_COMBO_PHY1 0x0fed70000

// HDMI RX is not on RK3588S

#define VOP_GRF 0x0fd5a4000
#define VO0_GRF 0x0fd5a6000
#define VO1_GRF 0x0fd5a8000
#define VOP 0x0fdd90000

#define CRU 0x0fd7c0000
#define SECURECRU 0x0fd7d0000
#define SBUSCRU 0x0fd7d8000
#define PMU1CRU 0x0fd7f0000
#define PHP_PPLL_CRU 0x0fd7c8000

#define PDM0 0x0fe4b0000
#define PDM1 0x0fe4c0000

#define FSPI 0x0fe2b0000
#define SPI0 0x0feb00000
#define SPI1 0x0feb10000
#define SPI2 0x0feb20000
#define SPI3 0x0feb30000

#ifndef __ASM__
struct __attribute__((packed)) Pmu1Ioc {
	uint32_t GPIO0A_IOMUX_SEL_L;   // Offset 0x0000
	uint32_t GPIO0A_IOMUX_SEL_H;   // Offset 0x0004
	uint32_t GPIO0B_IOMUX_SEL_L;   // Offset 0x0008
	uint32_t GPIO0A_DS_L;		  // Offset 0x0010
	uint32_t GPIO0A_DS_H;		  // Offset 0x0014
	uint32_t GPIO0B_DS_L;		  // Offset 0x0018
	uint32_t GPIO0A_P;			 // Offset 0x0020
	uint32_t GPIO0B_P;			 // Offset 0x0024
	uint32_t GPIO0A_IE;			// Offset 0x0028
	uint32_t GPIO0B_IE;			// Offset 0x002C
	uint32_t GPIO0A_SMT;		   // Offset 0x0030
	uint32_t GPIO0B_SMT;		   // Offset 0x0034
	uint32_t GPIO0A_PDIS;		  // Offset 0x0038
	uint32_t GPIO0B_PDIS;		  // Offset 0x003C
	uint32_t XIN_CON;			  // Offset 0x0040
};

struct __attribute__((packed)) Pmu2Ioc {
	uint32_t GPIO0B_IOMUX_SEL_H;   // Offset 0x0000
	uint32_t GPIO0C_IOMUX_SEL_L;   // Offset 0x0004
	uint32_t GPIO0C_IOMUX_SEL_H;   // Offset 0x0008
	uint32_t GPIO0D_IOMUX_SEL_L;   // Offset 0x000C
	uint32_t GPIO0D_IOMUX_SEL_H;   // Offset 0x0010
	uint32_t GPIO0B_DS_H;		  // Offset 0x0014
	uint32_t GPIO0C_DS_L;		  // Offset 0x0018
	uint32_t GPIO0C_DS_H;		  // Offset 0x001C
	uint32_t GPIO0D_DS_L;		  // Offset 0x0020
	uint32_t GPIO0D_DS_H;		  // Offset 0x0024
	uint32_t GPIO0B_P;			 // Offset 0x0028
	uint32_t GPIO0C_P;			 // Offset 0x002C
	uint32_t GPIO0D_P;			 // Offset 0x0030
	uint32_t GPIO0B_IE;			// Offset 0x0034
	uint32_t GPIO0C_IE;			// Offset 0x0038
	uint32_t GPIO0D_IE;			// Offset 0x003C
	uint32_t GPIO0B_SMT;		   // Offset 0x0040
	uint32_t GPIO0C_SMT;		   // Offset 0x0044
	uint32_t GPIO0D_SMT;		   // Offset 0x0048
	uint32_t GPIO0B_PDIS;		  // Offset 0x004C
	uint32_t GPIO0C_PDIS;		  // Offset 0x0050
	uint32_t GPIO0D_PDIS;		  // Offset 0x0054
};

struct __attribute__((packed)) BusIoc {
	uint32_t reserved_0000[3];			  // Reserved space (0x0000 - 0x000B)
	uint32_t GPIO0B_IOMUX_SEL_H;	// Offset 0x000C
	uint32_t GPIO0C_IOMUX_SEL_L;	// Offset 0x0010
	uint32_t GPIO0C_IOMUX_SEL_H;	// Offset 0x0014
	uint32_t GPIO0D_IOMUX_SEL_L;	// Offset 0x0018
	uint32_t GPIO0D_IOMUX_SEL_H;	// Offset 0x001C
	uint32_t GPIO1A_IOMUX_SEL_L;	// Offset 0x0020
	uint32_t GPIO1A_IOMUX_SEL_H;	// Offset 0x0024
	uint32_t GPIO1B_IOMUX_SEL_L;	// Offset 0x0028
	uint32_t GPIO1B_IOMUX_SEL_H;	// Offset 0x002C
	uint32_t GPIO1C_IOMUX_SEL_L;	// Offset 0x0030
	uint32_t gpio10c_iomux_sel_h; // Offset 0x0034
	uint32_t gpio10c_iomux_sel_l; // Offset 0x0038
	uint32_t gpio10d_iomux_sel_h; // Offset 0x003C
	uint32_t gpio10d_iomux_sel_l; // Offset 0x0040
	uint32_t gpio20a_iomux_sel_h; // Offset 0x0044
	uint32_t gpio20a_iomux_sel_l; // Offset 0x0048
	uint32_t gpio20b_iomux_sel_h; // Offset 0x004C
	uint32_t gpio20b_iomux_sel_l; // Offset 0x0050
	uint32_t gpio20c_iomux_sel_h; // Offset 0x0054
	uint32_t gpio20c_iomux_sel_l; // Offset 0x0058
	uint32_t gpio20d_iomux_sel_h; // Offset 0x005C
	uint32_t gpio20d_iomux_sel_l; // Offset 0x0060
	uint32_t gpio30a_iomux_sel_h; // Offset 0x0064
	uint32_t gpio30a_iomux_sel_l; // Offset 0x0068
	uint32_t gpio30b_iomux_sel_h; // Offset 0x006C
	uint32_t gpio30b_iomux_sel_l; // Offset 0x0070
	uint32_t gpio30c_iomux_sel_h; // Offset 0x0074
	uint32_t gpio30c_iomux_sel_l; // Offset 0x0078
	uint32_t gpio30d_iomux_sel_h; // Offset 0x007C
	uint32_t gpio30d_iomux_sel_l; // Offset 0x0080
	uint32_t gpio40a_iomux_sel_h; // Offset 0x0084
	uint32_t gpio40a_iomux_sel_l; // Offset 0x0088
	uint32_t gpio40b_iomux_sel_h; // Offset 0x008C
	uint32_t gpio40b_iomux_sel_l; // Offset 0x0090
	uint32_t gpio40c_iomux_sel_h; // Offset 0x0094
	uint32_t gpio40c_iomux_sel_l; // Offset 0x0098
};

struct __attribute__((packed)) Gpio {
	uint32_t GPIO_SWPORT_DR_L;	  // Offset: 0x0000, Port Data Register (Low)
	uint32_t GPIO_SWPORT_DR_H;	  // Offset: 0x0004, Port Data Register (High)
	uint32_t GPIO_SWPORT_DDR_L;	 // Offset: 0x0008, Port Data Direction Register (Low)
	uint32_t GPIO_SWPORT_DDR_H;	 // Offset: 0x000C, Port Data Direction Register (High)
	uint32_t GPIO_INT_EN_L;		 // Offset: 0x0010, Interrupt Enable Register (Low)
	uint32_t GPIO_INT_EN_H;		 // Offset: 0x0014, Interrupt Enable Register (High)
	uint32_t GPIO_INT_MASK_L;	   // Offset: 0x0018, Interrupt Mask Register (Low)
	uint32_t GPIO_INT_MASK_H;	   // Offset: 0x001C, Interrupt Mask Register (High)
	uint32_t GPIO_INT_TYPE_L;	   // Offset: 0x0020, Interrupt Level Register (Low)
	uint32_t GPIO_INT_TYPE_H;	   // Offset: 0x0024, Interrupt Level Register (High)
	uint32_t GPIO_INT_POLARITY_L;   // Offset: 0x0028, Interrupt Polarity Register (Low)
	uint32_t GPIO_INT_POLARITY_H;   // Offset: 0x002C, Interrupt Polarity Register (High)
};

struct __attribute__((packed)) Pwm {
	uint32_t pwm0_cnt;
	uint32_t pwm0_period_hpr;
	uint32_t pwm0_duty_lpr;
	uint32_t pwm0_ctrl;
	uint32_t pwm1_cnt;
	uint32_t pwm1_period_hpr;
	uint32_t pwm1_duty_lpr;
	uint32_t pwm1_ctrl;
	uint32_t pwm2_cnt;
	uint32_t pwm2_period_hpr;
	uint32_t pwm2_duty_lpr;
	uint32_t pwm2_ctrl;
	uint32_t pwm3_cnt;
	uint32_t pwm3_period_hpr;
	uint32_t pwm3_duty_lpr;
	uint32_t pwm3_ctrl;
	uint32_t inststs;
	uint32_t int_en;
	uint32_t res3[2];
	uint32_t fifo_ctrl;
	uint32_t fifo_inststs;
	uint32_t fifo_toutthr;
	uint32_t version_id;
	uint32_t fifo;
	uint32_t res0[7];
	uint32_t pwrmatch_ctrl;
	uint32_t pwrmatch_lpre;
	uint32_t pwrmatch_hpre;
	uint32_t pwrmatch_ld;
	uint32_t pwrmatch_hd_zero;
	uint32_t pwrmatch_hd_on;
	uint32_t pwrmatch_value0;
	uint32_t pwrmatch_value1;
	uint32_t pwrmatch_value2;
	uint32_t pwrmatch_value3;
	uint32_t pwrmatch_value4;
	uint32_t pwrmatch_value5;
	uint32_t pwrmatch_value6;
	uint32_t pwrmatch_value7;
	uint32_t pwrmatch_value8;
	uint32_t pwrmatch_value9;
	uint8_t res1[0xc];
	uint32_t pwm3_pwrcaptur_e_value;
	uint32_t channel_io_ctrl;
};

struct __attribute__((packed)) Pdm {
	uint32_t PDM_SYSCONFIG;	 // 0x0000: PDM System Configure Register
	uint32_t PDM_CTRL0;		 // 0x0004: PDM Control Register 0
	uint32_t PDM_CTRL1;		 // 0x0008: PDM Control Register 1
	uint32_t PDM_CLK_CTRL;	  // 0x000C: PDM Clock Control Register
	uint32_t PDM_HPF_CTRL;	  // 0x0010: PDM High-pass Filter Control Register
	uint32_t PDM_FIFO_CTRL;	 // 0x0014: PDM FIFO Control Register
	uint32_t PDM_DMA_CTRL;	  // 0x0018: PDM DMA Control Register
	uint32_t PDM_INT_EN;		// 0x001C: PDM Interrupt Enable Register
	uint32_t PDM_INT_CLR;	   // 0x0020: PDM Interrupt Clear Register
	uint32_t PDM_INT_ST;		// 0x0024: PDM Interrupt Status Register
	uint32_t res1[02];
	uint32_t PDM_RXFIFO_DATA_REG; // 0x0030: PDM Receive FIFO Data Register
	uint32_t PDM_DATA0R_REG;	// 0x0034: PDM Path0 Right Channel Data Register
	uint32_t PDM_DATA0L_REG;	// 0x0038: PDM Path0 Left Channel Data Register
	uint32_t PDM_DATA1R_REG;	// 0x003C: PDM Path1 Right Channel Data Register
	uint32_t PDM_DATA1L_REG;	// 0x0040: PDM Path1 Left Channel Data Register
	uint32_t PDM_DATA2R_REG;	// 0x0044: PDM Path2 Right Channel Data Register
	uint32_t PDM_DATA2L_REG;	// 0x0048: PDM Path2 Left Channel Data Register
	uint32_t PDM_DATA3R_REG;	// 0x004C: PDM Path3 Right Channel Data Register
	uint32_t PDM_DATA3L_REG;	// 0x0050: PDM Path3 Left Channel Data Register
	uint32_t PDM_DATA_VALID;	// 0x0054: PDM Path Data Valid Register
	uint32_t PDM_VERSION;	   // 0x0058: PDM Version Register
	uint32_t reserved[233];			  // Reserved space
	uint32_t PDM_INCR_RXDR;	 // 0x0400: Increment Address Receive FIFO Data Register
};

struct __attribute__((packed)) Dwhdmi {
	uint32_t core_id;
	uint32_t ver_number;
	uint32_t ver_type;
	uint32_t config_reg;
	uint8_t res0[4];
	uint32_t core_timestamp_hhmm;
	uint32_t core_timestamp_mmdd;
	uint32_t core_timestamp_yyyy;
	uint8_t res1[32];
	uint32_t global_swreset_request;
	uint32_t global_swdisable;
	uint32_t reset_manager_config0;
	uint8_t res2[4];
	uint32_t reset_manager_status0;
	uint32_t reset_manager_status1;
	uint32_t reset_manager_status2;
	uint8_t res3[36];
	uint32_t timer_base_config0;
	uint32_t timer_base_status0;
	uint8_t res4[88];
	uint32_t i2cm_sm_scl_config0;
	uint32_t i2cm_fm_scl_config0;
	uint32_t i2cm_config0;
	uint32_t i2cm_control0;
	uint32_t i2cm_status0;
	uint32_t i2cm_interface_control0;
	uint32_t i2cm_interface_control1;
	uint32_t i2cm_interface_wrdata_0_3;
	uint32_t i2cm_interface_wrdata_4_7;
	uint32_t i2cm_interface_wrdata_8_11;
	uint32_t i2cm_interface_wrdata_12_15;
	uint32_t i2cm_interface_rddata_0_3;
	uint32_t i2cm_interface_rddata_4_7;
	uint32_t i2cm_interface_rddata_8_11;
	uint32_t i2cm_interface_rddata_12_15;
	uint8_t res5[36];
	uint32_t scdc_config0;
	uint8_t res6[4];
	uint32_t scdc_control0;
	uint8_t res7[4];
	uint32_t scdc_status0;
	uint8_t res8[12];
	uint32_t flt_config0;
	uint32_t flt_config1;
	uint32_t flt_config2;
	uint8_t res9[4];
	uint32_t flt_control0;
	uint8_t res10[12];
	uint32_t mainunit_status0;
	uint8_t res11[1660];
	uint32_t video_interface_config0;
	uint8_t res12[4];
	uint32_t video_interface_config2;
	uint32_t video_interface_control0;
	uint8_t res13[4];
	uint32_t video_interface_status0;
	uint8_t res14[4];
	uint32_t video_packing_config0;
	uint32_t audio_interface_config0;
	uint32_t audio_interface_config1;
	uint8_t res15[4];
	uint32_t audio_interface_control0;
	uint8_t res16[4];
	uint32_t audio_interface_status0;
	uint8_t res17[8];
	uint32_t frame_composer_config0;
	uint32_t frame_composer_config1;
	uint32_t frame_composer_config2;
	uint32_t frame_composer_config3;
	uint32_t frame_composer_config4;
	uint32_t frame_composer_config5;
	uint32_t frame_composer_config6;
	uint32_t frame_composer_config7;
	uint32_t frame_composer_config8;
	uint32_t frame_composer_config9;
	uint8_t res18[4];
	uint32_t frame_composer_control0;
	uint8_t res19[16];
	uint32_t video_monitor_config0;
	uint32_t video_monitor_status0;
	uint32_t video_monitor_status1;
	uint32_t video_monitor_status2;
	uint32_t video_monitor_status3;
	uint32_t video_monitor_status4;
	uint32_t video_monitor_status5;
	uint32_t video_monitor_status6;
	uint8_t res20[64];
	uint32_t hdcp2logic_config0;
	uint32_t hdcp2logic_esm_gpio_in;
	uint32_t hdcp2logic_esm_gpio_out;
	uint8_t res21[20];
	uint32_t hdcp14_config0;
	uint32_t hdcp14_control0;
	uint32_t hdcp14_config1;
	uint32_t hdcp14_config2;
	uint8_t res22[4];
	uint32_t hdcp14_key_seed;
	uint32_t hdcp14_key_h;
	uint32_t hdcp14_key_l;
	uint32_t hdcp14_key_status;
	uint32_t hdcp14_aksv_h;
	uint32_t hdcp14_aksv_l;
	uint32_t hdcp14_an_h;
	uint32_t hdcp14_an_l;
	uint32_t hdcp14_status0;
	uint32_t hdcp14_status1;
	uint8_t res23[36];
	uint32_t scramb_config0;
	uint8_t res24[4];
	uint32_t link_config0;
	uint8_t res25[4];
	uint32_t tmds_fifo_config0;
	uint32_t tmds_fifo_control0;
	uint8_t res26[168];
	uint32_t frl_rsfec_config0;
	uint8_t res27[12];
	uint32_t frl_rsfec_status0;
	uint8_t res28[12];
	uint32_t frl_pktz_config0;
	uint32_t frl_pktz_control0;
	uint8_t res29[8];
	uint32_t frl_pktz_control1;
	uint32_t frl_pktz_status1;
	uint8_t res30[40];
	uint32_t pktsched_config0;
	uint32_t pktsched_prqueue0_config0;
	uint32_t pktsched_prqueue1_config0;
	uint32_t pktsched_prqueue2_config0;
	uint32_t pktsched_prqueue2_config1;
	uint32_t pktsched_prqueue2_config2;
	uint32_t pktsched_pkt_config0;
	uint32_t pktsched_pkt_config1;
	uint32_t pktsched_pkt_config2;
	uint32_t pktsched_pkt_config3;
	uint32_t pktsched_pkt_en;
	uint32_t pktsched_pkt_control0;
	uint32_t pktsched_pkt_send;
	uint32_t pktsched_pkt_status0;
	uint32_t pktsched_pkt_status1;
	uint8_t res31[68];
	uint32_t pkt_null_contents0;
	uint32_t pkt_null_contents1;
	uint32_t pkt_null_contents2;
	uint32_t pkt_null_contents3;
	uint32_t pkt_null_contents4;
	uint32_t pkt_null_contents5;
	uint32_t pkt_null_contents6;
	uint32_t pkt_null_contents7;
	uint32_t pkt_acp_contents0;
	uint32_t pkt_acp_contents1;
	uint32_t pkt_acp_contents2;
	uint32_t pkt_acp_contents3;
	uint32_t pkt_acp_contents4;
	uint32_t pkt_acp_contents5;
	uint32_t pkt_acp_contents6;
	uint32_t pkt_acp_contents7;
	uint32_t pkt_isrc1_contents0;
	uint32_t pkt_isrc1_contents1;
	uint32_t pkt_isrc1_contents2;
	uint32_t pkt_isrc1_contents3;
	uint32_t pkt_isrc1_contents4;
	uint32_t pkt_isrc1_contents5;
	uint32_t pkt_isrc1_contents6;
	uint32_t pkt_isrc1_contents7;
	uint32_t pkt_isrc2_contents0;
	uint32_t pkt_isrc2_contents1;
	uint32_t pkt_isrc2_contents2;
	uint32_t pkt_isrc2_contents3;
	uint32_t pkt_isrc2_contents4;
	uint32_t pkt_isrc2_contents5;
	uint32_t pkt_isrc2_contents6;
	uint32_t pkt_isrc2_contents7;
	uint32_t pkt_gmd_contents0;
	uint32_t pkt_gmd_contents1;
	uint32_t pkt_gmd_contents2;
	uint32_t pkt_gmd_contents3;
	uint32_t pkt_gmd_contents4;
	uint32_t pkt_gmd_contents5;
	uint32_t pkt_gmd_contents6;
	uint32_t pkt_gmd_contents7;
	uint32_t pkt_amd_contents0;
	uint32_t pkt_amd_contents1;
	uint32_t pkt_amd_contents2;
	uint32_t pkt_amd_contents3;
	uint32_t pkt_amd_contents4;
	uint32_t pkt_amd_contents5;
	uint32_t pkt_amd_contents6;
	uint32_t pkt_amd_contents7;
	uint32_t pkt_vsi_contents0;
	uint32_t pkt_vsi_contents1;
	uint32_t pkt_vsi_contents2;
	uint32_t pkt_vsi_contents3;
	uint32_t pkt_vsi_contents4;
	uint32_t pkt_vsi_contents5;
	uint32_t pkt_vsi_contents6;
	uint32_t pkt_vsi_contents7;
	uint32_t pkt_avi_contents0;
	uint32_t pkt_avi_contents1;
	uint32_t pkt_avi_contents2;
	uint32_t pkt_avi_contents3;
	uint32_t pkt_avi_contents4;
	uint32_t pkt_avi_contents5;
	uint32_t pkt_avi_contents6;
	uint32_t pkt_avi_contents7;
	uint32_t pkt_spdi_contents0;
	uint32_t pkt_spdi_contents1;
	uint32_t pkt_spdi_contents2;
	uint32_t pkt_spdi_contents3;
	uint32_t pkt_spdi_contents4;
	uint32_t pkt_spdi_contents5;
	uint32_t pkt_spdi_contents6;
	uint32_t pkt_spdi_contents7;
	uint32_t pkt_audi_contents0;
	uint32_t pkt_audi_contents1;
	uint32_t pkt_audi_contents2;
	uint32_t pkt_audi_contents3;
	uint32_t pkt_audi_contents4;
	uint32_t pkt_audi_contents5;
	uint32_t pkt_audi_contents6;
	uint32_t pkt_audi_contents7;
	uint32_t pkt_nvi_contents0;
	uint32_t pkt_nvi_contents1;
	uint32_t pkt_nvi_contents2;
	uint32_t pkt_nvi_contents3;
	uint32_t pkt_nvi_contents4;
	uint32_t pkt_nvi_contents5;
	uint32_t pkt_nvi_contents6;
	uint32_t pkt_nvi_contents7;
	uint32_t pkt_drmi_contents0;
	uint32_t pkt_drmi_contents1;
	uint32_t pkt_drmi_contents2;
	uint32_t pkt_drmi_contents3;
	uint32_t pkt_drmi_contents4;
	uint32_t pkt_drmi_contents5;
	uint32_t pkt_drmi_contents6;
	uint32_t pkt_drmi_contents7;
	uint32_t pkt_ghdmi1_contents0;
	uint32_t pkt_ghdmi1_contents1;
	uint32_t pkt_ghdmi1_contents2;
	uint32_t pkt_ghdmi1_contents3;
	uint32_t pkt_ghdmi1_contents4;
	uint32_t pkt_ghdmi1_contents5;
	uint32_t pkt_ghdmi1_contents6;
	uint32_t pkt_ghdmi1_contents7;
	uint32_t pkt_ghdmi2_contents0;
	uint32_t pkt_ghdmi2_contents1;
	uint32_t pkt_ghdmi2_contents2;
	uint32_t pkt_ghdmi2_contents3;
	uint32_t pkt_ghdmi2_contents4;
	uint32_t pkt_ghdmi2_contents5;
	uint32_t pkt_ghdmi2_contents6;
	uint32_t pkt_ghdmi2_contents7;
	uint8_t res32[32];
	uint32_t pkt_emp_config0;
	uint8_t res33[28];
	uint32_t pkt_emp_vtem_contents0;
	uint32_t pkt_emp_vtem_contents1;
	uint32_t pkt_emp_vtem_contents2;
	uint32_t pkt_emp_vtem_contents3;
	uint32_t pkt_emp_vtem_contents4;
	uint32_t pkt_emp_vtem_contents5;
	uint32_t pkt_emp_vtem_contents6;
	uint32_t pkt_emp_vtem_contents7;
	uint32_t pkt0_emp_cvtem_contents0;
	uint32_t pkt0_emp_cvtem_contents1;
	uint32_t pkt0_emp_cvtem_contents2;
	uint32_t pkt0_emp_cvtem_contents3;
	uint32_t pkt0_emp_cvtem_contents4;
	uint32_t pkt0_emp_cvtem_contents5;
	uint32_t pkt0_emp_cvtem_contents6;
	uint32_t pkt0_emp_cvtem_contents7;
	uint32_t pkt1_emp_cvtem_contents0;
	uint32_t pkt1_emp_cvtem_contents1;
	uint32_t pkt1_emp_cvtem_contents2;
	uint32_t pkt1_emp_cvtem_contents3;
	uint32_t pkt1_emp_cvtem_contents4;
	uint32_t pkt1_emp_cvtem_contents5;
	uint32_t pkt1_emp_cvtem_contents6;
	uint32_t pkt1_emp_cvtem_contents7;
	uint32_t pkt2_emp_cvtem_contents0;
	uint32_t pkt2_emp_cvtem_contents1;
	uint32_t pkt2_emp_cvtem_contents2;
	uint32_t pkt2_emp_cvtem_contents3;
	uint32_t pkt2_emp_cvtem_contents4;
	uint32_t pkt2_emp_cvtem_contents5;
	uint32_t pkt2_emp_cvtem_contents6;
	uint32_t pkt2_emp_cvtem_contents7;
	uint32_t pkt3_emp_cvtem_contents0;
	uint32_t pkt3_emp_cvtem_contents1;
	uint32_t pkt3_emp_cvtem_contents2;
	uint32_t pkt3_emp_cvtem_contents3;
	uint32_t pkt3_emp_cvtem_contents4;
	uint32_t pkt3_emp_cvtem_contents5;
	uint32_t pkt3_emp_cvtem_contents6;
	uint32_t pkt3_emp_cvtem_contents7;
	uint32_t pkt4_emp_cvtem_contents0;
	uint32_t pkt4_emp_cvtem_contents1;
	uint32_t pkt4_emp_cvtem_contents2;
	uint32_t pkt4_emp_cvtem_contents3;
	uint32_t pkt4_emp_cvtem_contents4;
	uint32_t pkt4_emp_cvtem_contents5;
	uint32_t pkt4_emp_cvtem_contents6;
	uint32_t pkt4_emp_cvtem_contents7;
	uint32_t pkt5_emp_cvtem_contents0;
	uint32_t pkt5_emp_cvtem_contents1;
	uint32_t pkt5_emp_cvtem_contents2;
	uint32_t pkt5_emp_cvtem_contents3;
	uint32_t pkt5_emp_cvtem_contents4;
	uint32_t pkt5_emp_cvtem_contents5;
	uint32_t pkt5_emp_cvtem_contents6;
	uint32_t pkt5_emp_cvtem_contents7;
	uint8_t res34[64];
	uint32_t audpkt_control0;
	uint32_t audpkt_control1;
	uint8_t res35[24];
	uint32_t audpkt_acr_control0;
	uint32_t audpkt_acr_control1;
	uint8_t res36[4];
	uint32_t audpkt_acr_status0;
	uint8_t res37[16];
	uint32_t audpkt_chstatus_ovr0;
	uint32_t audpkt_chstatus_ovr1;
	uint32_t audpkt_chstatus_ovr2;
	uint32_t audpkt_chstatus_ovr3;
	uint32_t audpkt_chstatus_ovr4;
	uint32_t audpkt_chstatus_ovr5;
	uint32_t audpkt_chstatus_ovr6;
	uint32_t audpkt_chstatus_ovr7;
	uint32_t audpkt_chstatus_ovr8;
	uint8_t res38[28];
	uint32_t audpkt_usrdata_ovr_msg_generic0;
	uint32_t audpkt_usrdata_ovr_msg_generic1;
	uint32_t audpkt_usrdata_ovr_msg_generic2;
	uint32_t audpkt_usrdata_ovr_msg_generic3;
	uint32_t audpkt_usrdata_ovr_msg_generic4;
	uint32_t audpkt_usrdata_ovr_msg_generic5;
	uint32_t audpkt_usrdata_ovr_msg_generic6;
	uint32_t audpkt_usrdata_ovr_msg_generic7;
	uint32_t audpkt_usrdata_ovr_msg_generic8;
	uint32_t audpkt_usrdata_ovr_msg_generic9;
	uint32_t audpkt_usrdata_ovr_msg_generic10;
	uint32_t audpkt_usrdata_ovr_msg_generic11;
	uint32_t audpkt_usrdata_ovr_msg_generic12;
	uint32_t audpkt_usrdata_ovr_msg_generic13;
	uint32_t audpkt_usrdata_ovr_msg_generic14;
	uint32_t audpkt_usrdata_ovr_msg_generic15;
	uint32_t audpkt_usrdata_ovr_msg_generic16;
	uint32_t audpkt_usrdata_ovr_msg_generic17;
	uint32_t audpkt_usrdata_ovr_msg_generic18;
	uint32_t audpkt_usrdata_ovr_msg_generic19;
	uint32_t audpkt_usrdata_ovr_msg_generic20;
	uint32_t audpkt_usrdata_ovr_msg_generic21;
	uint32_t audpkt_usrdata_ovr_msg_generic22;
	uint32_t audpkt_usrdata_ovr_msg_generic23;
	uint32_t audpkt_usrdata_ovr_msg_generic24;
	uint32_t audpkt_usrdata_ovr_msg_generic25;
	uint32_t audpkt_usrdata_ovr_msg_generic26;
	uint32_t audpkt_usrdata_ovr_msg_generic27;
	uint32_t audpkt_usrdata_ovr_msg_generic28;
	uint32_t audpkt_usrdata_ovr_msg_generic29;
	uint32_t audpkt_usrdata_ovr_msg_generic30;
	uint32_t audpkt_usrdata_ovr_msg_generic31;
	uint32_t audpkt_usrdata_ovr_msg_generic32;
	uint32_t audpkt_vbit_ovr0;
	uint8_t res39[216];
	uint32_t cec_tx_control;
	uint32_t cec_status;
	uint32_t cec_config;
	uint32_t cec_addr;
	uint8_t res40[16];
	uint32_t cec_tx_count;
	uint32_t cec_tx_data3_0;
	uint32_t cec_tx_data7_4;
	uint32_t cec_tx_data11_8;
	uint32_t cec_tx_data15_12;
	uint8_t res41[12];
	uint32_t cec_rx_count_status;
	uint32_t cec_rx_data3_0;
	uint32_t cec_rx_data7_4;
	uint32_t cec_rx_data11_8;
	uint32_t cec_rx_data15_12;
	uint32_t cec_lock_control;
	uint8_t res42[8];
	uint32_t cec_rxqual_bittime_config;
	uint32_t cec_rx_bittime_config;
	uint32_t cec_tx_bittime_config;
	uint8_t res43[1940];
	uint32_t earcrx_cmdc_config0;
	uint32_t earcrx_cmdc_config1;
	uint32_t earcrx_cmdc_control;
	uint32_t earcrx_cmdc_whitelist0_config;
	uint32_t earcrx_cmdc_whitelist1_config;
	uint32_t earcrx_cmdc_whitelist2_config;
	uint32_t earcrx_cmdc_whitelist3_config;
	uint32_t earcrx_cmdc_status;
	uint32_t earcrx_cmdc_xact_info;
	uint32_t earcrx_cmdc_xact_action;
	uint32_t earcrx_cmdc_heartbeat_rxstat_set;
	uint32_t earcrx_cmdc_heartbeat_status;
	uint8_t res44[16];
	uint32_t earcrx_cmdc_xact_wr0;
	uint32_t earcrx_cmdc_xact_wr1;
	uint32_t earcrx_cmdc_xact_wr2;
	uint32_t earcrx_cmdc_xact_wr3;
	uint32_t earcrx_cmdc_xact_wr4;
	uint32_t earcrx_cmdc_xact_wr5;
	uint32_t earcrx_cmdc_xact_wr6;
	uint32_t earcrx_cmdc_xact_wr7;
	uint32_t earcrx_cmdc_xact_wr8;
	uint32_t earcrx_cmdc_xact_wr9;
	uint32_t earcrx_cmdc_xact_wr10;
	uint32_t earcrx_cmdc_xact_wr11;
	uint32_t earcrx_cmdc_xact_wr12;
	uint32_t earcrx_cmdc_xact_wr13;
	uint32_t earcrx_cmdc_xact_wr14;
	uint32_t earcrx_cmdc_xact_wr15;
	uint32_t earcrx_cmdc_xact_wr16;
	uint32_t earcrx_cmdc_xact_wr17;
	uint32_t earcrx_cmdc_xact_wr18;
	uint32_t earcrx_cmdc_xact_wr19;
	uint32_t earcrx_cmdc_xact_wr20;
	uint32_t earcrx_cmdc_xact_wr21;
	uint32_t earcrx_cmdc_xact_wr22;
	uint32_t earcrx_cmdc_xact_wr23;
	uint32_t earcrx_cmdc_xact_wr24;
	uint32_t earcrx_cmdc_xact_wr25;
	uint32_t earcrx_cmdc_xact_wr26;
	uint32_t earcrx_cmdc_xact_wr27;
	uint32_t earcrx_cmdc_xact_wr28;
	uint32_t earcrx_cmdc_xact_wr29;
	uint32_t earcrx_cmdc_xact_wr30;
	uint32_t earcrx_cmdc_xact_wr31;
	uint32_t earcrx_cmdc_xact_wr32;
	uint32_t earcrx_cmdc_xact_wr33;
	uint32_t earcrx_cmdc_xact_wr34;
	uint32_t earcrx_cmdc_xact_wr35;
	uint32_t earcrx_cmdc_xact_wr36;
	uint32_t earcrx_cmdc_xact_wr37;
	uint32_t earcrx_cmdc_xact_wr38;
	uint32_t earcrx_cmdc_xact_wr39;
	uint32_t earcrx_cmdc_xact_wr40;
	uint32_t earcrx_cmdc_xact_wr41;
	uint32_t earcrx_cmdc_xact_wr42;
	uint32_t earcrx_cmdc_xact_wr43;
	uint32_t earcrx_cmdc_xact_wr44;
	uint32_t earcrx_cmdc_xact_wr45;
	uint32_t earcrx_cmdc_xact_wr46;
	uint32_t earcrx_cmdc_xact_wr47;
	uint32_t earcrx_cmdc_xact_wr48;
	uint32_t earcrx_cmdc_xact_wr49;
	uint32_t earcrx_cmdc_xact_wr50;
	uint32_t earcrx_cmdc_xact_wr51;
	uint32_t earcrx_cmdc_xact_wr52;
	uint32_t earcrx_cmdc_xact_wr53;
	uint32_t earcrx_cmdc_xact_wr54;
	uint32_t earcrx_cmdc_xact_wr55;
	uint32_t earcrx_cmdc_xact_wr56;
	uint32_t earcrx_cmdc_xact_wr57;
	uint32_t earcrx_cmdc_xact_wr58;
	uint32_t earcrx_cmdc_xact_wr59;
	uint32_t earcrx_cmdc_xact_wr60;
	uint32_t earcrx_cmdc_xact_wr61;
	uint32_t earcrx_cmdc_xact_wr62;
	uint32_t earcrx_cmdc_xact_wr63;
	uint32_t earcrx_cmdc_xact_wr64;
	uint8_t res45[28];
	uint32_t earcrx_cmdc_xact_rd0;
	uint32_t earcrx_cmdc_xact_rd1;
	uint32_t earcrx_cmdc_xact_rd2;
	uint32_t earcrx_cmdc_xact_rd3;
	uint32_t earcrx_cmdc_xact_rd4;
	uint32_t earcrx_cmdc_xact_rd5;
	uint32_t earcrx_cmdc_xact_rd6;
	uint32_t earcrx_cmdc_xact_rd7;
	uint32_t earcrx_cmdc_xact_rd8;
	uint32_t earcrx_cmdc_xact_rd9;
	uint32_t earcrx_cmdc_xact_rd10;
	uint32_t earcrx_cmdc_xact_rd11;
	uint32_t earcrx_cmdc_xact_rd12;
	uint32_t earcrx_cmdc_xact_rd13;
	uint32_t earcrx_cmdc_xact_rd14;
	uint32_t earcrx_cmdc_xact_rd15;
	uint32_t earcrx_cmdc_xact_rd16;
	uint32_t earcrx_cmdc_xact_rd17;
	uint32_t earcrx_cmdc_xact_rd18;
	uint32_t earcrx_cmdc_xact_rd19;
	uint32_t earcrx_cmdc_xact_rd20;
	uint32_t earcrx_cmdc_xact_rd21;
	uint32_t earcrx_cmdc_xact_rd22;
	uint32_t earcrx_cmdc_xact_rd23;
	uint32_t earcrx_cmdc_xact_rd24;
	uint32_t earcrx_cmdc_xact_rd25;
	uint32_t earcrx_cmdc_xact_rd26;
	uint32_t earcrx_cmdc_xact_rd27;
	uint32_t earcrx_cmdc_xact_rd28;
	uint32_t earcrx_cmdc_xact_rd29;
	uint32_t earcrx_cmdc_xact_rd30;
	uint32_t earcrx_cmdc_xact_rd31;
	uint32_t earcrx_cmdc_xact_rd32;
	uint32_t earcrx_cmdc_xact_rd33;
	uint32_t earcrx_cmdc_xact_rd34;
	uint32_t earcrx_cmdc_xact_rd35;
	uint32_t earcrx_cmdc_xact_rd36;
	uint32_t earcrx_cmdc_xact_rd37;
	uint32_t earcrx_cmdc_xact_rd38;
	uint32_t earcrx_cmdc_xact_rd39;
	uint32_t earcrx_cmdc_xact_rd40;
	uint32_t earcrx_cmdc_xact_rd41;
	uint32_t earcrx_cmdc_xact_rd42;
	uint32_t earcrx_cmdc_xact_rd43;
	uint32_t earcrx_cmdc_xact_rd44;
	uint32_t earcrx_cmdc_xact_rd45;
	uint32_t earcrx_cmdc_xact_rd46;
	uint32_t earcrx_cmdc_xact_rd47;
	uint32_t earcrx_cmdc_xact_rd48;
	uint32_t earcrx_cmdc_xact_rd49;
	uint32_t earcrx_cmdc_xact_rd50;
	uint32_t earcrx_cmdc_xact_rd51;
	uint32_t earcrx_cmdc_xact_rd52;
	uint32_t earcrx_cmdc_xact_rd53;
	uint32_t earcrx_cmdc_xact_rd54;
	uint32_t earcrx_cmdc_xact_rd55;
	uint32_t earcrx_cmdc_xact_rd56;
	uint32_t earcrx_cmdc_xact_rd57;
	uint32_t earcrx_cmdc_xact_rd58;
	uint32_t earcrx_cmdc_xact_rd59;
	uint32_t earcrx_cmdc_xact_rd60;
	uint32_t earcrx_cmdc_xact_rd61;
	uint32_t earcrx_cmdc_xact_rd62;
	uint32_t earcrx_cmdc_xact_rd63;
	uint32_t earcrx_cmdc_xact_rd64;
	uint8_t res46[156];
	uint32_t earcrx_cmdc_sync_config;
	uint8_t res47[252];
	uint32_t earcrx_dmac_phy_control;
	uint8_t res48[4];
	uint32_t earcrx_dmac_config;
	uint32_t earcrx_dmac_control0;
	uint32_t earcrx_dmac_control1;
	uint32_t earcrx_dmac_status;
	uint32_t earcrx_dmac_chstatus0;
	uint32_t earcrx_dmac_chstatus1;
	uint32_t earcrx_dmac_chstatus2;
	uint32_t earcrx_dmac_chstatus3;
	uint32_t earcrx_dmac_chstatus4;
	uint32_t earcrx_dmac_chstatus5;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt0;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt1;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt2;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt3;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt4;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt5;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt6;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt7;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt8;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt9;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt10;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_acp_pkt11;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt0;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt1;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt2;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt3;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt4;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt5;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt6;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt7;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt8;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt9;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt10;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc1_pkt11;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt0;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt1;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt2;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt3;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt4;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt5;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt6;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt7;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt8;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt9;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt10;
	uint32_t earcrx_dmac_usrdata_msg_hdmi_isrc2_pkt11;
	uint32_t earcrx_dmac_usrdata_msg_generic0;
	uint32_t earcrx_dmac_usrdata_msg_generic1;
	uint32_t earcrx_dmac_usrdata_msg_generic2;
	uint32_t earcrx_dmac_usrdata_msg_generic3;
	uint32_t earcrx_dmac_usrdata_msg_generic4;
	uint32_t earcrx_dmac_usrdata_msg_generic5;
	uint32_t earcrx_dmac_usrdata_msg_generic6;
	uint32_t earcrx_dmac_usrdata_msg_generic7;
	uint32_t earcrx_dmac_usrdata_msg_generic8;
	uint32_t earcrx_dmac_usrdata_msg_generic9;
	uint32_t earcrx_dmac_usrdata_msg_generic10;
	uint32_t earcrx_dmac_usrdata_msg_generic11;
	uint32_t earcrx_dmac_usrdata_msg_generic12;
	uint32_t earcrx_dmac_usrdata_msg_generic13;
	uint32_t earcrx_dmac_usrdata_msg_generic14;
	uint32_t earcrx_dmac_usrdata_msg_generic15;
	uint32_t earcrx_dmac_usrdata_msg_generic16;
	uint32_t earcrx_dmac_usrdata_msg_generic17;
	uint32_t earcrx_dmac_usrdata_msg_generic18;
	uint32_t earcrx_dmac_usrdata_msg_generic19;
	uint32_t earcrx_dmac_usrdata_msg_generic20;
	uint32_t earcrx_dmac_usrdata_msg_generic21;
	uint32_t earcrx_dmac_usrdata_msg_generic22;
	uint32_t earcrx_dmac_usrdata_msg_generic23;
	uint32_t earcrx_dmac_usrdata_msg_generic24;
	uint32_t earcrx_dmac_usrdata_msg_generic25;
	uint32_t earcrx_dmac_usrdata_msg_generic26;
	uint32_t earcrx_dmac_usrdata_msg_generic27;
	uint32_t earcrx_dmac_usrdata_msg_generic28;
	uint32_t earcrx_dmac_usrdata_msg_generic29;
	uint32_t earcrx_dmac_usrdata_msg_generic30;
	uint32_t earcrx_dmac_usrdata_msg_generic31;
	uint32_t earcrx_dmac_usrdata_msg_generic32;
	uint32_t earcrx_dmac_chstatus_streamer0;
	uint32_t earcrx_dmac_chstatus_streamer1;
	uint32_t earcrx_dmac_chstatus_streamer2;
	uint32_t earcrx_dmac_chstatus_streamer3;
	uint32_t earcrx_dmac_chstatus_streamer4;
	uint32_t earcrx_dmac_chstatus_streamer5;
	uint32_t earcrx_dmac_chstatus_streamer6;
	uint32_t earcrx_dmac_chstatus_streamer7;
	uint32_t earcrx_dmac_chstatus_streamer8;
	uint32_t earcrx_dmac_chstatus_streamer9;
	uint32_t earcrx_dmac_chstatus_streamer10;
	uint32_t earcrx_dmac_chstatus_streamer11;
	uint32_t earcrx_dmac_chstatus_streamer12;
	uint32_t earcrx_dmac_chstatus_streamer13;
	uint32_t earcrx_dmac_chstatus_streamer14;
	uint32_t earcrx_dmac_usrdata_streamer0;
	uint8_t res49[4732];
	uint32_t main_intvec_index;
	uint8_t res50[12];
	uint32_t mainunit_0_int_status;
	uint32_t mainunit_0_int_mask_n;
	uint32_t mainunit_0_int_clear;
	uint32_t mainunit_0_int_force;
	uint32_t mainunit_1_int_status;
	uint32_t mainunit_1_int_mask_n;
	uint32_t mainunit_1_int_clear;
	uint32_t mainunit_1_int_force;
	uint8_t res51[2000];
	uint32_t avp_intvec_index;
	uint8_t res52[12];
	uint32_t avp_0_int_status;
	uint32_t avp_0_int_mask_n;
	uint32_t avp_0_int_clear;
	uint32_t avp_0_int_force;
	uint32_t avp_1_int_status;
	uint32_t avp_1_int_mask_n;
	uint32_t avp_1_int_clear;
	uint32_t avp_1_int_force;
	uint32_t avp_2_int_status;
	uint32_t avp_2_int_mask_n;
	uint32_t avp_2_int_clear;
	uint32_t avp_2_int_force;
	uint32_t avp_3_int_status;
	uint32_t avp_3_int_mask_n;
	uint32_t avp_3_int_clear;
	uint32_t avp_3_int_force;
	uint32_t avp_4_int_status;
	uint32_t avp_4_int_mask_n;
	uint32_t avp_4_int_clear;
	uint32_t avp_4_int_force;
	uint32_t avp_5_int_status;
	uint32_t avp_5_int_mask_n;
	uint32_t avp_5_int_clear;
	uint32_t avp_5_int_force;
	uint32_t avp_6_int_status;
	uint32_t avp_6_int_mask_n;
	uint32_t avp_6_int_clear;
	uint32_t avp_6_int_force;
	uint8_t res53[1920];
	uint32_t cec_int_status;
	uint32_t cec_int_mask_n;
	uint32_t cec_int_clear;
	uint32_t cec_int_force;
	uint8_t res54[2032];
	uint32_t earcrx_intvec_index;
	uint8_t res55[12];
	uint32_t earcrx_0_int_status;
	uint32_t earcrx_0_int_mask_n;
	uint32_t earcrx_0_int_clear;
	uint32_t earcrx_0_int_force;
	uint32_t earcrx_1_int_status;
	uint32_t earcrx_1_int_mask_n;
	uint32_t earcrx_1_int_clear;
	uint32_t earcrx_1_int_force;
};

struct HdptxPhy {
	uint32_t cmn_reg0000;
	uint8_t res_0004[0x0020 - 0x0004];
	uint32_t cmn_reg0008;
	uint32_t cmn_reg0009;
	uint32_t cmn_reg000A;
	uint32_t cmn_reg000B;
	uint8_t res_0030[0x0080 - 0x0030];
	uint32_t cmn_reg0020;
	uint32_t cmn_reg0021;
	uint8_t res_0088[0x0094 - 0x0088];
	uint32_t cmn_reg0025;
	uint8_t res_0098[0x00A0 - 0x0098];
	uint32_t cmn_reg0028;
	uint8_t res_00A4[0x00B4 - 0x00A4];
	uint32_t cmn_reg002D;
	uint32_t cmn_reg002E;
	uint32_t cmn_reg002F;
	uint8_t res_00C0[0x00F0 - 0x00C0];
	uint32_t cmn_reg003C;
	uint32_t cmn_reg003D;
	uint8_t res_00F8[0x0138 - 0x00F8];
	uint32_t cmn_reg004E;
	uint8_t res_013C[0x0144 - 0x013C];
	uint32_t cmn_reg0051;
	uint32_t res84838;
	uint32_t cmn_reg0053;
	uint32_t res83282;
	uint32_t cmn_reg0055;
	uint8_t res_0158[0x0168 - 0x0158];
	uint32_t cmn_reg005A;
	uint8_t res_016C[0x0170 - 0x016C];
	uint32_t cmn_reg005C;
	uint32_t res98239442;
	uint32_t cmn_reg005E;
	uint32_t res182301230;
	uint32_t cmn_reg0060;
	uint32_t res02342;
	uint32_t res012831;
	uint32_t cmn_reg0063;
	uint32_t cmn_reg0064;
	uint32_t cmn_reg0065;
	uint8_t res_0198[0x01A4 - 0x0198];
	uint32_t cmn_reg0069;
	uint32_t res123123[2];
	uint32_t cmn_reg006C;
	uint8_t res_01B4[0x01C0 - 0x01B4];
	uint32_t cmn_reg0070;
	uint32_t res123121231233[3];
	uint32_t cmn_reg0074;
	uint8_t res_01D4[0x0204 - 0x01D4];
	uint32_t cmn_reg0081;
	uint32_t cmn_reg0082;
	uint32_t cmn_reg0083;
	uint32_t cmn_reg0084;
	uint32_t cmn_reg0085;
	//uint8_t res_0218[0x0224 - 0x0218];
	uint32_t cmn_reg0086;
	uint32_t cmn_reg0087;
	uint32_t res0920123;
	uint32_t cmn_reg0089;
	uint8_t res_0228[0x0254 - 0x0228];
	uint32_t cmn_reg0095;
//	uint8_t res_0258[0x0264 - 0x0258];
	uint32_t res01231;
	uint32_t cmn_reg0097;
	uint32_t res01923123;
	uint32_t cmn_reg0099;
	uint32_t cmn_reg009a;
	uint32_t cmn_reg009B;
	uint32_t res0812312[2];
	uint32_t cmn_reg009E;
	uint32_t cmn_reg009F;
	uint8_t res_0284[0x0408 - 0x0284];
	uint32_t res1023213;
	uint32_t sb_reg0102;
	uint32_t sb_reg0103;
	uint32_t sb_reg0104;
	uint32_t sb_reg0105;
	uint32_t sb_reg0106;
	uint8_t res_041C[0x0424 - 0x041C];
	uint32_t sb_reg0109;
	uint32_t res0123111[3];
	uint32_t sb_reg010D;
	uint32_t res10231;
	uint32_t sb_reg010F;
	uint32_t sb_reg0110;
	uint32_t res1231231[2];
	uint32_t sb_reg0113;
	uint32_t sb_reg0114;
	uint32_t sb_reg0115;
	uint32_t sb_reg0116;
	uint32_t sb_reg0117;
	uint32_t sb_reg0118;
	uint32_t sb_reg0119;
	uint32_t sb_reg011A;
	uint32_t sb_reg011B;
	uint32_t sb_reg011C;
	uint32_t sb_reg011D;
	uint32_t sb_reg011E;
	uint32_t sb_reg011F;
	uint32_t sb_reg0120;
	uint32_t res128922[2];
	uint32_t sb_reg0123;
	uint32_t res0120321[220];
	uint32_t intop_reg0200;
	uint32_t intop_reg0201;
	uint32_t res12893123;
	uint32_t intop_reg0203;
	uint32_t intop_reg0204;
	uint32_t intop_reg0205;
	uint32_t intop_reg0206;
	uint32_t intop_reg0207;
	uint8_t res_0820[0x0C0C - 0x0820];
	uint32_t lane_reg0303;
	uint32_t res19283132[3];
	uint32_t lane_reg0307;
	uint32_t res81238213[2];
	uint32_t lane_reg030A;
	uint32_t lane_reg030B;
	uint32_t lane_reg030C;
	uint32_t lane_reg030D;
	uint32_t lane_reg030E;
	uint32_t lane_reg030F;
	uint32_t lane_reg0310;
	uint32_t lane_reg0311;
	uint32_t lane_reg0312;
	uint32_t res12312323[3];
	uint32_t lane_reg0316;
	uint32_t res1231223[4];
	uint32_t lane_reg031B;
	uint32_t res1230123[3];
	uint32_t lane_reg031F;
	uint32_t lane_reg0320;
	uint32_t lane_reg0403;
//	uint32_t lane_reg0407;
//	uint32_t lane_reg040A;
//	uint32_t lane_reg040B;
//	uint32_t lane_reg040C;
//	uint32_t lane_reg040D;
//	uint32_t lane_reg040E;
//	uint32_t lane_reg0410;
//	uint32_t lane_reg0411;
//	uint32_t lane_reg0412;
//	uint32_t lane_reg0416;
//	uint32_t lane_reg041B;
//	uint32_t lane_reg041F;
//	uint32_t lane_reg0503;
//	uint32_t lane_reg0507;
//	uint32_t lane_reg050A;
//	uint32_t lane_reg050C;
//	uint32_t lane_reg050F;
//	uint32_t lane_reg0510;
//	uint32_t lane_reg0511;
};


#endif

#endif
