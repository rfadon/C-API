#include <stdio.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include "test_util.h"


// Test different attenuation settings
// results are stored in the pass/fail count variables
int16_t attenuation_tests(struct wsa_device *dev, struct test_data *test_info) {

	int16_t result;
	int32_t attenuator;
	int i;
    
    init_test_data(test_info);
    
    if ((strcmp(dev->descr.prod_model, WSA5000) == 0) || (strcmp(dev->descr.prod_model, RTSA7500) == 0)) {
    }
    else if ((strcmp(dev->descr.prod_model, R5500) == 0) || (strcmp(dev->descr.prod_model, RTSA7550) == 0)) {
        // test for attenuation
        for (i=0; i <= 30; i+=10) { //dev->desc.max_att
            result = wsa_set_attenuation(dev, i);
            result = wsa_get_attenuation(dev, &attenuator);
            verify_signed32_result(test_info, result, i, attenuator);
        }
        
        // TODO add expected failed tests
    }
    
	return 0;
}


// Test different sweep attenuation settings 
// results are stored in the pass/fail count variables
int16_t sweep_attenuation_tests(struct wsa_device *dev, struct test_data *test_info) {

	int16_t result;
	int32_t attenuator;
    int i;
    
    init_test_data(test_info);
	
    if ((strcmp(dev->descr.prod_model, WSA5000) == 0) || (strcmp(dev->descr.prod_model, RTSA7500) == 0)) {
    }
    else if ((strcmp(dev->descr.prod_model, R5500) == 0) || (strcmp(dev->descr.prod_model, RTSA7550) == 0)) {
        // test for attenuation
        for (i=0; i <= 30; i+=10) { //dev->desc.max_att
            result = wsa_set_sweep_attenuation(dev, (int32_t) 0);
            result = wsa_get_sweep_attenuation(dev, &attenuator);
            verify_signed32_result(test_info, result, 0, attenuator);
        }
        
        // TODO add expected failed tests
    }
    
	return 0;
}
