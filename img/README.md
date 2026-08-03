# Rockchip binary provenance

Files in this directory are redistributed under Rockchip's binary license in
[`LICENSE`](LICENSE). The RK356x files below were copied from
[`rockchip-linux/rkbin`](https://github.com/rockchip-linux/rkbin) commit
`ecb4fcbe954edf38b3ae037d5de6d9f5bccf81f4`; a separate rkbin checkout is not
required to build this repository.

| Vendored file | Original rkbin path | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `rk3566_ddr_1056MHz_v1.25.bin` | `bin/rk35/rk3566_ddr_1056MHz_v1.25.bin` | 59,392 | `c2a1b37673bf03ed338bc39efbe942136459cb3621dad09351144d744d78db26` |
| `rk3568_ddr_1560MHz_v1.25.bin` | `bin/rk35/rk3568_ddr_1560MHz_v1.25.bin` | 59,392 | `ab1d9b822a256b6ef4b3aa54b911c4d1e0faaebc882403c7a6b3efc3e69e07fc` |
| `rk356x_usbplug_v1.17.bin` | `bin/rk35/rk356x_usbplug_v1.17.bin` | 98,708 | `4038b7857b840f539760decc0daf1601b8ff61cc17798101e93b11128a7f333e` |

The DDR blobs are used by `makeboot.out` and the direct `rock.out` DDR-loading
flow. The shared USB-plug blob is used only by the optional `maskrom3566` and
`maskrom3568` xrock helper targets.
