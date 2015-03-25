#include <stdint.h>
#include <stdlib.h>
#include "wsa_sweep_device.h"


/**
 * creates a new sweep device object and returns it
 *
 * @param device - a pointer to the wsa we've connected to
 * @return - a pointer to the allocated struct, or NULL on failure
 */
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


/**
 * destroys a sweep device.  This does not free the device param that was passed in initially.  only the sweep device
 *
 * @sweepdev - the object to destroy
 */
void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev)
{
	// free the memory of the sweep device, (but not the real device, it came from the parent, so it's their problem)
	free(sweepdev);
}


/**
 * allocates memory to do power spectrum domain captures on the bandwidths indicated
 *
 * @param sweep_device - the sweep device
 * @param fstart - the start of the band to sweep
 * @param fstop - the end of the band to sweep
 * @param rbw - the minimum resolution bandwidth desired
 * @param mode - which mode to perform the sweep in
 * @param pscfg - a pointer to an unallocated power spectrum config struct
 * @returns - negative on error, otherwise the number of bytes allocated
 */
int wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	uint64_t fstart,
	uint64_t fstop,
	uint32_t rbw,
	char const *mode,
	struct wsa_power_spectrum_config **pscfg
)
{
	struct wsa_power_spectrum_config *ptr;

	// alloc some memory for it
	ptr = malloc(sizeof(struct wsa_power_spectrum_config));
	if (ptr == NULL)
		return -1;
	*pscfg = ptr;

	// copy the sweep settings into the cfg object
	ptr->fstart = fstart;
	ptr->fstop = fstop;
	ptr->rbw = rbw;
	ptr->buflen = (fstop - fstart) / rbw;
	ptr->buf = malloc(sizeof(float) * ptr->buflen);
	printf("-> buflen = %d\n", ptr->buflen);
	printf("-> buf @ 0x%08x\n", (unsigned int) ptr->buf);
	if (ptr->buf == NULL) {
		free(ptr);
		return -1;
	}

	return 0;
}


/**
 * destroys a power spectrum config object
 *
 * @param cfg - the config oject to destroy
 */
void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg)
{
	// free the buffer and then the struct
	free(cfg->buf);
	free(cfg);
}


/**
 * captures some power spectrum using the configuration supplied
 *
 * @param sweep_device - the sweep device to use
 * @param cfg - the power spectrum config to use
 * @param buf - if buf is not NULL, a pointer to the allocated memory is stored there for convience
 * @return - 0 on success, negative on error
 */
int wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	struct wsa_power_spectrum_config *cfg,
	float **buf
)
{
	// assign their convienence pointer
	if (*buf)
		*buf == cfg->buf;

	// do a single capture

	// fft that

	// apply reflevel


	return 0;
}
