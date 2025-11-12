# Conclusion on Rockchip

## Documentation
- In both the RK3399 and RK3588, exactly **67%** of subsystems[^1] on the chip are documented[^2]
	- RK3399 TRM only covers 33 out of 49 subsystems
	- RK3588 TRM only covers 89 out of 132 subsystems
- Rockchip's SGRF[^3] is the most important register file that is undocumented. Configuring it is necessary in order to enable shared memory, which is needed
in order to get a framebuffer working.
- RK3399 eMMC documentation is missing `CMD_INDEX` field from the EMMCCORE_CMD register (which is the most important field in the whole IP)
- Samsung HDMI/eDP Combo PHY docs are missing a lot of registers from the register map[^4]

## Design
- RK3588 GRF, CRU, and PMU is split into a dozen different register files and not organized in any way or set up for strided access
	- Offsets and bitfields have to be hardcoded in a map in order to setup PLLs and change iomux settings
- Why does VOP2 need 2 different layer systems? What does 'ESMART' stand for?
- It's impossible to configure iomux for a rockchip i2c controller to be used for HDMI. Instead, a completely separate DesignWare I2C driver has to be written specifically for HDMI
- HDMI 2.1 DesignWare IP is used - supports a lot of features and is difficult to get working
- RK3588 has a ton of variants and derivatives (RK3588S/RK3588J/RK3576...) and supporting them all is difficult

## Ecosystem
- Joshua Reik burned out from working on ubuntu-rockchip for various reasons, but mainly because he couldn't get in touch with Rockchip for assistance[^5], after working on Rockchip boards for free
- [edk2-rk3588](https://github.com/edk2-porting/edk2-rk3588) maintainer (Mario) moved on from the project because:
	- Rockchip refuses to adopt standards properly or upstream hardware support
	- Board vendors refuse to offer assistance to the community working on their hardware for free
	- **Most board vendors do the bare minimum for people to buy their product and leave their customers in the dark after that**
- *"I was told Rockchip no longer considers RK3588 to be an “open source” chip. In practice, that means Rockchip no longer contributes RK3588 code to open-source projects and they also decided not to sell RK3588 directly to SBC vendors."*[^8]
	- This checks out - RK3588 is nowhere to be found on https://opensource.rock-chips.com/
	- Rockchip is insane if they ever considered RK3588 'open source' in the first place
- Rockchip and their vendors (Orange Pi, Banana Pi, Radxa, Cool-Pi, etc) release boards and products with subpar hardware support
	- Some vendors require custom kernels and bootloaders
	- Some vendors use nonstandard hardware that requires additional kernel work to get working
- Rockchip and Collabora employees are *still* working on upstreaming support for rockchip[^6]
- Rockchip is shifting focus to Edge AI, Automotive, Robotics, and smart audio solutions. [^7]

[^1]: I count each separate register file as a 'subsystem', excluding duplicates
[^2]: 'documented' only means it has the register file defined. It may not have all the information needed to write a driver.
[^3]: Secure General Register File. There are multiple of these register files on the RK3588.
[^4]: check cmn regs such as offset `0x0148`, `0x0158`,`0x01b4`,`0x01b8`...
[^5]: https://www.jeffgeerling.com/blog/2024/popular-rockchip-sbc-distro-limbo-after-maintainer-burns-out, https://web.archive.org/web/20241112102432/https://github.com/Joshua-Riek/ubuntu-rockchip/discussions/1104
[^6]: https://lore.kernel.org/all/?q=rock-chips.com, https://www.collabora.com/news-and-blog/news-and-events/rockchip-rk3588-upstream-support-progress-future-plans.html
[^7]: https://www.myirtech.com/news_list.asp?id=930
[^8]: https://x.com/geerlingguy/status/1870959892242342030
