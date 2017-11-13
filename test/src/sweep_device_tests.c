
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

// uses an R5500 device to test different sweep device settings
int16_t sweep_device_tests(struct wsa_device *dev, struct test_data *test_info){

	FILE *output_file;
	int16_t result;
	int16_t acq_status = 0;
	float max = -FLT_MAX;
	float location_ratio; 
	float peak_freq = 0.0f;
	float freq = 0.0f;
	int32_t capt_counter = 0;
	uint64_t fstart =  26500000000;
	uint64_t fstop =  27000000000;
	uint64_t span = fstop - fstart;
	uint32_t rbw = 10000;
	uint64_t peak_freq_u64 = 0;

	// For elapsed time measurements.
	clock_t start, end;
	//time_t start, end;
	double diff;

#if defined(DEBUG_DROPOUT)
	// Looking for data dropouts 2017-11-09
	int32_t runlength;
	int32_t max_runlength;
	float const dropout_threshold = -130.0f;
	float dropout_freq = 0.0f;
#endif

	uint32_t max_index = 0;
	struct wsa_sweep_device wsa_sweep_device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_sweep_device;

	// initialize attenuation
	unsigned int attenuation = 30;

	// initialize config buffer
	struct wsa_power_spectrum_config *pscfg;
	
	// initialize power spectrum buffer
	float *psbuf;
	uint32_t i = 0;

	// reset R5500 state
	result = wsa_system_abort_capture(dev);
	wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_status);

	// create the sweep device
	wsa_sweep_dev = wsa_sweep_device_new(dev);


	// TEST CASE 1: Test for capture: Sart 20 MHz, stop 40 MHz, RBW 100kHz
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 20000000, (uint64_t) 40000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 0);

	// TEST CASE 2: Test for capture: Sart 145 MHz, stop 40 MHz, RBW 100kHz, this test should fail
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 145000000, (uint64_t) 40000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 1);

	// TEST CASE 3: Test for capture: Sart 145 MHz, stop 42.22 GHz, RBW 100kHz, this test should fail
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 145000000, (uint64_t) 42220000000, (uint32_t)100000, "SH", &pscfg);
	verify_result(test_info, result, 1);


	// TEST CASE 4: Test for capture: 50 MHz to 6 GHz at 20 kHz RBW, this test should pass


	if (DEBUG_FILE_OUT & g_debug_mask) {
		output_file = fopen("TestOutput.csv", "w");
	}

#if defined(DEBUG_DROPOUT)
	output_file = fopen("DropoutCheck.txt", "w");
#endif

	fstart = 9000;
	fstop = 8 * GHZ;
	rbw = 20000UL;
	span = fstop - fstart;
	capt_counter = 0;
	while(capt_counter < 5) {
		DEBUG_PRINTF(DEBUG_INFO, "----- Test %lu -----", capt_counter);

#if defined(DEBUG_DROPOUT)
		// Looking for dropouts.
		runlength = 0;
		max_runlength = 0;
#endif

		capt_counter++;
		wsa_power_spectrum_free(pscfg);
		result = wsa_power_spectrum_alloc(wsa_sweep_dev, fstart,  fstop, rbw, "SH", &pscfg);
		verify_result(test_info, result, 0);
		result = wsa_configure_sweep(wsa_sweep_dev, pscfg);
		verify_result(test_info, result, 0);
		start = clock();
		result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &psbuf);
		end = clock();
		diff = (double)(fstop - fstart) / 1000000.0F / ((double)(end - start) / CLOCKS_PER_SEC);		// Sweep speed in MHz / second
		DEBUG_PRINTF(DEBUG_SPEED, "Sweep speed = %.2f MHz/s", diff);
		verify_result(test_info, result, 0);
		// print the spectral data
		for(i = 0; i < pscfg->buflen; i++) {

			location_ratio = ((float)i + 0.5f) / ((float)pscfg->buflen);
			freq = (float)fstart + ((float)fstop - (float)fstart) * location_ratio;
			DEBUG_PRINTF(DEBUG_SPEC_DATA, ", %lu, %0.2f, %0.2f", i, freq, pscfg->buf[i]);

#if defined(DEBUG_DROPOUT)
			// Looking for dropouts
			if (pscfg->buf[i] < dropout_threshold) {
				// printf("%lu, %0.2f, %0.2f\n", i, freq, pscfg->buf[i]);
				runlength++;
				if (runlength > max_runlength) {
					max_runlength = runlength;
					dropout_freq = freq;
				}
				else {
					runlength = 0;
				}
			}
#endif

			if (DEBUG_FILE_OUT & g_debug_mask) {
				fprintf(output_file, "%lu, %0.2f, %0.2f\n", i, freq, pscfg->buf[i]);
			}

			if (pscfg->buf[i] > max){
				
				max_index = i;
				max = pscfg->buf[i];
				//printf("MAX\n");

			}
			else {
				//printf("\n");
			}
		}

#if defined(DEBUG_DROPOUT)
		// Print out the max length of any dropout
		printf("Max dropout length below %f was %lu ending at freq %f\n", dropout_threshold, max_runlength, dropout_freq);
		fprintf(output_file, "Max dropout length below %f was %lu ending at freq %f\n", dropout_threshold, max_runlength, dropout_freq);
#endif

		// Print peak location as a frequency.
		location_ratio = ( (float)max_index + 0.5f ) / ( (float)pscfg->buflen );		// Peak location as a fraction of the sweep band (rounded).
		peak_freq = (float)fstart + ((float)fstop - (float)fstart) * location_ratio;		// Convert that to frequency.

		DEBUG_PRINTF(DEBUG_PEAKS, "Peak value: %0.2f  Peak freq: %0.2f\n", max, peak_freq);
		// printf("%d %d, %d, %0.2f, %0.2f \n",capt_counter, max_index, i, (((float) fstart) + (span * location_ratio)) / 1000000, max);
		max = -FLT_MAX;

		psd_peak_find(fstart, fstop, rbw, pscfg->buflen, pscfg->buf, &peak_freq_u64, &max);
		DEBUG_PRINTF(DEBUG_PEAKS, "psd_peak_find() Peak power %0.2f  Peak freq: %llu\n", max, peak_freq_u64);

	}
	if (DEBUG_FILE_OUT & g_debug_mask) {
		fclose(output_file);
	}
	fclose(output_file);
    return 0;
}

