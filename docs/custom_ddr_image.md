# Custom DDR images

A custom DDR image for a rockchip platform can be made with an assembly stub like this:
```
.global _start
_start:

b shim
bin: .incbin "../../img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin", 4

shim:

stp x0, x1, [sp, #-0x10]!
stp x2, x3, [sp, #-0x10]!
stp x4, x5, [sp, #-0x10]!
stp x6, x7, [sp, #-0x10]!
stp x8, x9, [sp, #-0x10]!
stp x10, x11, [sp, #-0x10]!
stp x12, x13, [sp, #-0x10]!
stp x14, x15, [sp, #-0x10]!
stp x16, x17, [sp, #-0x10]!
stp x18, x30, [sp, #-0x10]!
bl ddr_shim
ldp x18, x30, [sp], #0x10
ldp x16, x17, [sp], #0x10
ldp x14, x15, [sp], #0x10
ldp x12, x13, [sp], #0x10
ldp x10, x11, [sp], #0x10
ldp x8, x9, [sp], #0x10
ldp x6, x7, [sp], #0x10
ldp x4, x5, [sp], #0x10
ldp x2, x3, [sp], #0x10
ldp x0, x1, [sp], #0x10

b bin
```

`ddr_shim` can perform early power-up tasks, such as turning on power domains, GPIO pins, etc. Allowing devices to power on while DDR is initialized
makes it boot slightly faster.
