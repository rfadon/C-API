
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>

// uses an R5500 device to test different attenuation settings
// results are stored in the pass/fail count variables
int16_t attenuation_tests(struct wsa_device *dev, struct test_data *test_info){

	int16_t result;
	int32_t attenuator;
	
	// test for attenuation 0
	result = wsa_set_attenuation(dev, 0);
	result = wsa_get_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 0, attenuator);

	// test for attenuation 10
	result = wsa_set_attenuation(dev, 10);
	result = wsa_get_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 10, attenuator);
	
	// test for attenuation 20
	result = wsa_set_attenuation(dev, 20);
	result = wsa_get_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 20, attenuator);
	
	// test for attenuation 30
	result = wsa_set_attenuation(dev, 30);
	result = wsa_get_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 30, attenuator);
	return 0;
}


// uses an R5500 device to test different sweep attenuation settings 
// results are stored in the pass/fail count variables
int16_t sweep_attenuation_tests(struct wsa_device *dev, struct test_data *test_info){

	int16_t result;
	int32_t attenuator;
	
	// test for attenuation 0
	result = wsa_set_sweep_attenuation(dev, (int32_t) 0);
	result = wsa_get_sweep_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 0, attenuator);

	// test for attenuation 10
	result = wsa_set_sweep_attenuation(dev, (int32_t) 10);
	result = wsa_get_sweep_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 10, attenuator);
	
	// test for attenuation 20
	result = wsa_set_sweep_attenuation(dev, (int32_t) 20);
	result = wsa_get_sweep_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 20, attenuator);
	
	// test for attenuation 30
	result = wsa_set_sweep_attenuation(dev, (int32_t) 30);
	result = wsa_get_sweep_attenuation(dev, &attenuator);
	verify_signed32_result(test_info, result, 30, attenuator);
	return 0;
}