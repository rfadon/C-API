#ifndef __WSA_SWEEP_DEVICE_H__
#define __WSA_SWEEP_DEVICE_H__

#include "wsa_lib.h"
#include "wsa_api.h"

struct wsa_sweep_device {
	struct wsa_device *real_device;
};

struct wsa_sweep_device *wsa_sweep_device_new(struct wsa_device *device);
void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev);
int wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	int64_t const fstart,
	int64_t const fstop,
	int64_t const rbw,
	char const *mode,
//	struct wsa_settings const device_settings,
	float *spectral_data
);
#endif
