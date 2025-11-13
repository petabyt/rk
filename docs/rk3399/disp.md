# VOP
- 6 layers supported
- Background color
- 4 Window Layers
- Hardware Cursor Layer
- Hardware alpha blending

## VOP Setup process
- Select reference clock
- Set VOP to L or B (`grf.edp_lcdc_sel`)
- Setup and lock PLL
- Set to standby mode
- Enable desired interface (such as eDP) and disable unneeded interfaces
- Configure display settings
- Configure window layers settings
  - Width, height
  - Porch offsets
  - Enable the window(s)
  - Set format (eg, ARGB888)
  - Set framebuffer address (win0_yrgb_mst)
- Set reg_cfg_done to `0b1`.
- The framebuffer is fetched over AXI from DDR.

## eDP TX Setup process
- Setup reference clock
- Wait for HDP
- Set SW_FUNC_EN_N to normal operation
- dp_pd = 0
- Reset PLL
- Wait until PLL is locked
- Setup AUX channel to get DPCD (optional)
  - Enable AUX AUX_FUNC_EN_N
  - Clear AUX interrupts
  - Disable AUX, set aux defer setting, enable AUX
  - Set AUX channel module to normal func_en_2
  - Enable all function and video modes (func_en_1)
  - Perform a AUX DPCD transfer to get display info (`aux_` registers)
    - Get the display resolution
    - Get the lane count and link rate
    - Set `link_bw_set` and `lane_count_set`
- Link training
  - Link rates, clock recovery, etc
  - This can be skipped
- Make sure eDP clock is on
- Disable video mute
- Enable video output
