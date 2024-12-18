# RK3588 Clocks

## Example: Setting up v0pll
- Force PLL into slow mode
Connects the PLL to a slow clock to ensure stability while we set it up.
`CRU_MODE_CON00.clk_v0pll_mode = clk_deepslow` (or xin_osc0_func)
- Power down the PLL
`CRU_V0PLL_CON1.v0pll_resetb = 1`
- *See calulating settings*
- Set pre-divider value: `CRU_V0PLL_CON1.v0pll_p = P`
- set main divider value: `CRU_V0PLL_CON0.v0pll_m = M`
- Set scalar value: `CRU_V0PLL_CON1.v0pll_s = S`
- Optional: Set DSM: `CRU_V0PLL_CON2.v0pll_k = K`
- Power up: `CRU_V0PLL_CON1.v0pll_resetb = 0`
- Wait until PLL is locked: `CRU_V0PLL_CON6.v0pll_lock == 1`

## Calculating settings
There are 4 settings that need to be set:
### P
Pre divider value.
### M
Main divider value.
### S
Scalar value
### K
Delta-sigma modulation value. For eliminating noise.
TODO: Document this more.

The Linux kernel has a pre-calculated table for getting these values:
```
/* _mhz, _p, _m, _s, _k */
RK3588_PLL_RATE(1500000000, 2, 250, 1, 0),
RK3588_PLL_RATE(1200000000, 2, 200, 1, 0),
RK3588_PLL_RATE(1188000000, 2, 198, 1, 0),
RK3588_PLL_RATE(1100000000, 3, 550, 2, 0),
RK3588_PLL_RATE(1008000000, 2, 336, 2, 0),
RK3588_PLL_RATE(1000000000, 3, 500, 2, 0),
RK3588_PLL_RATE(900000000, 2, 300, 2, 0),
RK3588_PLL_RATE(850000000, 3, 425, 2, 0),
RK3588_PLL_RATE(816000000, 2, 272, 2, 0),
RK3588_PLL_RATE(786432000, 2, 262, 2, 9437),
RK3588_PLL_RATE(786000000, 1, 131, 2, 0),
RK3588_PLL_RATE(742500000, 4, 495, 2, 0),
RK3588_PLL_RATE(722534400, 8, 963, 2, 24850),
RK3588_PLL_RATE(600000000, 2, 200, 2, 0),
RK3588_PLL_RATE(594000000, 2, 198, 2, 0),
RK3588_PLL_RATE(200000000, 3, 400, 4, 0),
RK3588_PLL_RATE(100000000, 3, 400, 5, 0),
```

And some code to calculate the idela values from an input and output frequency.
```
static struct rockchip_pll_rate_table *
rockchip_rk3588_pll_frac_by_auto(unsigned long fin_hz, unsigned long fout_hz)
{
	struct rockchip_pll_rate_table *rate_table = &rockchip_auto_table;
	u32 p, m, s, k;
	u64 fvco;

	for (s = 0; s <= 6; s++) {
		fvco = (u64)fout_hz << s;
		if (fvco < RK3588_VCO_MIN_HZ || fvco > RK3588_VCO_MAX_HZ)
			continue;
		for (p = 1; p <= 4; p++) {
			for (m = 64; m <= 1023; m++) {
				if ((fvco >= m * fin_hz / p) &&
				    (fvco < (m + 1) * fin_hz / p)) {
					//k = rockchip_rk3588_pll_k_get(m, p, s,
					//			      fin_hz,
					//			      fvco);
					//if (!k)
					//	continue;
					rate_table->p = p;
					rate_table->s = s;
					rate_table->k = 0;
					if (k > 32767)
						rate_table->m = m + 1;
					else
						rate_table->m = m;
					return rate_table;
				}
			}
		}
	}
	return NULL;
}
```
