struct PMU1_IOC {
    uint32_t GPIO0A_IOMUX_SEL_L;   // Offset 0x0000
    uint32_t GPIO0A_IOMUX_SEL_H;   // Offset 0x0004
    uint32_t GPIO0B_IOMUX_SEL_L;   // Offset 0x0008
    uint32_t GPIO0A_DS_L;          // Offset 0x0010
    uint32_t GPIO0A_DS_H;          // Offset 0x0014
    uint32_t GPIO0B_DS_L;          // Offset 0x0018
    uint32_t GPIO0A_P;             // Offset 0x0020
    uint32_t GPIO0B_P;             // Offset 0x0024
    uint32_t GPIO0A_IE;            // Offset 0x0028
    uint32_t GPIO0B_IE;            // Offset 0x002C
    uint32_t GPIO0A_SMT;           // Offset 0x0030
    uint32_t GPIO0B_SMT;           // Offset 0x0034
    uint32_t GPIO0A_PDIS;          // Offset 0x0038
    uint32_t GPIO0B_PDIS;          // Offset 0x003C
    uint32_t XIN_CON;              // Offset 0x0040
};

struct PMU2_IOC {
    uint32_t GPIO0B_IOMUX_SEL_H;   // Offset 0x0000
    uint32_t GPIO0C_IOMUX_SEL_L;   // Offset 0x0004
    uint32_t GPIO0C_IOMUX_SEL_H;   // Offset 0x0008
    uint32_t GPIO0D_IOMUX_SEL_L;   // Offset 0x000C
    uint32_t GPIO0D_IOMUX_SEL_H;   // Offset 0x0010
    uint32_t GPIO0B_DS_H;          // Offset 0x0014
    uint32_t GPIO0C_DS_L;          // Offset 0x0018
    uint32_t GPIO0C_DS_H;          // Offset 0x001C
    uint32_t GPIO0D_DS_L;          // Offset 0x0020
    uint32_t GPIO0D_DS_H;          // Offset 0x0024
    uint32_t GPIO0B_P;             // Offset 0x0028
    uint32_t GPIO0C_P;             // Offset 0x002C
    uint32_t GPIO0D_P;             // Offset 0x0030
    uint32_t GPIO0B_IE;            // Offset 0x0034
    uint32_t GPIO0C_IE;            // Offset 0x0038
    uint32_t GPIO0D_IE;            // Offset 0x003C
    uint32_t GPIO0B_SMT;           // Offset 0x0040
    uint32_t GPIO0C_SMT;           // Offset 0x0044
    uint32_t GPIO0D_SMT;           // Offset 0x0048
    uint32_t GPIO0B_PDIS;          // Offset 0x004C
    uint32_t GPIO0C_PDIS;          // Offset 0x0050
    uint32_t GPIO0D_PDIS;          // Offset 0x0054
};

struct BUS_IOC {
    uint32_t reserved_0000[3];              // Reserved space (0x0000 - 0x000B)
    uint32_t GPIO0B_IOMUX_SEL_H;    // Offset 0x000C
    uint32_t GPIO0C_IOMUX_SEL_L;    // Offset 0x0010
    uint32_t GPIO0C_IOMUX_SEL_H;    // Offset 0x0014
    uint32_t GPIO0D_IOMUX_SEL_L;    // Offset 0x0018
    uint32_t GPIO0D_IOMUX_SEL_H;    // Offset 0x001C
    uint32_t GPIO1A_IOMUX_SEL_L;    // Offset 0x0020
    uint32_t GPIO1A_IOMUX_SEL_H;    // Offset 0x0024
    uint32_t GPIO1B_IOMUX_SEL_L;    // Offset 0x0028
    uint32_t GPIO1B_IOMUX_SEL_H;    // Offset 0x002C
    uint32_t GPIO1C_IOMUX_SEL_L;    // Offset 0x0030
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

struct GPIO {
	uint32_t GPIO_SWPORT_DR_L;      // Offset: 0x0000, Port Data Register (Low)
	uint32_t GPIO_SWPORT_DR_H;      // Offset: 0x0004, Port Data Register (High)
	uint32_t GPIO_SWPORT_DDR_L;     // Offset: 0x0008, Port Data Direction Register (Low)
	uint32_t GPIO_SWPORT_DDR_H;     // Offset: 0x000C, Port Data Direction Register (High)
	uint32_t GPIO_INT_EN_L;         // Offset: 0x0010, Interrupt Enable Register (Low)
	uint32_t GPIO_INT_EN_H;         // Offset: 0x0014, Interrupt Enable Register (High)
	uint32_t GPIO_INT_MASK_L;       // Offset: 0x0018, Interrupt Mask Register (Low)
	uint32_t GPIO_INT_MASK_H;       // Offset: 0x001C, Interrupt Mask Register (High)
	uint32_t GPIO_INT_TYPE_L;       // Offset: 0x0020, Interrupt Level Register (Low)
	uint32_t GPIO_INT_TYPE_H;       // Offset: 0x0024, Interrupt Level Register (High)
	uint32_t GPIO_INT_POLARITY_L;   // Offset: 0x0028, Interrupt Polarity Register (Low)
	uint32_t GPIO_INT_POLARITY_H;   // Offset: 0x002C, Interrupt Polarity Register (High)
};

struct PWM {
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

struct  PDM_Registers {
     uint32_t PDM_SYSCONFIG;     // 0x0000: PDM System Configure Register
     uint32_t PDM_CTRL0;         // 0x0004: PDM Control Register 0
     uint32_t PDM_CTRL1;         // 0x0008: PDM Control Register 1
     uint32_t PDM_CLK_CTRL;      // 0x000C: PDM Clock Control Register
     uint32_t PDM_HPF_CTRL;      // 0x0010: PDM High-pass Filter Control Register
     uint32_t PDM_FIFO_CTRL;     // 0x0014: PDM FIFO Control Register
     uint32_t PDM_DMA_CTRL;      // 0x0018: PDM DMA Control Register
     uint32_t PDM_INT_EN;        // 0x001C: PDM Interrupt Enable Register
     uint32_t PDM_INT_CLR;       // 0x0020: PDM Interrupt Clear Register
     uint32_t PDM_INT_ST;        // 0x0024: PDM Interrupt Status Register
     uint32_t res1[02];
     uint32_t PDM_RXFIFO_DATA_REG; // 0x0030: PDM Receive FIFO Data Register
     uint32_t PDM_DATA0R_REG;    // 0x0034: PDM Path0 Right Channel Data Register
     uint32_t PDM_DATA0L_REG;    // 0x0038: PDM Path0 Left Channel Data Register
     uint32_t PDM_DATA1R_REG;    // 0x003C: PDM Path1 Right Channel Data Register
     uint32_t PDM_DATA1L_REG;    // 0x0040: PDM Path1 Left Channel Data Register
     uint32_t PDM_DATA2R_REG;    // 0x0044: PDM Path2 Right Channel Data Register
     uint32_t PDM_DATA2L_REG;    // 0x0048: PDM Path2 Left Channel Data Register
     uint32_t PDM_DATA3R_REG;    // 0x004C: PDM Path3 Right Channel Data Register
     uint32_t PDM_DATA3L_REG;    // 0x0050: PDM Path3 Left Channel Data Register
     uint32_t PDM_DATA_VALID;    // 0x0054: PDM Path Data Valid Register
     uint32_t PDM_VERSION;       // 0x0058: PDM Version Register
    uint32_t reserved[233];              // Reserved space
     uint32_t PDM_INCR_RXDR;     // 0x0400: Increment Address Receive FIFO Data Register
};
