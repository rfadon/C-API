
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>
#include <test_util.h>


// uses an R5500 device to test different sweep device settings
int16_t sweep_device_tests(struct wsa_device *dev, struct test_data *test_info){


	int16_t result;
	int16_t acq_status = 0;
	float max = -999;
	float location_ratio; 
	int32_t capt_counter = 0;
	uint64_t fstart =  26500000000;
	uint64_t fstop =  27000000000;
	uint64_t span = fstop - fstart;
	 

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


	// TEST CASE 4: Test for capture: Sart 10 kHz, stop 8 GHz, RBW 100kHz, this test should pass
	fstart = 100000;
	fstop = 1000000;
	span = fstop - fstart;
	while(capt_counter < 20){
		capt_counter++;
		wsa_power_spectrum_free(pscfg);
		result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) fstart,  (uint64_t) fstop, (uint32_t) 100000, "SH", &pscfg);
		verify_result(test_info, result, 0);
		result = wsa_configure_sweep(wsa_sweep_dev, pscfg);
		verify_result(test_info, result, 0);
		result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &psbuf);
		verify_result(test_info, result, 0);
		// print the spectral data
		for(i = 0; i < pscfg->buflen; i++){
			if (pscfg->buf[i] > max){
				
				max_index = i;
				max = pscfg->buf[i];
			

			}
		}

		location_ratio = ((float) max_index) / ((float) pscfg->buflen);

		printf("%d %d, %d, %0.2f, %0.2f \n", capt_counter, max_index, i, (((float) fstart) + (span * location_ratio)) / 1000000, max);
		max = -300;
	}


	// TEST CASE 4: Test for capture: Sart 10 kHz, stop 8 GHz, RBW 100kHz, this test should pass
	fstart = 100000;
	fstop = 1000000000;
	span = fstop - fstart;
	capt_counter = 0;
	while(capt_counter < 10){
		capt_counter++;
		wsa_power_spectrum_free(pscfg);
		result = wsa_power_spectrum_alloc(wsa_sweep_dev, (uint64_t) 300000,  (uint64_t) fstop, (uint32_t) 100000, "SH", &pscfg);
		verify_result(test_info, result, 0);
		result = wsa_configure_sweep(wsa_sweep_dev, pscfg);
		verify_result(test_info, result, 0);
		result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &psbuf);
		verify_result(test_info, result, 0);
		// print the spectral data
		for(i = 0; i < pscfg->buflen; i++){
			if (pscfg->buf[i] > max){
				
				max_index = i;
				max = pscfg->buf[i];
			

			}
		}

		location_ratio = ((float) max_index) / ((float) pscfg->buflen);
		printf("%d %d, %d, %0.2f, %0.2f \n",capt_counter, max_index, i, (((float) fstart) + (span * location_ratio)) / 1000000, max);
		max = -300;
	}
    return 0;
}

