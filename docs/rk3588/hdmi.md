# RK3588 display init sequence

# iomux/grf settings
A few registers must be setup for the HDMI TX:
```
VO1_GRF_CON3 (for TX0)
VO1_GRF_CON6 (for TX1)
GRF_SOC_CON7
VO1_GRF_CON9
```

The iomux settings for `SCL` and `SDA` pins for HDMI must be routed to I2C.
As documented on [wikipedia](https://en.wikipedia.org/wiki/HDMI) pins 15 and 16 in the HDMI type A connector are
for I2C. This is used primarily to transport the [EDID](https://en.wikipedia.org/wiki/Extended_Display_Identification_Data) from the display,
which has important information like supported resolutions, timing info, refresh rate, etc.

(Looking at the TRM, it seems impossible for these pins to be multiplexed over regular I2C???)
What were you smoking rockchip???

# VOP2 init
- *Note that you cannot read/write some of the HDMI TX registers at this point.*
- Set connection to HDMI 0/eDP/etc in `V01_GRF`
- Power on (`power_ctrl`)
- Turn overlay interface on
  - Config `layer_sel`, `port_sel`, `dp2_bg_mix_ctrl`
- Setup interface
  - Set regdone_imd_en in regdone_imd_en
  - Setup `dsp_inface_pol`
  - Setup according to the display device chosen (`dsp_inface_en`, `dsp_inface_ctrl`)
- Enable display standby
  - Disable all layers and frame transfer while setup is being done
- Setup the display (POST)
  - Set all clock/timing settings as needed
- Set `dual_display_ctrl`, `dsp_bg`, `scl_ctrl` as needed
- Setup layer
  - Settings depend on what layer system you decide to use
- Setup Pixel clock source, data source port
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
