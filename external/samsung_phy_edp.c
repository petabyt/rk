/** @file

  Rockchip HDMI/DP Combo PHY with Samsung IP block
  Copyright (c) 2022, Linaro, Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <main.h>
#include <rk3588.h>

static void MmioWrite32(uintptr_t addr, uint32_t v) {
  ((volatile uint32_t *)addr)[0] = v;
}
static uint32_t MmioRead32(uintptr_t addr) {
	return ((volatile uint32_t *)addr)[0];
}
static void NanoSecondDelay(uint32_t ns) {
	usleep(ns / 1000);
}
static void MicroSecondDelay(uint32_t us) {
	usleep(us);
}

struct RockchipHdptxPhy {
  uint32_t  Id;
  uint32_t LANE_POLARITY_INVERT[4];
};

struct PhyConfigureOptsDp {
  uint32_t LINKRATE;
  uint32_t LANES;
  uint32_t VOLTAGE[4];
  uint32_t PRE[4];
  uint8_t SSC : 1;
  uint8_t SETRATE : 1;
  uint8_t SETLANES : 1;
  uint8_t SETVOLTAGES : 1;
};

#define BIT(x) (1 << (x))
#define BITS_PER_LONG 	(sizeof(unsigned long) * 8)
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define EREMOTEIO       121     /* Remote I/O error */
#define EINVAL          22      /* Invalid argument */
#define ETIMEDOUT       110     /* Connection timed out */

#define ANALOGIX_DP0_REG_BASE                    0xFDEC0000
#define ANALOGIX_DP1_REG_BASE                    0xFDED0000

#define ANALOGIX_DP_TX_SW_RESET                 0x14
#define ANALOGIX_DP_FUNC_EN_1                   0x18
#define ANALOGIX_DP_FUNC_EN_2                   0x1C
#define ANALOGIX_DP_VIDEO_CTL_1                 0x20
#define ANALOGIX_DP_VIDEO_CTL_2                 0x24
#define ANALOGIX_DP_VIDEO_CTL_3                 0x28

#define ANALOGIX_DP_VIDEO_CTL_8                 0x3C
#define ANALOGIX_DP_VIDEO_CTL_10                0x44

#define ANALOGIX_DP_TOTAL_LINE_CFG_L            0x48
#define ANALOGIX_DP_TOTAL_LINE_CFG_H            0x4C
#define ANALOGIX_DP_ACTIVE_LINE_CFG_L           0x50
#define ANALOGIX_DP_ACTIVE_LINE_CFG_H           0x54
#define ANALOGIX_DP_V_F_PORCH_CFG               0x58
#define ANALOGIX_DP_V_SYNC_WIDTH_CFG            0x5C
#define ANALOGIX_DP_V_B_PORCH_CFG               0x60
#define ANALOGIX_DP_TOTAL_PIXEL_CFG_L           0x64
#define ANALOGIX_DP_TOTAL_PIXEL_CFG_H           0x68
#define ANALOGIX_DP_ACTIVE_PIXEL_CFG_L          0x6C
#define ANALOGIX_DP_ACTIVE_PIXEL_CFG_H          0x70
#define ANALOGIX_DP_H_F_PORCH_CFG_L             0x74
#define ANALOGIX_DP_H_F_PORCH_CFG_H             0x78
#define ANALOGIX_DP_H_SYNC_CFG_L                0x7C
#define ANALOGIX_DP_H_SYNC_CFG_H                0x80
#define ANALOGIX_DP_H_B_PORCH_CFG_L             0x84
#define ANALOGIX_DP_H_B_PORCH_CFG_H             0x88

#define ANALOGIX_DP_PLL_REG_1                   0xfc
#define ANALOGIX_DP_PLL_REG_2                   0x9e4
#define ANALOGIX_DP_PLL_REG_3                   0x9e8
#define ANALOGIX_DP_PLL_REG_4                   0x9ec
#define ANALOGIX_DP_PLL_REG_5                   0xa00

#define ANALOGIX_DP_BIAS                        0x124
#define ANALOGIX_DP_PD                          0x12c

#define ANALOGIX_DP_LANE_MAP                    0x35C

#define ANALOGIX_DP_ANALOG_CTL_1                0x370
#define ANALOGIX_DP_ANALOG_CTL_2                0x374
#define ANALOGIX_DP_ANALOG_CTL_3                0x378
#define ANALOGIX_DP_PLL_FILTER_CTL_1            0x37C
#define ANALOGIX_DP_TX_AMP_TUNING_CTL           0x380

#define ANALOGIX_DP_AUX_HW_RETRY_CTL            0x390

#define ANALOGIX_DP_COMMON_INT_STA_1            0x3C4
#define ANALOGIX_DP_COMMON_INT_STA_2            0x3C8
#define ANALOGIX_DP_COMMON_INT_STA_3            0x3CC
#define ANALOGIX_DP_COMMON_INT_STA_4            0x3D0

#define ANALOGIX_DP_INT_STA                     0x3DC
#define ANALOGIX_DP_COMMON_INT_MASK_1           0x3E0
#define ANALOGIX_DP_COMMON_INT_MASK_2           0x3E4
#define ANALOGIX_DP_COMMON_INT_MASK_3           0x3E8
#define ANALOGIX_DP_COMMON_INT_MASK_4           0x3EC
#define ANALOGIX_DP_INT_STA_MASK                0x3F8
#define ANALOGIX_DP_INT_CTL                     0x3FC

#define ANALOGIX_DP_SYS_CTL_1                   0x600
#define ANALOGIX_DP_SYS_CTL_2                   0x604
#define ANALOGIX_DP_SYS_CTL_3                   0x608
#define ANALOGIX_DP_SYS_CTL_4                   0x60C

#define ANALOGIX_DP_PKT_SEND_CTL                0x640
#define ANALOGIX_DP_HDCP_CTL                    0x648

#define ANALOGIX_DP_LINK_BW_SET                 0x680
#define ANALOGIX_DP_LANE_COUNT_SET              0x684
#define ANALOGIX_DP_TRAINING_PTN_SET            0x688
#define ANALOGIX_DP_LN0_LINK_TRAINING_CTL       0x68C
#define ANALOGIX_DP_LN1_LINK_TRAINING_CTL       0x690
#define ANALOGIX_DP_LN2_LINK_TRAINING_CTL       0x694
#define ANALOGIX_DP_LN3_LINK_TRAINING_CTL       0x698

#define ANALOGIX_DP_DEBUG_CTL                   0x6C0
#define ANALOGIX_DP_HPD_DEGLITCH_L              0x6C4
#define ANALOGIX_DP_HPD_DEGLITCH_H              0x6C8
#define ANALOGIX_DP_LINK_DEBUG_CTL              0x6E0

#define ANALOGIX_DP_M_VID_0                     0x700
#define ANALOGIX_DP_M_VID_1                     0x704
#define ANALOGIX_DP_M_VID_2                     0x708
#define ANALOGIX_DP_N_VID_0                     0x70C
#define ANALOGIX_DP_N_VID_1                     0x710
#define ANALOGIX_DP_N_VID_2                     0x714

#define ANALOGIX_DP_PLL_CTL			0x71C
#define ANALOGIX_DP_PHY_PD			0x720
#define ANALOGIX_DP_PHY_TEST			0x724

#define ANALOGIX_DP_VIDEO_FIFO_THRD		0x730
#define ANALOGIX_DP_AUDIO_MARGIN		0x73C

#define ANALOGIX_DP_M_VID_GEN_FILTER_TH		0x764
#define ANALOGIX_DP_M_AUD_GEN_FILTER_TH		0x778
#define ANALOGIX_DP_AUX_CH_STA			0x780
#define ANALOGIX_DP_AUX_CH_DEFER_CTL		0x788
#define ANALOGIX_DP_AUX_RX_COMM			0x78C
#define ANALOGIX_DP_BUFFER_DATA_CTL		0x790
#define ANALOGIX_DP_AUX_CH_CTL_1		0x794
#define ANALOGIX_DP_AUX_ADDR_7_0		0x798
#define ANALOGIX_DP_AUX_ADDR_15_8		0x79C
#define ANALOGIX_DP_AUX_ADDR_19_16		0x7A0
#define ANALOGIX_DP_AUX_CH_CTL_2		0x7A4

#define ANALOGIX_DP_BUF_DATA_0			0x7C0

#define ANALOGIX_DP_SOC_GENERAL_CTL		0x800

/* ANALOGIX_DP_TX_SW_RESET */
#define RESET_DP_TX				(0x1 << 0)

/* ANALOGIX_DP_FUNC_EN_1 */
#define MASTER_VID_FUNC_EN_N			(0x1 << 7)
#define SLAVE_VID_FUNC_EN_N			(0x1 << 5)
#define AUD_FIFO_FUNC_EN_N			(0x1 << 4)
#define AUD_FUNC_EN_N				(0x1 << 3)
#define HDCP_FUNC_EN_N				(0x1 << 2)
#define CRC_FUNC_EN_N				(0x1 << 1)
#define SW_FUNC_EN_N				(0x1 << 0)

/* ANALOGIX_DP_FUNC_EN_2 */
#define SSC_FUNC_EN_N				(0x1 << 7)
#define AUX_FUNC_EN_N				(0x1 << 2)
#define SERDES_FIFO_FUNC_EN_N			(0x1 << 1)
#define LS_CLK_DOMAIN_FUNC_EN_N			(0x1 << 0)

/* ANALOGIX_DP_VIDEO_CTL_1 */
#define VIDEO_EN				(0x1 << 7)
#define HDCP_VIDEO_MUTE				(0x1 << 6)

/* ANALOGIX_DP_VIDEO_CTL_1 */
#define IN_D_RANGE_MASK				(0x1 << 7)
#define IN_D_RANGE_SHIFT			(7)
#define IN_D_RANGE_CEA				(0x1 << 7)
#define IN_D_RANGE_VESA				(0x0 << 7)
#define IN_BPC_MASK				(0x7 << 4)
#define IN_BPC_SHIFT				(4)
#define IN_BPC_12_BITS				(0x3 << 4)
#define IN_BPC_10_BITS				(0x2 << 4)
#define IN_BPC_8_BITS				(0x1 << 4)
#define IN_BPC_6_BITS				(0x0 << 4)
#define IN_COLOR_F_MASK				(0x3 << 0)
#define IN_COLOR_F_SHIFT			(0)
#define IN_COLOR_F_YCBCR444			(0x2 << 0)
#define IN_COLOR_F_YCBCR422			(0x1 << 0)
#define IN_COLOR_F_RGB				(0x0 << 0)

/* ANALOGIX_DP_VIDEO_CTL_3 */
#define IN_YC_COEFFI_MASK			(0x1 << 7)
#define IN_YC_COEFFI_SHIFT			(7)
#define IN_YC_COEFFI_ITU709			(0x1 << 7)
#define IN_YC_COEFFI_ITU601			(0x0 << 7)
#define VID_CHK_UPDATE_TYPE_MASK		(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_SHIFT		(4)
#define VID_CHK_UPDATE_TYPE_1			(0x1 << 4)
#define VID_CHK_UPDATE_TYPE_0			(0x0 << 4)

/* ANALOGIX_DP_VIDEO_CTL_8 */
#define VID_HRES_TH(x)				(((x) & 0xf) << 4)
#define VID_VRES_TH(x)				(((x) & 0xf) << 0)

/* ANALOGIX_DP_VIDEO_CTL_10 */
#define FORMAT_SEL				(0x1 << 4)
#define INTERACE_SCAN_CFG			(0x1 << 2)
#define VSYNC_POLARITY_CFG			(0x1 << 1)
#define HSYNC_POLARITY_CFG			(0x1 << 0)

/* ANALOGIX_DP_TOTAL_LINE_CFG_L */
#define TOTAL_LINE_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_TOTAL_LINE_CFG_H */
#define TOTAL_LINE_CFG_H(x)			(((x) & 0xf) << 0)

/* ANALOGIX_DP_ACTIVE_LINE_CFG_L */
#define ACTIVE_LINE_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_ACTIVE_LINE_CFG_H */
#define ACTIVE_LINE_CFG_H(x)			(((x) & 0xf) << 0)

/* ANALOGIX_DP_V_F_PORCH_CFG */
#define V_F_PORCH_CFG(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_V_SYNC_WIDTH_CFG */
#define V_SYNC_WIDTH_CFG(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_V_B_PORCH_CFG */
#define V_B_PORCH_CFG(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_TOTAL_PIXEL_CFG_L */
#define TOTAL_PIXEL_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_TOTAL_PIXEL_CFG_H */
#define TOTAL_PIXEL_CFG_H(x)			(((x) & 0x3f) << 0)

/* ANALOGIX_DP_ACTIVE_PIXEL_CFG_L */
#define ACTIVE_PIXEL_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_ACTIVE_PIXEL_CFG_H */
#define ACTIVE_PIXEL_CFG_H(x)			(((x) & 0x3f) << 0)

/* ANALOGIX_DP_H_F_PORCH_CFG_L */
#define H_F_PORCH_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_H_F_PORCH_CFG_H */
#define H_F_PORCH_CFG_H(x)			(((x) & 0xf) << 0)

/* ANALOGIX_DP_H_SYNC_CFG_L */
#define H_SYNC_CFG_L(x)				(((x) & 0xff) << 0)

/* ANALOGIX_DP_H_SYNC_CFG_H */
#define H_SYNC_CFG_H(x)				(((x) & 0xf) << 0)

/* ANALOGIX_DP_H_B_PORCH_CFG_L */
#define H_B_PORCH_CFG_L(x)			(((x) & 0xff) << 0)

/* ANALOGIX_DP_H_B_PORCH_CFG_H */
#define H_B_PORCH_CFG_H(x)			(((x) & 0xf) << 0)

/* ANALOGIX_DP_PLL_REG_1 */
#define REF_CLK_24M				(0x1 << 0)
#define REF_CLK_27M				(0x0 << 0)
#define REF_CLK_MASK				(0x1 << 0)

/* ANALOGIX_DP_LANE_MAP */
#define LANE3_MAP_LOGIC_LANE_0			(0x0 << 6)
#define LANE3_MAP_LOGIC_LANE_1			(0x1 << 6)
#define LANE3_MAP_LOGIC_LANE_2			(0x2 << 6)
#define LANE3_MAP_LOGIC_LANE_3			(0x3 << 6)
#define LANE2_MAP_LOGIC_LANE_0			(0x0 << 4)
#define LANE2_MAP_LOGIC_LANE_1			(0x1 << 4)
#define LANE2_MAP_LOGIC_LANE_2			(0x2 << 4)
#define LANE2_MAP_LOGIC_LANE_3			(0x3 << 4)
#define LANE1_MAP_LOGIC_LANE_0			(0x0 << 2)
#define LANE1_MAP_LOGIC_LANE_1			(0x1 << 2)
#define LANE1_MAP_LOGIC_LANE_2			(0x2 << 2)
#define LANE1_MAP_LOGIC_LANE_3			(0x3 << 2)
#define LANE0_MAP_LOGIC_LANE_0			(0x0 << 0)
#define LANE0_MAP_LOGIC_LANE_1			(0x1 << 0)
#define LANE0_MAP_LOGIC_LANE_2			(0x2 << 0)
#define LANE0_MAP_LOGIC_LANE_3			(0x3 << 0)

/* ANALOGIX_DP_ANALOG_CTL_1 */
#define TX_TERMINAL_CTRL_50_OHM			(0x1 << 4)

/* ANALOGIX_DP_ANALOG_CTL_2 */
#define SEL_24M					(0x1 << 3)
#define TX_DVDD_BIT_1_0625V			(0x4 << 0)

/* ANALOGIX_DP_ANALOG_CTL_3 */
#define DRIVE_DVDD_BIT_1_0625V			(0x4 << 5)
#define VCO_BIT_600_MICRO			(0x5 << 0)

/* ANALOGIX_DP_PLL_FILTER_CTL_1 */
#define PD_RING_OSC				(0x1 << 6)
#define AUX_TERMINAL_CTRL_50_OHM		(0x2 << 4)
#define TX_CUR1_2X				(0x1 << 2)
#define TX_CUR_16_MA				(0x3 << 0)

/* ANALOGIX_DP_TX_AMP_TUNING_CTL */
#define CH3_AMP_400_MV				(0x0 << 24)
#define CH2_AMP_400_MV				(0x0 << 16)
#define CH1_AMP_400_MV				(0x0 << 8)
#define CH0_AMP_400_MV				(0x0 << 0)

/* ANALOGIX_DP_AUX_HW_RETRY_CTL */
#define AUX_BIT_PERIOD_EXPECTED_DELAY(x)	(((x) & 0x7) << 8)
#define AUX_HW_RETRY_INTERVAL_MASK		(0x3 << 3)
#define AUX_HW_RETRY_INTERVAL_600_MICROSECONDS	(0x0 << 3)
#define AUX_HW_RETRY_INTERVAL_800_MICROSECONDS	(0x1 << 3)
#define AUX_HW_RETRY_INTERVAL_1000_MICROSECONDS	(0x2 << 3)
#define AUX_HW_RETRY_INTERVAL_1800_MICROSECONDS	(0x3 << 3)
#define AUX_HW_RETRY_COUNT_SEL(x)		(((x) & 0x7) << 0)

/* ANALOGIX_DP_COMMON_INT_STA_1 */
#define VSYNC_DET				(0x1 << 7)
#define PLL_LOCK_CHG				(0x1 << 6)
#define SPDIF_ERR				(0x1 << 5)
#define SPDIF_UNSTBL				(0x1 << 4)
#define VID_FORMAT_CHG				(0x1 << 3)
#define AUD_CLK_CHG				(0x1 << 2)
#define VID_CLK_CHG				(0x1 << 1)
#define SW_INT					(0x1 << 0)

/* ANALOGIX_DP_COMMON_INT_STA_2 */
#define ENC_EN_CHG				(0x1 << 6)
#define HW_BKSV_RDY				(0x1 << 3)
#define HW_SHA_DONE				(0x1 << 2)
#define HW_AUTH_STATE_CHG			(0x1 << 1)
#define HW_AUTH_DONE				(0x1 << 0)

/* ANALOGIX_DP_COMMON_INT_STA_3 */
#define AFIFO_UNDER				(0x1 << 7)
#define AFIFO_OVER				(0x1 << 6)
#define R0_CHK_FLAG				(0x1 << 5)

/* ANALOGIX_DP_COMMON_INT_STA_4 */
#define PSR_ACTIVE				(0x1 << 7)
#define PSR_INACTIVE				(0x1 << 6)
#define SPDIF_BI_PHASE_ERR			(0x1 << 5)
#define HOTPLUG_CHG				(0x1 << 2)
#define HPD_LOST				(0x1 << 1)
#define PLUG					(0x1 << 0)

/* ANALOGIX_DP_INT_STA */
#define INT_HPD					(0x1 << 6)
#define HW_TRAINING_FINISH			(0x1 << 5)
#define RPLY_RECEIV				(0x1 << 1)
#define AUX_ERR					(0x1 << 0)

/* ANALOGIX_DP_INT_CTL */
#define SOFT_INT_CTRL				(0x1 << 2)
#define INT_POL1				(0x1 << 1)
#define INT_POL0				(0x1 << 0)

/* ANALOGIX_DP_SYS_CTL_1 */
#define DET_STA					(0x1 << 2)
#define FORCE_DET				(0x1 << 1)
#define DET_CTRL				(0x1 << 0)

/* ANALOGIX_DP_SYS_CTL_2 */
#define CHA_CRI(x)				(((x) & 0xf) << 4)
#define CHA_STA					(0x1 << 2)
#define FORCE_CHA				(0x1 << 1)
#define CHA_CTRL				(0x1 << 0)

/* ANALOGIX_DP_SYS_CTL_3 */
#define HPD_STATUS				(0x1 << 6)
#define F_HPD					(0x1 << 5)
#define HPD_CTRL				(0x1 << 4)
#define HDCP_RDY				(0x1 << 3)
#define STRM_VALID				(0x1 << 2)
#define F_VALID					(0x1 << 1)
#define VALID_CTRL				(0x1 << 0)

/* ANALOGIX_DP_SYS_CTL_4 */
#define FIX_M_AUD				(0x1 << 4)
#define ENHANCED				(0x1 << 3)
#define FIX_M_VID				(0x1 << 2)
#define M_VID_UPDATE_CTRL			(0x3 << 0)

/* ANALOGIX_DP_TRAINING_PTN_SET */
#define SCRAMBLER_TYPE				(0x1 << 9)
#define HW_LINK_TRAINING_PATTERN		(0x1 << 8)
#define SCRAMBLING_DISABLE			(0x1 << 5)
#define SCRAMBLING_ENABLE			(0x0 << 5)
#define LINK_QUAL_PATTERN_SET_MASK		(0x3 << 2)
#define LINK_QUAL_PATTERN_SET_PRBS7		(0x3 << 2)
#define LINK_QUAL_PATTERN_SET_D10_2		(0x1 << 2)
#define LINK_QUAL_PATTERN_SET_DISABLE		(0x0 << 2)
#define SW_TRAINING_PATTERN_SET_MASK		(0x3 << 0)
#define SW_TRAINING_PATTERN_SET_PTN3		(0x3 << 0)
#define SW_TRAINING_PATTERN_SET_PTN2		(0x2 << 0)
#define SW_TRAINING_PATTERN_SET_PTN1		(0x1 << 0)
#define SW_TRAINING_PATTERN_SET_NORMAL		(0x0 << 0)

/* ANALOGIX_DP_LN0_LINK_TRAINING_CTL */
#define PRE_EMPHASIS_SET_MASK			(0x3 << 3)
#define PRE_EMPHASIS_SET_SHIFT			(3)

/* ANALOGIX_DP_DEBUG_CTL */
#define PLL_LOCK				(0x1 << 4)
#define F_PLL_LOCK				(0x1 << 3)
#define PLL_LOCK_CTRL				(0x1 << 2)
#define PN_INV					(0x1 << 0)

/* ANALOGIX_DP_PLL_CTL */
#define DP_PLL_PD				(0x1 << 7)
#define DP_PLL_RESET				(0x1 << 6)
#define DP_PLL_LOOP_BIT_DEFAULT			(0x1 << 4)
#define DP_PLL_REF_BIT_1_1250V			(0x5 << 0)
#define DP_PLL_REF_BIT_1_2500V			(0x7 << 0)

/* ANALOGIX_DP_PHY_PD */
#define DP_PHY_PD				(0x1 << 5)
#define AUX_PD					(0x1 << 4)
#define CH3_PD					(0x1 << 3)
#define CH2_PD					(0x1 << 2)
#define CH1_PD					(0x1 << 1)
#define CH0_PD					(0x1 << 0)

/* ANALOGIX_DP_PHY_TEST */
#define MACRO_RST				(0x1 << 5)
#define CH1_TEST				(0x1 << 1)
#define CH0_TEST				(0x1 << 0)

/* ANALOGIX_DP_AUX_CH_STA */
#define AUX_BUSY				(0x1 << 4)
#define AUX_STATUS_MASK				(0xf << 0)

/* ANALOGIX_DP_AUX_CH_DEFER_CTL */
#define DEFER_CTRL_EN				(0x1 << 7)
#define DEFER_COUNT(x)				(((x) & 0x7f) << 0)

/* ANALOGIX_DP_AUX_RX_COMM */
#define AUX_RX_COMM_I2C_DEFER			(0x2 << 2)
#define AUX_RX_COMM_AUX_DEFER			(0x2 << 0)

/* ANALOGIX_DP_BUFFER_DATA_CTL */
#define BUF_CLR					(0x1 << 7)
#define BUF_DATA_COUNT(x)			(((x) & 0x1f) << 0)

/* ANALOGIX_DP_AUX_CH_CTL_1 */
#define AUX_LENGTH(x)				(((x - 1) & 0xf) << 4)
#define AUX_TX_COMM_MASK			(0xf << 0)
#define AUX_TX_COMM_DP_TRANSACTION		(0x1 << 3)
#define AUX_TX_COMM_I2C_TRANSACTION		(0x0 << 3)
#define AUX_TX_COMM_MOT				(0x1 << 2)
#define AUX_TX_COMM_WRITE			(0x0 << 0)
#define AUX_TX_COMM_READ			(0x1 << 0)

/* ANALOGIX_DP_AUX_ADDR_7_0 */
#define AUX_ADDR_7_0(x)				(((x) >> 0) & 0xff)

/* ANALOGIX_DP_AUX_ADDR_15_8 */
#define AUX_ADDR_15_8(x)			(((x) >> 8) & 0xff)

/* ANALOGIX_DP_AUX_ADDR_19_16 */
#define AUX_ADDR_19_16(x)			(((x) >> 16) & 0x0f)

/* ANALOGIX_DP_AUX_CH_CTL_2 */
#define ADDR_ONLY				(0x1 << 1)
#define AUX_EN					(0x1 << 0)

/* ANALOGIX_DP_SOC_GENERAL_CTL */
#define AUDIO_MODE_SPDIF_MODE			(0x1 << 8)
#define AUDIO_MODE_MASTER_MODE			(0x0 << 8)
#define MASTER_VIDEO_INTERLACE_EN		(0x1 << 4)
#define VIDEO_MASTER_CLK_SEL			(0x1 << 2)
#define VIDEO_MASTER_MODE_EN			(0x1 << 1)
#define VIDEO_MODE_MASK				(0x1 << 0)
#define VIDEO_MODE_SLAVE_MODE			(0x1 << 0)
#define VIDEO_MODE_MASTER_MODE			(0x0 << 0)

#define DP_TIMEOUT_LOOP_COUNT 10000
#define MAX_CR_LOOP 5
#define MAX_EQ_LOOP 5

/* I2C EDID Chip ID, Slave Address */
#define I2C_EDID_DEVICE_ADDR			0x50
#define I2C_E_EDID_DEVICE_ADDR			0x30

#define EDID_BLOCK_LENGTH			0x80
#define EDID_HEADER_PATTERN			0x00
#define EDID_EXTENSION_FLAG			0x7e
#define EDID_CHECKSUM				0x7f

/* DP_MAX_LANE_COUNT */
#define DPCD_ENHANCED_FRAME_CAP(x)		(((x) >> 7) & 0x1)
#define DPCD_MAX_LANE_COUNT(x)			((x) & 0x1f)

/* DP_LANE_COUNT_SET */
#define DPCD_LANE_COUNT_SET(x)			((x) & 0x1f)

/* DP_TRAINING_LANE0_SET */
#define DPCD_PRE_EMPHASIS_SET(x)		(((x) & 0x3) << 3)
#define DPCD_PRE_EMPHASIS_GET(x)		(((x) >> 3) & 0x3)
#define DPCD_VOLTAGE_SWING_SET(x)		(((x) & 0x3) << 0)
#define DPCD_VOLTAGE_SWING_GET(x)		(((x) >> 0) & 0x3)

/* HDPTXPHy_GRF*/
#define HDPTXPHY0_GRF_BASE                      0xFD5E0000
#define HDPTXPHY1_GRF_BASE                      0xFD5E4000
#define HDPTXPHY_GRF_CON0                       0x00
#define HDPTXPHY_GRF_CON1                       0x04
#define HDPTXPHY_GRF_STATUS0                    0x80
#define PCS_BIASES                              0x27c
#define O_PLL_LOCK                              0x280
#define O_PLL_LOCK_done                         (0x1<<3)
#define O_PHY_RDY                               ((0x1<<1)&(0x1<<3))

/* ENABLE_BIASES */
#define ENABLE_BIASES_LANE6                     (0x1<<6)
#define ENABLE_BIASES_LANE7                     (0x1<<7)

/* INITIAL POR RESET*/
#define INITIAL_POR_RESET_LANE2                 (0x1<<2)
#define INITIAL_POR_RESET_LANE3                 (0x1<<3)

/* DRM_DP_HELPER */
#define DP_RECEIVER_CAP_SIZE                    0xf
#define EDP_PSR_RECEIVER_CAP_SIZE	              2
#define EDP_DISPLAY_CTL_CAP_SIZE	              3

#define DP_TRAINING_AUX_RD_INTERVAL             0x00e   /* XXX 1.2? */
# define DP_TRAINING_AUX_RD_MASK                0x7F    /* XXX 1.2? */
# define DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT (1 << 7) /* DP 1.3 */

#define DP_SET_POWER                        0x600
# define DP_SET_POWER_D0                    0x1
# define DP_SET_POWER_D3                    0x2
# define DP_SET_POWER_MASK                  0x3
# define DP_SET_POWER_D3_AUX_ON             0x5

#define DP_EDP_DPCD_REV			    0x700    /* eDP 1.2 */
# define DP_EDP_11			    0x00
# define DP_EDP_12			    0x01
# define DP_EDP_13			    0x02
# define DP_EDP_14			    0x03

#define DP_TRAINING_LANE0_SET		    0x103
#define DP_TRAINING_LANE1_SET		    0x104
#define DP_TRAINING_LANE2_SET		    0x105
#define DP_TRAINING_LANE3_SET		    0x106

# define DP_TRAIN_VOLTAGE_SWING_MASK	    0x3
# define DP_TRAIN_VOLTAGE_SWING_SHIFT	    0
# define DP_TRAIN_MAX_SWING_REACHED	    (1 << 2)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_0 (0 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_1 (1 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_2 (2 << 0)
# define DP_TRAIN_VOLTAGE_SWING_LEVEL_3 (3 << 0)

# define DP_TRAIN_PRE_EMPHASIS_MASK	    (3 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_0		(0 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_1		(1 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_2		(2 << 3)
# define DP_TRAIN_PRE_EMPH_LEVEL_3		(3 << 3)

# define DP_TRAIN_PRE_EMPHASIS_SHIFT	    3
# define DP_TRAIN_MAX_PRE_EMPHASIS_REACHED  (1 << 5)

#define DP_TEST_REQUEST         0x218
# define DP_TEST_LINK_TRAINING        (1 << 0)
# define DP_TEST_LINK_VIDEO_PATTERN     (1 << 1)
# define DP_TEST_LINK_EDID_READ       (1 << 2)
# define DP_TEST_LINK_PHY_TEST_PATTERN      (1 << 3) /* DPCD >= 1.1 */
# define DP_TEST_LINK_FAUX_PATTERN      (1 << 4) /* DPCD >= 1.2 */

#define DP_ADJUST_REQUEST_LANE0_1     0x206
#define DP_TEST_RESPONSE        0x260
# define DP_TEST_ACK          (1 << 0)
# define DP_TEST_NAK          (1 << 1)
# define DP_TEST_EDID_CHECKSUM_WRITE      (1 << 2)

#define DP_TEST_EDID_CHECKSUM       0x261

/* AUX CH addresses */
/* DPCD */
#define DP_DPCD_REV                         0x000
# define DP_DPCD_REV_10                     0x10
# define DP_DPCD_REV_11                     0x11
# define DP_DPCD_REV_12                     0x12
# define DP_DPCD_REV_13                     0x13
# define DP_DPCD_REV_14                     0x14

#define EDP0TX_PHY_BASE                      0xFED60000
#define EDP1TX_PHY_BASE                      0xFED70000

#define DP_MAX_LANE_COUNT                   0x002
# define DP_MAX_LANE_COUNT_MASK             0x1f
# define DP_TPS3_SUPPORTED                  (1 << 6) /* 1.2 */
# define DP_ENHANCED_FRAME_CAP              (1 << 7)
#define DP_INC_BG                           (0x1 << 7)
#define DP_EXP_BG                           (0x1 << 6)
#define RK_AUX_PD                           (0x1 << 5)
#define AUX_PD                              (0x1 << 4)
#define RK_PLL_PD                           (0x1 << 4)
#define CH3_PD                              (0x1 << 3)
#define CH2_PD                              (0x1 << 2)
#define CH1_PD                              (0x1 << 1)
#define CH0_PD                              (0x1 << 0)



#define RO_REF_CLK_SEL				GENMASK(11, 10)
#define LC_REF_CLK_SEL				GENMASK(9, 8)
#define PLL_EN					BIT(7)
#define BIAS_EN					BIT(6)
#define BGR_EN					BIT(5)
#define HDPTX_MODE_SEL				BIT(0)
#define PLL_LOCK_DONE				BIT(3)
#define PHY_CLK_RDY				BIT(2)
#define PHY_RDY					BIT(1)
#define SB_RDY					BIT(0)

/* cmn_reg0008 */
#define OVRD_LCPLL_EN				BIT(7)
#define LCPLL_EN				BIT(6)

/* cmn_reg003C */
#define ANA_LCPLL_RESERVED7			BIT(7)

/* cmn_reg003D */
#define OVRD_ROPLL_EN				BIT(7)
#define ROPLL_EN				BIT(6)

/* cmn_reg0046 */
#define ROPLL_ANA_CPP_CTRL_COARSE		GENMASK(7, 4)
#define ROPLL_ANA_CPP_CTRL_FINE			GENMASK(3, 0)

/* cmn_reg0047 */
#define ROPLL_ANA_LPF_C_SEL_COARSE		GENMASK(5, 3)
#define ROPLL_ANA_LPF_C_SEL_FINE		GENMASK(2, 0)

/* cmn_reg004E */
#define ANA_ROPLL_PI_EN				BIT(5)

/* cmn_reg0051 */
#define ROPLL_PMS_MDIV				GENMASK(7, 0)

/* cmn_reg0055 */
#define ROPLL_PMS_MDIV_AFC			GENMASK(7, 0)

/* cmn_reg0059 */
#define ANA_ROPLL_PMS_PDIV			GENMASK(7, 4)
#define ANA_ROPLL_PMS_REFDIV			GENMASK(3, 0)

/* cmn_reg005A */
#define ROPLL_PMS_SDIV_RBR			GENMASK(7, 4)
#define ROPLL_PMS_SDIV_HBR			GENMASK(3, 0)

/* cmn_reg005B */
#define ROPLL_PMS_SDIV_HBR2			GENMASK(7, 4)
#define ROPLL_PMS_SDIV_HBR3			GENMASK(3, 0)

/* cmn_reg005D */
#define OVRD_ROPLL_REF_CLK_SEL			BIT(5)
#define ROPLL_REF_CLK_SEL			GENMASK(4, 3)

/* cmn_reg005E */
#define ANA_ROPLL_SDM_EN			BIT(6)
#define OVRD_ROPLL_SDM_RSTN			BIT(5)
#define ROPLL_SDM_RSTN				BIT(4)
#define ROPLL_SDC_FRACTIONAL_EN_RBR		BIT(3)
#define ROPLL_SDC_FRACTIONAL_EN_HBR		BIT(2)
#define ROPLL_SDC_FRACTIONAL_EN_HBR2		BIT(1)
#define ROPLL_SDC_FRACTIONAL_EN_HBR3		BIT(0)

/* cmn_reg005F */
#define OVRD_ROPLL_SDC_RSTN			BIT(5)
#define ROPLL_SDC_RSTN				BIT(4)

/* cmn_reg0060 */
#define ROPLL_SDM_DENOMINATOR			GENMASK(7, 0)

/* cmn_reg0064 */
#define ROPLL_SDM_NUMERATOR_SIGN_RBR		BIT(3)
#define ROPLL_SDM_NUMERATOR_SIGN_HBR		BIT(2)
#define ROPLL_SDM_NUMERATOR_SIGN_HBR2		BIT(1)
#define ROPLL_SDM_NUMERATOR_SIGN_HBR3		BIT(0)

/* cmn_reg0065 */
#define ROPLL_SDM_NUMERATOR			GENMASK(7, 0)

/* cmn_reg0069 */
#define ROPLL_SDC_N_RBR				GENMASK(2, 0)

/* cmn_reg006A */
#define ROPLL_SDC_N_HBR				GENMASK(5, 3)
#define ROPLL_SDC_N_HBR2			GENMASK(2, 0)

/* cmn_reg006B */
#define ROPLL_SDC_N_HBR3			GENMASK(3, 1)

/* cmn_reg006C */
#define ROPLL_SDC_NUMERATOR			GENMASK(5, 0)

/* cmn_reg0070 */
#define ROPLL_SDC_DENOMINATOR			GENMASK(5, 0)

/* cmn_reg0074 */
#define OVRD_ROPLL_SDC_NDIV_RSTN		BIT(3)
#define ROPLL_SDC_NDIV_RSTN			BIT(2)
#define OVRD_ROPLL_SSC_EN			BIT(1)
#define ROPLL_SSC_EN				BIT(0)

/* cmn_reg0075 */
#define ANA_ROPLL_SSC_FM_DEVIATION		GENMASK(5, 0)

/* cmn_reg0076 */
#define ANA_ROPLL_SSC_FM_FREQ			GENMASK(6, 2)

/* cmn_reg0077 */
#define ANA_ROPLL_SSC_CLK_DIV_SEL		GENMASK(6, 3)

/* cmn_reg0081 */
#define ANA_PLL_CD_TX_SER_RATE_SEL		BIT(3)
#define ANA_PLL_CD_HSCLK_WEST_EN		BIT(1)
#define ANA_PLL_CD_HSCLK_EAST_EN		BIT(0)

/* cmn_reg0082 */
#define ANA_PLL_CD_VREG_GAIN_CTRL		GENMASK(3, 0)

/* cmn_reg0083 */
#define ANA_PLL_CD_VREG_ICTRL			GENMASK(6, 5)

/* cmn_reg0084 */
#define PLL_LCRO_CLK_SEL			BIT(5)

/* cmn_reg0085 */
#define ANA_PLL_SYNC_LOSS_DET_MODE		GENMASK(1, 0)

/* cmn_reg0087 */
#define ANA_PLL_TX_HS_CLK_EN			BIT(2)

/* cmn_reg0095 */
#define DP_TX_LINK_BW				GENMASK(1, 0)

/* cmn_reg0097 */
#define DIG_CLK_SEL				BIT(1)

/* cmn_reg0099 */
#define SSC_EN					GENMASK(7, 6)
#define CMN_ROPLL_ALONE_MODE			BIT(2)

/* cmn_reg009A */
#define HS_SPEED_SEL				BIT(0)

/* cmn_reg009B */
#define LS_SPEED_SEL				BIT(4)

/* sb_reg0102 */
#define OVRD_SB_RXTERM_EN			BIT(5)
#define SB_RXRERM_EN				BIT(4)
#define ANA_SB_RXTERM_OFFSP			GENMASK(3, 0)

/* sb_reg0103 */
#define ANA_SB_RXTERM_OFFSN			GENMASK(6, 3)
#define OVRD_SB_RX_RESCAL_DONE			BIT(1)
#define SB_RX_RESCAL_DONE			BIT(0)

/* sb_reg0104 */
#define OVRD_SB_EN				BIT(5)
#define SB_EN					BIT(4)
#define OVRD_SB_AUX_EN				BIT(1)
#define SB_AUX_EN				BIT(0)

/* sb_reg0105 */
#define ANA_SB_TX_HLVL_PROG			GENMASK(2, 0)

/* sb_reg0106 */
#define ANA_SB_TX_LLVL_PROG			GENMASK(6, 4)

/* sb_reg010D */
#define ANA_SB_DMRX_LPBK_DATA			BIT(4)

/* sb_reg010F */
#define OVRD_SB_VREG_EN				BIT(7)
#define SB_VREG_EN				BIT(6)
#define ANA_SB_VREG_GAIN_CTRL			GENMASK(3, 0)

/* sb_reg0110 */
#define ANA_SB_VREG_OUT_SEL			BIT(1)
#define ANA_SB_VREG_REF_SEL			BIT(0)

/* sb_reg0113 */
#define SB_RX_RCAL_OPT_CODE			GENMASK(5, 4)
#define SB_RX_RTERM_CTRL			GENMASK(3, 0)

/* sb_reg0114 */
#define SB_TG_SB_EN_DELAY_TIME			GENMASK(5, 3)
#define SB_TG_RXTERN_EN_DELAY_TIME		GENMASK(2, 0)

/* sb_reg0115 */
#define SB_READY_DELAY_TIME			GENMASK(5, 3)
#define SB_TG_OSC_EN_DELAY_TIME			GENMASK(2, 0)

/* sb_reg0116 */
#define SB_TG_OSC_EN_TO_AFC_RSTN_DELAT_TIME	GENMASK(6, 4)

/* sb_reg0117 */
#define SB_TG_PLL_CD_VREG_FAST_PULSE_TIME	GENMASK(3, 0)

/* sb_reg0118 */
#define SB_TG_EARC_DMRX_RECVRD_CLK_CNT		GENMASK(7, 0)

/* sb_reg011A */
#define SB_TG_CNT_RUN_NO_7_0			GENMASK(7, 0)

/* sb_reg011B */
#define SB_EARC_SIG_DET_BYPASS			BIT(4)
#define SB_AFC_TOL				GENMASK(3, 0)

/* sb_reg011C */
#define SB_AFC_STB_NUM				GENMASK(3, 0)

/* sb_reg011D */
#define SB_TG_OSC_CNT_MIN			GENMASK(7, 0)

/* sb_reg011E */
#define SB_TG_OSC_CNT_MAX			GENMASK(7, 0)

/* sb_reg011F */
#define SB_PWM_AFC_CTRL				GENMASK(7, 2)
#define SB_RCAL_RSTN				BIT(1)

/* sb_reg0120 */
#define SB_AUX_EN_IN				BIT(7)

/* sb_reg0123 */
#define OVRD_SB_READY				BIT(5)
#define SB_READY				BIT(4)

/* lntop_reg0200 */
#define PROTOCOL_SEL				BIT(2)

/* lntop_reg0206 */
#define DATA_BUS_WIDTH				GENMASK(2, 1)
#define BUS_WIDTH_SEL				BIT(0)

/* lntop_reg0207 */
#define LANE_EN					GENMASK(3, 0)

/* LANE_reg0301 */
#define OVRD_LN_TX_DRV_EI_EN			BIT(7)
#define LN_TX_DRV_EI_EN				BIT(6)

/* LANE_reg0303 */
#define OVRD_LN_TX_DRV_LVL_CTRL			BIT(5)
#define LN_TX_DRV_LVL_CTRL			GENMASK(4, 0)

/* LANE_reg0304 */
#define OVRD_LN_TX_DRV_POST_LVL_CTRL    BIT(4)
#define LN_TX_DRV_POST_LVL_CTRL     GENMASK(3, 0)
#define LN_TX_DRV_POST_LVL_CTRL			GENMASK(3, 0)

/* LANE_reg0305 */
#define OVRD_LN_TX_DRV_PRE_LVL_CTRL		BIT(6)
#define LN_TX_DRV_PRE_LVL_CTRL			GENMASK(5, 2)

/* LANE_reg0306 */
#define LN_ANA_TX_DRV_IDRV_IDN_CTRL		GENMASK(7, 5)
#define LN_ANA_TX_DRV_IDRV_IUP_CTRL		GENMASK(4, 2)
#define LN_ANA_TX_DRV_ACCDRV_EN			BIT(0)

/* LANE_reg0307 */
#define LN_ANA_TX_DRV_ACCDRV_POL_SEL		BIT(6)
#define LN_ANA_TX_DRV_ACCDRV_CTRL		GENMASK(5, 3)

/* LANE_reg030A */
#define LN_ANA_TX_JEQ_EN			BIT(4)
#define LN_TX_JEQ_EVEN_CTRL_RBR			GENMASK(3, 0)

/* LANE_reg030B */
#define LN_TX_JEQ_EVEN_CTRL_HBR			GENMASK(7, 4)
#define LN_TX_JEQ_EVEN_CTRL_HBR2		GENMASK(3, 0)

/* LANE_reg030C */
#define LN_TX_JEQ_EVEN_CTRL_HBR3		GENMASK(7, 4)
#define LN_TX_JEQ_ODD_CTRL_RBR			GENMASK(3, 0)

/* LANE_reg030D */
#define LN_TX_JEQ_ODD_CTRL_HBR			GENMASK(7, 4)
#define LN_TX_JEQ_ODD_CTRL_HBR2			GENMASK(3, 0)

/* LANE_reg030E */
#define LN_TX_JEQ_ODD_CTRL_HBR3			GENMASK(7, 4)

/* LANE_reg0310 */
#define LN_ANA_TX_SYNC_LOSS_DET_MODE		GENMASK(1, 0)

/* LANE_reg0311 */
#define LN_TX_SER_40BIT_EN_RBR			BIT(3)
#define LN_TX_SER_40BIT_EN_HBR			BIT(2)
#define LN_TX_SER_40BIT_EN_HBR2			BIT(1)
#define LN_TX_SER_40BIT_EN_HBR3			BIT(0)

/* LANE_reg0316 */
#define LN_ANA_TX_SER_VREG_GAIN_CTRL		GENMASK(3, 0)

/* LANE_reg031B */
#define LN_ANA_TX_RESERVED			GENMASK(7, 0)

/* LANE_reg031E */
#define LN_POLARITY_INV				BIT(2)

#define LANE_REG(LANE, offset)			(0x400 * (LANE) + (offset))

enum {
  DP_BW_RBR,
  DP_BW_HBR,
  DP_BW_HBR2,
  DP_BW_HBR3,
};

struct TxDrvCtrl {
  uint8_t Tx_Drv_Lvl_Ctrl;
  uint8_t Tx_Drv_Post_Lvl_Ctrl;
  uint8_t Ana_Tx_Drv_Idrv_Idn_Ctrl;
  uint8_t Ana_Tx_Drv_Idrv_Iup_Ctrl;
  uint8_t Ana_Tx_Drv_Accdrv_En;
  uint8_t Ana_Tx_Drv_Accdrv_Ctrl;
};

static const struct TxDrvCtrl TX_DRV_CTRL_RBR[4][4] = {
  /* VOLTAGE swing 0, pre-emphasis 0->3 */
  {
    { 0x1, 0x0, 0x4, 0x6, 0x0, 0x4 },
    { 0x4, 0x3, 0x4, 0x6, 0x0, 0x4 },
    { 0x7, 0x6, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0xb, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 1, pre-emphasis 0->2 */
  {
    { 0x4, 0x0, 0x4, 0x6, 0x0, 0x4 },
    { 0xa, 0x5, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x8, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 2, pre-emphasis 0->1 */
  {
    { 0x8, 0x0, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x5, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 3, pre-emphasis 0 */
  {
    { 0xd, 0x0, 0x7, 0x7, 0x1, 0x4 },
  }
};

static const struct TxDrvCtrl TX_DRV_CTRL_HBR[4][4] = {
  /* VOLTAGE swing 0, pre-emphasis 0->3 */
  {
    { 0x2, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0x5, 0x4, 0x4, 0x6, 0x0, 0x4 },
    { 0x9, 0x8, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0xb, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 1, pre-emphasis 0->2 */
  {
    { 0x6, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0xb, 0x6, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x8, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 2, pre-emphasis 0->1 */
  {
    { 0x9, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x6, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 3, pre-emphasis 0 */
  {
    { 0xd, 0x1, 0x7, 0x7, 0x1, 0x4 },
  }
};

static const struct TxDrvCtrl TX_DRV_CTRL_HBR2[4][4] = {
  /* VOLTAGE swing 0, pre-emphasis 0->3 */
  {
    { 0x2, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0x5, 0x4, 0x4, 0x6, 0x0, 0x4 },
    { 0x9, 0x8, 0x4, 0x6, 0x1, 0x4 },
    { 0xd, 0xb, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 1, pre-emphasis 0->2 */
  {
    { 0x6, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0xc, 0x7, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x8, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 2, pre-emphasis 0->1 */
  {
    { 0x9, 0x1, 0x4, 0x6, 0x0, 0x4 },
    { 0xd, 0x6, 0x7, 0x7, 0x1, 0x7 },
  },

  /* VOLTAGE swing 3, pre-emphasis 0 */
  {
    { 0xd, 0x0, 0x7, 0x7, 0x1, 0x4 },
  }
};

void
PhyWrite (
  struct RockchipHdptxPhy *Hdptx,
  uint32_t Reg,
  uint32_t Val
  )
{
  uint32_t Shift;

  if (!Hdptx->Id)
    Shift = EDP0TX_PHY_BASE;
  else
    Shift = EDP1TX_PHY_BASE;

  MmioWrite32(Shift + Reg, Val );
}

uint32_t
PhyRead (
  struct RockchipHdptxPhy *Hdptx,
  uint32_t Reg
  )
{
  uint32_t Shift;

  if(!Hdptx->Id)
    Shift = EDP0TX_PHY_BASE;
  else
    Shift = EDP1TX_PHY_BASE;

  return MmioRead32(Shift + Reg);
}

static 
void 
PhyUpdateBits (
  struct RockchipHdptxPhy *Hdptx, 
  uint32_t Reg,
  uint32_t Mask, 
  uint32_t Val
  )
{
  uint32_t Orig, Tmp;

  Orig = PhyRead(Hdptx, Reg);
  Tmp = Orig & ~Mask;
  Tmp |= Val & Mask;
  PhyWrite(Hdptx, Reg, Tmp);
};

static
void
GrfWrite (
  struct RockchipHdptxPhy *Hdptx,
  uint32_t Reg,
  uint32_t Mask,
  uint32_t Val
  )
{
  uint32_t TempVal = 0;
  uint32_t Shift;

  if (!Hdptx->Id)
    Shift = HDPTXPHY0_GRF_BASE;
  else
    Shift = HDPTXPHY1_GRF_BASE;

  TempVal = (Mask << 16) | (Val & Mask);
  MmioWrite32(Shift + Reg, TempVal);
};

static
void
RockchipHdptxPhyDpPllInit (
  struct RockchipHdptxPhy *Hdptx
  )
{
  puts(__func__);
  PhyUpdateBits(Hdptx, 0x20, OVRD_LCPLL_EN |
      LCPLL_EN,
      0x80); 
  PhyWrite(Hdptx, 0xf4, 0xC0);
  PhyUpdateBits(Hdptx, 0x0138, ANA_ROPLL_PI_EN,
      0x20);
  PhyWrite(Hdptx, 0x0144, 0x87);
  PhyWrite(Hdptx, 0x0148, 0x71);
  PhyWrite(Hdptx, 0x014c, 0x71);
  PhyWrite(Hdptx, 0x0154, 0x87);
  PhyWrite(Hdptx, 0x0158, 0x71);
  PhyWrite(Hdptx, 0x015c, 0x71);
  PhyWrite(Hdptx, 0x0164, 0x11);
  PhyWrite(Hdptx, 0x0168, 0x31);
  PhyUpdateBits(Hdptx, 0x016c, ROPLL_PMS_SDIV_HBR2, 0x0);
  PhyWrite(Hdptx, 0x0178, 0x7f);
  PhyWrite(Hdptx, 0x017c, 0x31);
  PhyWrite(Hdptx, 0x0180, 0x21);
  PhyWrite(Hdptx, 0x0184, 0x27);
  PhyWrite(Hdptx, 0x0188, 0x27);
  PhyWrite(Hdptx, 0x0190, 0x7);
  PhyWrite(Hdptx, 0x0194, 0x0);
  PhyWrite(Hdptx, 0x0198, 0xd);
  PhyWrite(Hdptx, 0x019c, 0xd);
  PhyWrite(Hdptx, 0x01a4, 0x2);
  PhyWrite(Hdptx, 0x01a8, 0x09);
  PhyWrite(Hdptx, 0x01b0, 0x3);
  PhyWrite(Hdptx, 0x01b4, 0x7);
  PhyWrite(Hdptx, 0x01b8, 0x7);
  PhyWrite(Hdptx, 0x01c0, 0x8);
  PhyWrite(Hdptx, 0x01c4, 0x18);
  PhyWrite(Hdptx, 0x01c8, 0x18);
  PhyWrite(Hdptx, 0x01d0, 0x0f);
  PhyWrite(Hdptx, 0x01dc, 0x08);
  PhyWrite(Hdptx, 0x0118, 0xee);
  PhyWrite(Hdptx, 0x011c, 0x24);
  PhyWrite(Hdptx, 0x0204, 0x01);
  PhyUpdateBits(Hdptx, 0x025c, DIG_CLK_SEL, 0x2);
  PhyUpdateBits(Hdptx, 0x021c, ANA_PLL_TX_HS_CLK_EN, 0x4);
  PhyUpdateBits(Hdptx, 0x0204, ANA_PLL_CD_HSCLK_EAST_EN |
      ANA_PLL_CD_HSCLK_WEST_EN,
      0x1);
  PhyWrite(Hdptx, 0x0264, 0x84);
  PhyWrite(Hdptx, 0x0208, 0x04);
  PhyUpdateBits(Hdptx, 0x00f0, ANA_LCPLL_RESERVED7,
      0x80);
  PhyWrite(Hdptx, 0x020c, 0x24);
  PhyWrite(Hdptx, 0x0214, 0x03);
  PhyUpdateBits(Hdptx, 0x0210, PLL_LCRO_CLK_SEL,
      0x20);
  PhyUpdateBits(Hdptx, 0x0268, HS_SPEED_SEL,
      0x1);
  PhyUpdateBits(Hdptx, 0x026c, LS_SPEED_SEL,
      0x10);
}

static
int
RockchipHdptxPhyDpAuxInit (
  struct RockchipHdptxPhy *Hdptx
  )
{
  puts(__func__);
  PhyUpdateBits(Hdptx, 0x0414, ANA_SB_TX_HLVL_PROG, 0x7);
  PhyUpdateBits(Hdptx, 0x0418, ANA_SB_TX_LLVL_PROG, 0x70);
  PhyWrite(Hdptx, 0x044c, 0x13);
  PhyWrite(Hdptx, 0x0450, 0x12);
  PhyWrite(Hdptx, 0x0454, 0x12);
  PhyWrite(Hdptx, 0x0458, 0x20);
  PhyWrite(Hdptx, 0x045c, 0x04);
  PhyWrite(Hdptx, 0x0460, 0x0a);
  PhyWrite(Hdptx, 0x0468, 0x03);
  PhyWrite(Hdptx, 0x046c, 0x13);
  PhyWrite(Hdptx, 0x0470, 0x04);
  PhyUpdateBits(Hdptx, 0x0474, SB_TG_OSC_CNT_MIN, 0x67);
  PhyUpdateBits(Hdptx, 0x0478, SB_TG_OSC_CNT_MAX, 0x6a);
  PhyWrite(Hdptx, 0x047c, 0x16);
  PhyUpdateBits(Hdptx, 0x0434, ANA_SB_DMRX_LPBK_DATA, 0x10);
  PhyWrite(Hdptx, 0x0440, 0x03);
  PhyUpdateBits(Hdptx, 0x043c, ANA_SB_VREG_GAIN_CTRL, 0x0);
  PhyUpdateBits(Hdptx, 0x0408, ANA_SB_RXTERM_OFFSP, 0x3);
  PhyWrite(Hdptx, 0x040c, 0x1b);
  PhyUpdateBits(Hdptx, 0x047c, SB_RCAL_RSTN, 0x2);
  PhyUpdateBits(Hdptx, 0x0410, SB_AUX_EN, 0x0);
  PhyUpdateBits(Hdptx, 0x0480, SB_AUX_EN_IN, 0x80);
  PhyUpdateBits(Hdptx, 0x0410, OVRD_SB_EN, 0x20);
  PhyUpdateBits(Hdptx, 0x0408, OVRD_SB_RXTERM_EN, 0x20);
  PhyUpdateBits(Hdptx, 0x0410, OVRD_SB_AUX_EN, 0x2);

  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BGR_EN, 0x21);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BIAS_EN, 0x61);
  MicroSecondDelay(10);

  PhyUpdateBits(Hdptx, 0x0410, SB_EN, 0x10);
  PhyUpdateBits(Hdptx, 0x0408, SB_RXRERM_EN, 0x10);
  PhyWrite(Hdptx, 0x043c, 0xc0);
  PhyUpdateBits(Hdptx, 0x0410, SB_AUX_EN, 0x1);
  NanoSecondDelay (100000);
  GrfWrite(Hdptx, HDPTXPHY_GRF_STATUS0, SB_RDY, 0x1);
  MicroSecondDelay(10);
  return 0;
}

static
void
RockchipHdptxPhyReset (
  struct RockchipHdptxPhy *Hdptx
  )
{
  puts(__func__);
  uint32_t LANE;

  for (LANE = 0; LANE < 4; LANE++)
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c04),
        OVRD_LN_TX_DRV_EI_EN | LN_TX_DRV_EI_EN,
        0x80);

  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, PLL_EN, 0x0);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BIAS_EN, 0x0);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BGR_EN, 0x0);
};

void
RockchipHdptxPhyInit (
  struct RockchipHdptxPhy *Hdptx
  )
{
  puts(__func__);
  RockchipHdptxPhyReset(Hdptx);

  MmioWrite32(0xFD7F0A0C, 0xFFFF8000);

  /* Todo lane-polarity-invert  */
  PhyWrite(Hdptx, 0x0C78, 0x00);
  PhyWrite(Hdptx, 0x1078, 0x00);
  PhyWrite(Hdptx, 0x1478, 0x00);
  PhyWrite(Hdptx, 0x1878, 0x00);

  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, HDPTX_MODE_SEL, 0x01);

  /* Todo RockchipHdptxPhyPowerOn */
  PhyWrite(Hdptx, 0x0800, 0x00);
  PhyWrite(Hdptx, 0x0818, 0x02);

  RockchipHdptxPhyDpPllInit(Hdptx);
  RockchipHdptxPhyDpAuxInit(Hdptx);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BGR_EN, 0x21);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, BIAS_EN, 0x61);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, PLL_EN, 0xe1);
  // reset assert
  MmioWrite32(0xFD7F0A0C, 0xFFFF8000);
  MmioWrite32(0xFD7F0A0C, 0xFFFF0000);
  MmioWrite32(0xFD7F0A10, 0xFFFF0000);
};

int
RockchipHdptxPhySetRate (
  struct RockchipHdptxPhy *Hdptx,
  struct PhyConfigureOptsDp *Dp
  )
{
  puts(__func__);
  uint32_t BW;

	GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, PLL_EN, 0x0);
	PhyUpdateBits(Hdptx, 0x081c, LANE_EN, 0x0);

  switch (Dp->LINKRATE) {
  case 1620:
    BW = DP_BW_RBR;
    break;
  case 2700:
    BW = DP_BW_HBR;
    break;
  case 5400:
    BW = DP_BW_HBR2;
    break;
  default:
    return -EINVAL;
  }

  PhyUpdateBits(Hdptx, 0x0254, DP_TX_LINK_BW, BW);

  if (Dp->SSC) {
    PhyUpdateBits(Hdptx, 0x01d0, OVRD_ROPLL_SSC_EN | ROPLL_SSC_EN,
        0x3);
    PhyUpdateBits(Hdptx, 0x01d4, ANA_ROPLL_SSC_FM_DEVIATION,
        0xc);
    PhyUpdateBits(Hdptx, 0x01d8, ANA_ROPLL_SSC_FM_FREQ,
        0x7c);
    PhyUpdateBits(Hdptx, 0x0264, SSC_EN,
        0x80);
  } else {
    PhyUpdateBits(Hdptx, 0x01d0, OVRD_ROPLL_SSC_EN | ROPLL_SSC_EN,
        0x3);
    PhyUpdateBits(Hdptx, 0x01d4, ANA_ROPLL_SSC_FM_DEVIATION,
        0xe);
    PhyUpdateBits(Hdptx, 0x01d8, ANA_ROPLL_SSC_FM_FREQ,
        0xd);
    PhyUpdateBits(Hdptx, 0x0264, SSC_EN,
        0x20);
    PhyUpdateBits(Hdptx, 0x01d8, ANA_ROPLL_SSC_FM_FREQ,
        0x30);
  }
  PhyWrite(Hdptx, 0x081C, 0x03);
  PhyWrite(Hdptx, 0x0C0C, 0x26);
  PhyWrite(Hdptx, 0x0C10, 0x11);
  PhyWrite(Hdptx, 0x100C, 0x26);
  PhyWrite(Hdptx, 0x101C, 0x60);
  PhyWrite(Hdptx, 0x140C, 0x00);
  PhyWrite(Hdptx, 0x1410, 0x00);
  PhyWrite(Hdptx, 0x141C, 0x00);
  PhyWrite(Hdptx, 0x1428, 0x00);
  PhyWrite(Hdptx, 0x142C, 0x00);
  PhyWrite(Hdptx, 0x1434, 0x00);
  PhyWrite(Hdptx, 0x1444, 0x00);
  PhyWrite(Hdptx, 0x1458, 0x00);
  PhyWrite(Hdptx, 0x146C, 0x00);
  PhyWrite(Hdptx, 0x180C, 0x00);
  PhyWrite(Hdptx, 0x1810, 0x00);
  PhyWrite(Hdptx, 0x181C, 0x00);
  PhyWrite(Hdptx, 0x1828, 0x00);
  PhyWrite(Hdptx, 0x182C, 0x00);
  PhyWrite(Hdptx, 0x1834, 0x00);
  PhyWrite(Hdptx, 0x1844, 0x00);
  PhyWrite(Hdptx, 0x1858, 0x00);
  PhyWrite(Hdptx, 0x186C, 0x00);
  GrfWrite(Hdptx, HDPTXPHY_GRF_CON0, PLL_EN, 0x80);
  GrfWrite(Hdptx, HDPTXPHY_GRF_STATUS0, PLL_LOCK_DONE, 0x8);
  PhyUpdateBits(Hdptx, 0x081c, LANE_EN,
      GENMASK(Dp->LANES - 1, 0));
  NanoSecondDelay (100000);
  GrfWrite(Hdptx, HDPTXPHY_GRF_STATUS0, PHY_RDY, 0x2);
  NanoSecondDelay (10000);
  return 0;
}

void
RockchipHdptxPhySetVoltage (
  struct RockchipHdptxPhy *Hdptx,
  struct PhyConfigureOptsDp *Dp,
  uint8_t LANE
  )
{
  puts(__func__);
  const struct TxDrvCtrl *ctrl;

  switch (Dp->LINKRATE) {
  case 1620:
    ctrl = &TX_DRV_CTRL_RBR[Dp->VOLTAGE[LANE]][Dp->PRE[LANE]];
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c44),
        LN_TX_SER_40BIT_EN_RBR,
        0x8);
    break;
  case 2700:
    ctrl = &TX_DRV_CTRL_HBR[Dp->VOLTAGE[LANE]][Dp->PRE[LANE]];
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c2c),
        LN_TX_JEQ_EVEN_CTRL_HBR,
        0x70);
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c34),
        LN_TX_JEQ_ODD_CTRL_HBR,
        0x70);
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c44),
        LN_TX_SER_40BIT_EN_HBR,
        0x4);
    break;
  case 5400:
  default:
    ctrl = &TX_DRV_CTRL_HBR2[Dp->VOLTAGE[LANE]][Dp->PRE[LANE]];
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c2c),
        LN_TX_JEQ_EVEN_CTRL_HBR2,
        0x7);
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c34),
        LN_TX_JEQ_ODD_CTRL_HBR2,
        0x7);
    PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c44),
        LN_TX_SER_40BIT_EN_HBR2,
        0x2);
    break;
  }
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c0c),
      OVRD_LN_TX_DRV_LVL_CTRL | LN_TX_DRV_LVL_CTRL,
      0x2d);
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c10),
      OVRD_LN_TX_DRV_POST_LVL_CTRL | LN_TX_DRV_POST_LVL_CTRL,
      0x10 | ctrl->Tx_Drv_Post_Lvl_Ctrl
      );
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c18),
      LN_ANA_TX_DRV_IDRV_IDN_CTRL |
      LN_ANA_TX_DRV_IDRV_IUP_CTRL |
      LN_ANA_TX_DRV_ACCDRV_EN,
      0x00); 
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c1c),
      LN_ANA_TX_DRV_ACCDRV_POL_SEL | LN_ANA_TX_DRV_ACCDRV_CTRL,
      0x40 | ctrl->Ana_Tx_Drv_Accdrv_Ctrl);
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c6c),
      LN_ANA_TX_RESERVED,
      0x1);
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c58),
      LN_ANA_TX_SER_VREG_GAIN_CTRL,
      0x2);
  PhyUpdateBits(Hdptx, LANE_REG(LANE, 0x0c40),
      LN_ANA_TX_SYNC_LOSS_DET_MODE,
      0x3);
}

int 
RockchipHdptxPhySetVoltages (
  struct RockchipHdptxPhy *Hdptx,
  struct PhyConfigureOptsDp *Dp
  )
{
  uint8_t LANE;

  for (LANE = 0; LANE < Dp->LANES; LANE++)
    RockchipHdptxPhySetVoltage(Hdptx, Dp, LANE);

  return 0;
}

static
int
RockchipHdptxPhyVerifyConfig (
  struct RockchipHdptxPhy *Hdptx,
  struct PhyConfigureOptsDp *Dp
  )
{
  int i;

  if (Dp->SETRATE) {
    switch (Dp->LINKRATE) {
    case 1620:
    case 2700:
    case 5400:
      break;
    default:
      puts("Invalid params for link rate");
      return -EINVAL;
    }
  }

  switch (Dp->LANES) {
  case 1:
  case 2:
  case 4:
    break;
  default:
    puts("Invalid params for lanes");
    return -EINVAL;
  }

  if (Dp->SETVOLTAGES) {
    for (i = 0; i < Dp->LANES; i++) {
      if (Dp->VOLTAGE[i] > 3 || Dp->PRE[i] > 3) {
        puts("Invalid params for voltage");
        return -EINVAL;
      }

      if (Dp->VOLTAGE[i] + Dp->PRE[i] > 3) {
        puts("Invalid params for voltage");
        return -EINVAL;
      }
		}
	}

  return 0;
}

int 
RockchipHdptxPhyConfigure (
  struct RockchipHdptxPhy *Hdptx,
  struct PhyConfigureOptsDp *Dp
  )
{
  int Ret;

  Ret = RockchipHdptxPhyVerifyConfig(Hdptx, Dp);
  if (Ret) {
    puts("Invalid params for phy configure");
    return Ret;
  }

  if (Dp->SETRATE) {
    Ret = RockchipHdptxPhySetRate(Hdptx, Dp);
    if (Ret) {
      puts("Failed to set rate");
      return Ret;
    }
  }

  if (Dp->SETVOLTAGES) {
    Ret = RockchipHdptxPhySetVoltages(Hdptx, Dp);
    if (Ret) {
      puts("Failed to set Voltages");
      return Ret;
    }
  }

  return 0;
}

int hdptx_phy_configure_edp(int lanes, int linkrate_mbps) {
  puts("edk2_hdptx_phy_configure_edp");
  struct PhyConfigureOptsDp OPTS_DP;
  struct RockchipHdptxPhy Hdptx;
  Hdptx.Id = 1;

  OPTS_DP.LANES = lanes;
  OPTS_DP.LINKRATE = linkrate_mbps;
  OPTS_DP.SSC = 1;
  OPTS_DP.SETLANES = 1;
  OPTS_DP.SETRATE = 1;
  OPTS_DP.SETVOLTAGES = 1;
  // wtf
  OPTS_DP.VOLTAGE[0] = 2;
  OPTS_DP.VOLTAGE[1] = 2;
  OPTS_DP.PRE[0] = 1;
  OPTS_DP.PRE[1] = 1;

  RockchipHdptxPhyConfigure(&Hdptx, &OPTS_DP);
  puts("edk2_hdptx_phy_configure_edp exit");
  return 0;
}

int hdptx_phy_init(int id) {
  puts("edk2_hdptx_phy_init");
  struct RockchipHdptxPhy Hdptx;
  Hdptx.Id = id;

  if (id == 0) {
    MmioWrite32(0xFD5A8000, 0xFFFF0001);
  } else {
    MmioWrite32(0xFD5A8004, 0xFFFF0001);
  }

  RockchipHdptxPhyInit(&Hdptx);
  puts("edk2_hdptx_phy_init exit");
  return 0;
}
