# RK3588 Display Docs

## Capabilities
### RK3588S
- 1x Samsung HDMI/eDP Combo Interface
- Supports DP-Alt (USBDP Combo PHY)
- VOP2
- 1x Designware HDMI TX
### RK3588
- Everything that RK3588S has
- +1 HDMI/eDP Combo PHY
- +1 Designware HDMI TX

## Samsung HDMI TX/EDP Combo PHY (HDPTX)
- HDMI 2.1
- 12Gbps
- Manages all clock lines
- Serializer
- Lots of low-level circuits Daniel doesn't understand

## Synopsis DesignWare HDMI TX Controller
- HDMI 2.1
- Up to 4K120Hz and 8K60Hz
- SCDC (Status and Control Data Channel Module) over I2C
- E-EDID over I2C
- HDCP over I2C
- Built-in I2C master
- Lots of modern HDMI stuff I don't understand

## EDP TX Controller
- Not much different than the RK3399 EDP TX

## VOP2
- 7680x4320@30Hz through DP
- 7680x4320@60Hz through HDMI PHY
- 4096x2304@60Hz through eDP
- MIPI DSI2
- 4 video ports (not all equal)
- Cluster (4)
  - 4096x4320
  - AFBC compression
  - Image scaling
- Esmart (4)
  - 4096x4320
  - Smart layer is not recommended
- AXI 0 Write Back
  - ARGB888
  - Max 1920x1080
  - AXI1 can also be used
- LUT
- Dolby Vision
