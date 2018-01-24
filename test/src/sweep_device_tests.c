
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_dsp.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>
#include <test_util.h>
#include <float.h>
#include <time.h>					// For sweep time measurements.

#include "debug_printf.h"

#if defined (MHZ)
#undef MHZ
#endif
#define MHZ	(1000000ULL)
#define GHZ (1000000000ULL)


// Define the following to test using repeated runs of two sweeps back-to-back --
// one wide and one more narrow.
#define TWO_SWEEPS  1

#if defined(TWO_SWEEPS)
// uses an R5500 device to test different sweep device settings
int16_t sweep_device_tests(struct wsa_device *dev, struct test_data *test_info) {

	FILE *output_file;
	int16_t result;
	int16_t acq_status = 0;
	float max = -FLT_MAX;
	float location_ratio;
	float peak_freq = 0.0f;
	float freq = 0.0f;
	int32_t capt_counter = 0;
	uint64_t peak_freq_u64 = 0;
	uint64_t fstart1 = 26500000000;
	uint64_t fstop1 = 27000000000;
	uint32_t rbw1 = 10000;
	uint64_t fstart2 = 26500000000;
	uint64_t fstop2 = 27000000000;
	uint32_t rbw2 = 10000;

	// For elapsed time measurements.
	clock_t start, end;
	double diff;

	uint32_t max_index = 0;
	struct wsa_sweep_device wsa_sweep_device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_sweep_device;

	// initialize attenuation
	unsigned int attenuation = 30;

	// initialize config buffers
	struct wsa_power_spectrum_config *pscfg1;
	struct wsa_power_spectrum_config *pscfg2;

	// initialize power spectrum buffers
	float *psbuf1;
	float *psbuf2;

	uint32_t i = 0;
	uint32_t j = 0;
	int isweep;

	init_test_data(test_info);

	// reset R5500 state
	result = wsa_system_abort_capture(dev);
	wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_status);

	// create the sweep device
	wsa_sweep_dev = wsa_sweep_device_new(dev);
	wsa_sweep_device_set_attenuator(wsa_sweep_dev, 0);		// 0 dB as recommended in R5500 programmer's guide

	if (DEBUG_FILE_OUT & g_debug_mask) {
		output_file = fopen("TestOutput.csv", "w");
	}

	// SWEEP 1 -- R5500-418
	fstart1 = 1000 * MHZ;
	fstop1 = 1900 * MHZ;
	rbw1 = 1 * MHZ;

	// SWEEP 2 -- R5500-418
	fstart2 = 9000;
	fstop2 = 18 * GHZ;
	rbw2 = 20000UL;

	capt_counter = 0;
	while (capt_counter < 1) {

		DEBUG_PRINTF(DEBUG_INFO, "----- Test %lu -----", capt_counter);
		capt_counter++;

		result = wsa_power_spectrum_alloc(wsa_sweep_dev, fstart1, fstop1, rbw1, "SH", &pscfg1);
		verify_result(test_info, result, 0);

		result = wsa_power_spectrum_alloc(wsa_sweep_dev, fstart2, fstop2, rbw2, "SH", &pscfg2);
		verify_result(test_info, result, 0);

		isweep = 1;

		for (j = 0; j < 200000; j++)
		{
			if (isweep == 1) {

				// Do first sweep.
				result = wsa_configure_sweep(wsa_sweep_dev, pscfg1);
				verify_result(test_info, result, 0);

				start = clock();
				result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg1, &psbuf1);
				end = clock();
				diff = (double)(pscfg1->fstop_actual - pscfg1->fstart_actual) / 1000000.0F / ((double)(end - start) / CLOCKS_PER_SEC);		// Sweep speed in MHz / second
				DEBUG_PRINTF(DEBUG_SPEED, "Sweep speed = %.2f MHz/s", diff);
				if (0 != result) {
					printf("\nwsa_capture_power_spectrum() returned %d\n", result);
				}

				verify_result(test_info, result, 0);
				isweep = 2;

				// Print peak.
				max = -FLT_MAX;
				for (i = 0; i < pscfg1->buflen; i++) {
					if (pscfg1->buf[i] == POISONED_BUFFER_VALUE) {
						printf("Sweep %d %d sample %i is poisoned\n", capt_counter, j, i);
					}
					if (pscfg1->buf[i] > max) {
						max_index = i;
						max = pscfg1->buf[i];
					}
				}

				location_ratio = (float)max_index / (float)pscfg1->buflen;		// Peak location as a fraction of the sweep band.
				peak_freq = (float)pscfg1->fstart_actual + ((float)pscfg1->fstop_actual - (float)pscfg1->fstart_actual) * location_ratio;		// Convert that to frequency.
				printf("%8d, %8d, %8d, %10.2f, %16.2f \n", capt_counter, j, max_index, max, peak_freq);
				psd_peak_find(pscfg1->fstart_actual, pscfg1->fstop_actual, rbw1, pscfg1->buflen, pscfg1->buf, &peak_freq_u64, &max);
				DEBUG_PRINTF(DEBUG_PEAKS, "psd_peak_find() Peak power %10.2f  Peak freq: %llu", max, peak_freq_u64);

			}
			else {
				// Do second sweep.
				result = wsa_configure_sweep(wsa_sweep_dev, pscfg2);
				verify_result(test_info, result, 0);

				start = clock();
				result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg2, &psbuf2);
				end = clock();
				diff = (double)(pscfg2->fstop_actual - pscfg2->fstart_actual) / 1000000.0F / ((double)(end - start) / CLOCKS_PER_SEC);		// Sweep speed in MHz / second
				DEBUG_PRINTF(DEBUG_SPEED, "Sweep speed = %.2f MHz/s", diff);
				if (0 != result) {
					printf("\nwsa_capture_power_spectrum() returned %d\n", result);
				}

				verify_result(test_info, result, 0);
				isweep = 1;

				// Print the peak.
				max = -FLT_MAX;
				for (i = 0; i < pscfg2->buflen; i++) {
					if (pscfg2->buf[i] == POISONED_BUFFER_VALUE) {
						printf("Sweep %d %d sample %i is poisoned\n", capt_counter, j, i);
					}
					if (pscfg2->buf[i] > max) {
						max_index = i;
						max = pscfg2->buf[i];
					}
				}

				location_ratio = (float)max_index / (float)pscfg1->buflen;		// Peak location as a fraction of the sweep band.
				peak_freq = (float)pscfg2->fstart_actual + ((float)pscfg2->fstop_actual - (float)pscfg2->fstart_actual) * location_ratio;		// Convert that to frequency.
				printf("%8d, %8d, %8d, %10.2f, %16.2f \n", capt_counter, j, max_index, max, peak_freq);
				psd_peak_find(pscfg2->fstart_actual, pscfg2->fstop_actual, rbw2, pscfg2->buflen, pscfg2->buf, &peak_freq_u64, &max);
				DEBUG_PRINTF(DEBUG_PEAKS, "psd_peak_find() Peak power %10.2f  Peak freq: %llu", max, peak_freq_u64);

			}
		}
		printf("\n");

		wsa_power_spectrum_free(pscfg1);
		wsa_power_spectrum_free(pscfg2);

	}
	if (DEBUG_FILE_OUT & g_debug_mask) {
		fclose(output_file);
	}
	return 0;
}

#else

// uses an R5500 device to test different sweep device settings
int16_t sweep_device_tests(struct wsa_device *dev, struct test_data *test_info) {

	int16_t result;
	int16_t acq_status = 0;
	float max = -FLT_MAX;
	float location_ratio; 
	int32_t capt_counter = 0;
	uint32_t max_index = 0;
	uint32_t rbw;
	uint64_t fstart =  26500000000;
	uint64_t fstop =  27000000000;
	float peak_freq;
	uint64_t peak_freq_u64 = 0;
	struct wsa_sweep_device wsa_sweep_device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_sweep_device;

	// initialize attenuation
	unsigned int attenuation = 30;

	// initialize config buffer
	struct wsa_power_spectrum_config *pscfg;
	
	// initialize power spectrum buffer
	float *psbuf;
	uint32_t i = 0;
    
    init_test_data(test_info);

	// reset R5500 state
	result = wsa_system_abort_capture(dev);
	wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_status);

	// create the sweep device
	wsa_sweep_dev = wsa_sweep_device_new(dev);

	// TEST CASE 1: Test for capture: Start 20 MHz, stop 40 MHz, RBW 100kHz
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 20000000, (uint64_t) 40000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 0);

	// TEST CASE 2: Test for capture: Start 145 MHz, stop 40 MHz, RBW 100kHz, this test should fail
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 145000000, (uint64_t) 40000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 1);

	// TEST CASE 3: Test for capture: Start 145 MHz, stop 42.22 GHz, RBW 100kHz, this test should fail
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 145000000, (uint64_t) 42220000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 1);

	// TEST CASE 4: Test for capture: 9 kHz to 18 GHz at 20 kHz RBW, this test should pass on our target R5500-418.
	fstart = 9000;
	fstop = 18 * GHZ;
	rbw = 20000UL;
	capt_counter = 0;
	while(capt_counter < 40) {
		capt_counter++;
		wsa_power_spectrum_free(pscfg);
		result = wsa_power_spectrum_alloc(wsa_sweep_dev, fstart,  fstop, rbw, "SHN", &pscfg);
		if (0 != result) {
			printf("\nwsa_capture_power_spectrum() returned %d\n", result);
		}
		verify_result(test_info, result, 0);
		result = wsa_configure_sweep(wsa_sweep_dev, pscfg);
		verify_result(test_info, result, 0);
		result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &psbuf);
		verify_result(test_info, result, 0);

		// Find peak location as a frequency.
		max = -FLT_MAX;
		for(i = 0; i < pscfg->buflen; i++) {
			if (pscfg->buf[i] > max) {
				max_index = i;
				max = pscfg->buf[i];
			}
		}

		location_ratio = (float)max_index / (float)pscfg->buflen;																	// Peak location as a fraction of the sweep band.
		peak_freq = (float)pscfg->fstart_actual + ((float)pscfg->fstop_actual - (float)pscfg->fstart_actual) * location_ratio;		// Convert that to frequency.
		printf("%8d, %8d, %10.2f, %16.2f \n", capt_counter, max_index, max, peak_freq);
		psd_peak_find(pscfg->fstart_actual, pscfg->fstop_actual, rbw, pscfg->buflen, pscfg->buf, &peak_freq_u64, &max);
		DEBUG_PRINTF(DEBUG_PEAKS, "psd_peak_find() Peak power: %10.2f  Peak freq: %llu", max, peak_freq_u64);
	}
    return 0;
}

#endif

