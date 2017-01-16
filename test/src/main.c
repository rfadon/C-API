
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>
#include <attenuation_tests.h>


/**
 * Starting point
 */
int32_t main(int32_t argc, char *argv[])
{
// initialize WSA structure and IP setting
    struct wsa_device wsa_dev;	// the wsa device structure
    struct wsa_device *dev;
	char wsa_addr[255] = "10.126.110.104";	// store wsa ip address
    char intf_str[255];
	int i;
	int32_t fail_count = 0;
	int32_t pass_count = 0;

	// initialize WSA settings
	int32_t attenuator = 0;

	int16_t result;

	int16_t acq_result;
	float *spectral_data;

	// initialize sweep device
	struct wsa_sweep_device wsa_Sweep_device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_Sweep_device;
	sprintf(intf_str, "TCPIP::%s", wsa_addr);
    dev = &wsa_dev; // create device pointer
	result = wsa_open(dev, intf_str); 
	result =  wsa_send_scpi(dev, "*RST");
	result = wsa_system_abort_capture(dev);
	result = wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_result);

	// ATTENUATION TESTS: Test attenuation for all 4 valid values (0, 10, 20, 30)
	result = attenuation_tests(dev, &fail_count, &pass_count);

	printf("TEST RESULTS: %d Tests, %d Passes, %d Fails", fail_count + pass_count, pass_count, fail_count);
	return 0;
}
