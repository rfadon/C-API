#include <stdio.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include "test_util.h"


/**
 * Test different freq settings
 * results are stored in the pass/fail count variables
 */
int16_t freq_tests(struct wsa_device *dev, struct test_data *test_info) {

    int16_t result;
    uint64_t freq;
    
    init_test_data(test_info);
    
    // test for min tunable freq
    /*result = wsa_set_freq(dev, dev->descr.min_tune_freq);
    if (result < 0) printf("Failed setting freq %ld\n", dev->descr.min_tune_freq);
    
    result = wsa_get_freq(dev, &freq);
    verify_float_result(test_info, result, (float) (dev->descr.min_tune_freq/MHZ), (float) (freq/MHZ));*/

    // test for freq max
    result = wsa_set_freq(dev, dev->descr.max_tune_freq);
    result = wsa_get_freq(dev, &freq);
    verify_float_result(test_info, result, (float) (dev->descr.max_tune_freq/MHZ), (float) (freq/MHZ));
    
    // test for freq 80
    result = wsa_set_freq(dev, 80*MHZ);
    result = wsa_get_freq(dev, &freq);
    verify_float_result(test_info, result, 80, (float) (freq/MHZ));
    
    // test for freq 300
    result = wsa_set_freq(dev, 300*MHZ);
    result = wsa_get_freq(dev, &freq);
    verify_float_result(test_info, result, 300, (float) (freq/MHZ));
    
    return 0;
}


/**
 * Test different sweep freq settings 
 * results are stored in the pass/fail count variables
 */
int16_t sweep_freq_tests(struct wsa_device *dev, struct test_data *test_info) {

    int16_t result;
    uint64_t freq_start;
    uint64_t freq_stop;
    
    init_test_data(test_info);
    
    // TODO
    //result = wsa_set_sweep_freq(dev, dev->descr.min_tune_freq, dev->descr.max_tune_freq);
    result = wsa_set_sweep_freq(dev, 50*MHZ, dev->descr.max_tune_freq);
    result = wsa_get_sweep_freq(dev, &freq_start, &freq_stop);
    verify_float_result(test_info, result, (float) (dev->descr.min_tune_freq/MHZ), (float) (freq_start/MHZ));
    verify_float_result(test_info, result, (float) (dev->descr.max_tune_freq/MHZ), (float) (freq_stop/MHZ));
    
    return 0;
}
