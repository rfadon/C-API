#ifndef __WSA_SWEEP_DEVICE_H__
#define __WSA_SWEEP_DEVICE_H__

#include "wsa_lib.h"
#include "wsa_api.h"

/// this struct represents our sweep device object
struct wsa_sweep_device {
	/// a reference to the wsa we're connected to
	struct wsa_device *real_device;
};

/// struct representing a configuration that we are going to sweep with and capture power spectrum data
struct wsa_power_spectrum_config {
	/// the start frequency
	uint64_t fstart;

	/// the stop frequency
	uint64_t fstop;

	/// the minimum desired rbw
	uint32_t rbw;

	/// the actual rbw 
	uint32_t rbw_actual;

	/// the float buffer 
	float *buf;

	/// the length of the float buffer
	uint32_t buflen;
};


struct wsa_sweep_device *wsa_sweep_device_new(struct wsa_device *device);
void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev);
int wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	uint64_t fstart,
	uint64_t fstop,
	uint32_t rbw,
	char const *mode,
	struct wsa_power_spectrum_config **pscfg
);
void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg);
int wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	struct wsa_power_spectrum_config *pscfg,
	float **buf
);
#endif
