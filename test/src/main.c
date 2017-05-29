
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "wsa_lib.h"
#include "wsa_api.h"
#include "test_util.h"
#include "thinkrf_stdint.h"
#include <attenuation_tests.h>
#include <sweep_device_tests.h>


/**
 * Starting point
 */
int32_t main(int32_t argc, char *argv[])
{
// initialize WSA structure and IP setting
    struct wsa_device wsa_dev;	// the wsa device structure
    struct wsa_device *dev;
	char wsa_addr[255] = "10.126.110.132";	// store wsa ip address
    char intf_str[255];

	struct test_data test_info;

	int16_t result;
	int16_t acq_result;

	test_info.fail_count = 0;
	test_info.pass_count = 0;
	test_info.bug_count = 0;
	test_info.fail_expected = 0;

	// connect to device
	sprintf(intf_str, "TCPIP::%s", wsa_addr);
    dev = &wsa_dev;
	result = wsa_open(dev, intf_str); 
	
	// reset the device
	result =  wsa_send_scpi(dev, "*RST");
	result = wsa_system_abort_capture(dev);
	result = wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_result);

	// ATTENUATION TESTS: Test attenuation for all 4 valid values (0, 10, 20, 30)
	result = attenuation_tests(dev, &test_info);
	printf("ATTENATION TEST RESULTS: %d Tests, %d Passes, %d Fails \n \n",test_info.pass_count + test_info.fail_count, test_info.pass_count, test_info.fail_count);


	// ATTENUATION TESTS: Test sweep attenuation for all 4 valid values (0, 10, 20, 30)
	result = sweep_attenuation_tests(dev, &test_info);
	printf("SWEEP ATTENATION TEST RESULTS: %d Tests, %d Passes, %d Fails \n \n", test_info.pass_count + test_info.fail_count, test_info.pass_count, test_info.fail_count);

	result = sweep_device_tests(dev, &test_info);
	printf("SWEEP DEVICE RESULTS: %d Tests, %d Passes, %d Fails \n \n", test_info.pass_count + test_info.fail_count, test_info.pass_count, test_info.fail_count);

	printf("TOTAL TEST RESULTS: %d Tests, %d Passes, %d Fails \n \n", test_info.pass_count + test_info.fail_count, test_info.pass_count, test_info.fail_count);
	return 0;
}
