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
