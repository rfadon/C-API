#ifndef __WSA_SWEEP_DEVICE_H__
#define __WSA_SWEEP_DEVICE_H__

#include "wsa_lib.h"
#include "wsa_api.h"

/// a struct for holding all the info about captured data being received

/// a struct for holding sweep device properties
struct wsa_sweep_device_properties {
	uint32_t mode;
	uint32_t sample_type;
	uint8_t fshift_available;
	uint64_t min_tunable;
	uint64_t max_tunable;
	uint32_t tuning_resolution;
	uint32_t full_bw;
	uint32_t usable_bw;
	uint32_t passband_center;
	uint32_t usable_left;
	uint32_t usable_right;
	uint32_t min_decimation;
	uint32_t max_decimation;
};

/// a sweep plan entry struct
struct wsa_sweep_plan {
	/// next sweep plan entry
	struct wsa_sweep_plan *next_entry;

	/// sweep start
	uint64_t fcstart;

	/// sweep stop
	uint64_t fcstop;

	/// step size
	uint32_t fstep;

	/// samples per packet
	uint32_t spp;
	
	/// packets per block
	uint32_t ppb;

	/// indicate whether DD mode is required
	uint8_t dd_mode;

};

/// this struct represents our sweep device object
struct wsa_sweep_device {
	/// a reference to the wsa we're connected to
	struct wsa_device *real_device;

	/// device settings that get sent to a sweep
	struct {
		uint8_t attenuator;
	} device_settings;
};

/// struct representing a configuration that we are going to sweep with and capture power spectrum data
struct wsa_power_spectrum_config {

	// keep track if only a dd packet is available
	uint8_t only_dd;

	// determine if an entry is required at the end to compensate for last frequency
	uint8_t compensation_entry;

	// frequency of the very last entry
	uint64_t compensation_freq;

	/// the mode to perform the sweep in
	uint32_t mode;

	/// the start frequency
	uint64_t fstart;

	/// the stop frequency
	uint64_t fstop;

	/// the rbw
	uint64_t rbw;
			
	/// a sweep plan that achieves capturing the spectrum requested
	struct wsa_sweep_plan *sweep_plan;

	/// how many packets will this sweep plan generate
	uint32_t packet_total;

	/// The number of packets within each frequency steps
	uint32_t packets_per_block;

	
	/// The number of samples per packet
	uint32_t samples_per_packet;

	/// the float buffer 
	float *buf;

	// determine if the reference level needs to be modified
	uint8_t modify_ref;

	/// the length of the float buffer
	uint32_t buflen;


};


struct wsa_sweep_device *wsa_sweep_device_new(struct wsa_device *device);
void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev);
void wsa_sweep_device_set_attenuator(struct wsa_sweep_device *sweep_device, unsigned int val);
int16_t wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	uint64_t fstart,
	uint64_t fstop,
	uint32_t rbw,
	char const *mode,
	struct wsa_power_spectrum_config **pscfg
);
void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg);
int16_t wsa_configure_sweep(struct wsa_sweep_device *sweep_device, struct wsa_power_spectrum_config *pscfg);
int16_t wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	struct wsa_power_spectrum_config *pscfg,
	float **buf
);
#endif
