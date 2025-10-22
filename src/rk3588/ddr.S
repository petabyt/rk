.global _start
_start:

b shim
bin: .incbin "../../img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin", 4

shim:

// do shim stuff here

b bin
