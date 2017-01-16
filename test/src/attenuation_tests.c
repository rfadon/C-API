
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>

int16_t attenuation_tests(struct wsa_device *dev, int32_t *fail_count, int32_t *pass_count){

	int16_t result;
	int32_t attenuator;
	
	// test for attenuation 0
	result = wsa_set_attenuation(dev, 0);
	if (result < 0)
		*fail_count = *fail_count + 1;
		
	else{
		result = wsa_get_attenuation(dev, &attenuator);
		if (attenuator != 0)
			*fail_count = *fail_count + 1;
		else
			*pass_count = *pass_count + 1;
	}
	
	// test for attenuation 10
	result = wsa_set_attenuation(dev, 10);
	if (result < 0)
		*fail_count = *fail_count + 1;
	else{
		result = wsa_get_attenuation(dev, &attenuator);
		if (attenuator != 10)
			*fail_count = *fail_count + 1;
		else
			*pass_count = *pass_count + 1;
	}
	
	// test for attenuation 20
	result = wsa_set_attenuation(dev, 20);
	if (result < 0)
		*fail_count = *fail_count + 1;
	else{
		result = wsa_get_attenuation(dev, &attenuator);
		if (attenuator != 20)
			*fail_count = *fail_count + 1;
		else
			*pass_count = *pass_count + 1;
	}
	
	// test for attenuation 30
	result = wsa_set_attenuation(dev, 30);
	if (result < 0)
		*fail_count = *fail_count + 1;
	else{
		result = wsa_get_attenuation(dev, &attenuator);
		if (attenuator != 30)
			*fail_count = *fail_count + 1;
		else
			*pass_count = *pass_count + 1;
	}
	return 0;
}
