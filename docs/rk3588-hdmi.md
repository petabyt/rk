# RK3588 display init sequence

# iomux/grf settings
A few registers must be setup for the HDMI TX:
```
VO1_GRF_CON3 (for TX0)
VO1_GRF_CON6 (for TX1)
GRF_SOC_CON7
VO1_GRF_CON9
```

The iomux settings for `SCL` and `SDA` pins for HDMI must be routed to the I2C controller of your choice.
As documented on [wikipedia](https://en.wikipedia.org/wiki/HDMI) pins 15 and 16 in the HDMI type A connector are
for I2C. This is used primarily to transport the [EDID](https://en.wikipedia.org/wiki/Extended_Display_Identification_Data) from the display,
which has important information like supported resolutions, timing info, refresh rate, etc.

(Looking at the TRM, it seems impossible for these pins to be multiplexed over regular I2C???)
What were you smoking rockchip??? I really do not want to write another i2c driver!

# VOP2 init
- Setup your video signal.
- *Note that you cannot read/write some of the HDMI TX registers at this point.*
- Config security registers
  - Magic registers
  - sgrf_vop_sec_en[1] = 0b1 // (Secure mode)
  - sgrf_vop_port[1:0] = 0b00 // Setup port
  - sgrf_vop_sec_ctrl[10:0] // ???
  - DDR config??
- Power up DSC encoder (THis can be skipped)
- Set interface to HDMI 0/eDP/etc in `V01_GRF`
- Setup esmart/clusters if needed
- Setup Pixel clock source, data source port
- Program video timing (?)
- Set output timing scan delay number
- Set active splice number
- Setup PPS (Picture Parameter Set)
  - Program PPS register (128 bytes, ?)
  - Enable PPS update
  - Poll for PPS update ack
- Set DONE_CFG
- Video signal is active, all of the HDMI TX registers can be accessed now.
- https://github.com/yanyitech/coolpi-kernel/blob/b7a84509cc732e4590d988dfcd008842b09b17b0/drivers/gpu/drm/rockchip/rockchip_drm_vop2.c
- https://github.com/yanyitech/coolpi-kernel/blob/b7a84509cc732e4590d988dfcd008842b09b17b0/drivers/gpu/drm/rockchip/rockchip_vop2_reg.c

## 2. PHY Setup
- (`HdptxRopllCmnConfig`)
- Power everything on
- Configure clocks
  - ro_ref_clk_sel and lc_ref_clk_sel?
- Check clock is valid
- Reset PHY
- Release APB reset of PHY
- Write internal registers
  - Configure AUX
  - Reset and enable the lanes
  - Configure PHY main link
    - Clock
    - transmitter settings (?)
    - PLL settings
  - TRM supposedly says what registers to write for all of this
- Enable PLL
- Wait until PLL is locked
- Done
- Clock setup: https://github.com/yanyitech/coolpi-kernel/blob/b7a84509cc732e4590d988dfcd008842b09b17b0/drivers/phy/rockchip/phy-rockchip-samsung-hdptx-hdmi.c
- Resets: https://github.com/torvalds/linux/blob/c964ced7726294d40913f2127c3f185a92cb4a41/drivers/clk/rockchip/rst-rk3588.c#L703

## 3. HDMI TX Setup
- Reset TX controller
- Setup I2C controller to communicate with [DDC](https://en.wikipedia.org/wiki/Display_Data_Channel#DDC2) SCDC and get the EDID
- Rockchip people were smoking something potent when they made this.
- Configure DW controller
  - Configure the link
  - Configure packet property registers
  - Configure the HDCP 1.4 encoder
    - Configure the decryption seed
    - Configure HDCP memory
    - **This can (and should tbh) be skipped**
  - Configure audio settings
    - **This can be skipped.**
  - Configure video settings
    - TODO: More here
  - Configure SCDC (Status and Control Data Channel) Interface
    - Configure the SCDC module over I2C
    - Scrambling
    - TMDS Clock ratio
    - RR enable
    - FRL Rate
    - FFE levels
  - Set video input (VOP2) and audio input
  - Link training
    - *We'll try to skip this for now.*
  - https://github.com/yanyitech/coolpi-kernel/blob/b7a84509cc732e4590d988dfcd008842b09b17b0/drivers/gpu/drm/bridge/synopsys/dw-hdmi-qp.c
  - https://github.com/yanyitech/coolpi-kernel/blob/b7a84509cc732e4590d988dfcd008842b09b17b0/drivers/gpu/drm/rockchip/dw_hdmi-rockchip.c
