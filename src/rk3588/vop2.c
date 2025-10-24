#include "rk3588.h"

struct LayerConfig {
	uint32_t width;
	uint32_t height;
	uint64_t framebuffer;
};

static struct Vop2Overlay *get_overlay(uintptr_t base, int i) {
	return (struct Vop2Overlay *)(base + 0x600);
}

static struct Vop2ESmart *get_esmart(uintptr_t base, int i) {
	return (struct Vop2ESmart *)((base + 0x1800) + (i * 0x200));
}

static struct Vop2Post *get_post(uintptr_t base, int i) {
	return (struct Vop2Post *)((base + 0xc00) + (i * 0x100));
}

void set_layer_port(uintptr_t base, int layer, int port) {
	
}

void config_smart_layer(uintptr_t base, struct LayerConfig cfg) {
	// TODO:	
}

void config_post(uintptr_t base) {
	
}

void config_hdmi(uintptr_t base) {
	
}

void config_edp0(uintptr_t base) {
	
}

void config_edp1(uintptr_t base) {
	
}

void vop2_init(uintptr_t base) {
	
}

void setup_display(uintptr_t base) {
	
}
