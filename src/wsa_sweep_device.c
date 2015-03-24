#include "wsa_sweep_device.h"


struct wsa_sweep_device *wsa_sweep_device_new(struct wsa_device *device)
{
	struct wsa_sweep_device *sweepdev;

	// alloc memory for our object
	sweepdev = malloc(sizeof(struct wsa_sweep_device));
	if (sweepdev == NULL)
		return NULL;

	// initialize everything in the struct
	sweepdev->real_device = device;

	return sweepdev;
}


void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev)
{
	// free the memory of the sweep device, (but not the real device, it came from the parent, so it's their problem)
	free(sweepdev);
}


int wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	int64_t const fstart,
	int64_t const fstop,
	int64_t const rbw,
	char const *mode,
//	struct wsa_settings const device_settings,
	struct wsa_power_spectrum_config *cfg,
	float *buf
)
{


	return 0;
}


void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg, float *buf) { }


int wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	struct wsa_power_spectrum_config *cfg,
	float *spectral_data
)
{


	return 0;
}
