#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wsa_lib.h"
#include "wsa_api.h"
#include "test_util.h"
#include "thinkrf_stdint.h"


/**
 * Starting point
 */
int32_t main(int32_t argc, char *argv[])
{
    // initialize WSA structure and IP setting
    struct wsa_device wsa_dev;	// the wsa device structure
    struct wsa_device *dev;
    char intf_str[255];
	struct test_data test_info;
	int16_t result;
	int16_t acq_result;
    int total_tests = 0;
    int total_passes = 0;
    int total_fails = 0;

    // Check for the correct number of arguments
    if (argc != 2) {
       fprintf(stderr, "Usage: %s <Device IP>\n", argv[0]);
       exit(1);
    }

	// connect to device
	sprintf(intf_str, "TCPIP::%s", argv[1]);
    dev = &wsa_dev;
	result = wsa_open(dev, intf_str); 
	
	// reset the device
	result = wsa_send_scpi(dev, "*RST");
	result = wsa_system_abort_capture(dev);
	result = wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_result);

    
    printf("\n===============================\n");
    printf("*****  DUT - IP: %s  *****\n\n", argv[1]);
   
	result = test_device_descr(dev, &test_info);
	printf("\nDEVICE DESCR RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;

     
    printf("\n\n===============================\n");
	// ATTENUATION TESTS: Test attenuation for all valid values
	result = attenuation_tests(dev, &test_info);
	printf("ATTENUATION TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;

/*
    printf("\n\n===============================\n");
	// SWEEP ATTENUATION TESTS: Test sweep attenuation for all valid values
	result = sweep_attenuation_tests(dev, &test_info);
	printf("SWEEP ATTENUATION TEST RESULTS: %d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;

    
    printf("\n\n===============================\n");
	// FREQUENCY TESTS:
	result = freq_tests(dev, &test_info);
	printf("FREQUENCY TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;
    

    printf("\n\n===============================\n");
	// BLOCK CAPTURE TESTS:
	result = block_capture_tests(dev, &test_info);
	printf("BLOCK CAPTURE TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;
    
   
    printf("\n\n===============================\n");
	// STREAM TESTS:
	result = stream_tests(dev, &test_info);
	printf("STREAM TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;
   
   
    printf("\n\n===============================\n");
	// SWEEP FREQUENCY TESTS:
	result = sweep_freq_tests(dev, &test_info);
	printf("SWEEP FREQUENCY TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;

    
    printf("\n\n===============================\n");
	// SWEEP TESTS:
	result = sweep_tests(dev, &test_info);
	printf("SWEEP TEST RESULTS:\n\t%d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;
   
 */    
    printf("\n\n===============================\n");
    printf("SWEEP DEVICE TEST\n");
	result = sweep_device_tests(dev, &test_info);
	printf("SWEEP DEVICE RESULTS: %d Tests, %d Passes, %d Fails\n", test_info.test_count, test_info.pass_count, test_info.fail_count);
    total_tests += test_info.test_count;
    total_passes += test_info.pass_count;
    total_fails += test_info.fail_count;
 
 
    printf("\n\nTOTAL TEST RESULTS: %d Tests, %d Passes, %d Fails\n", total_tests, total_passes, total_fails);
        
    wsa_close(dev);
    
	return 0;
}
