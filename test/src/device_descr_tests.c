#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_error.h>
#include <test_util.h>
#include <string.h>

#ifdef _WIN32
# define strtok_r strtok_s
#endif

// uses an R5500 device to test different sweep device settings
int16_t test_device_descr(struct wsa_device *dev, struct test_data *test_info)
{

	int16_t result;
	char raw_idn[255];

	init_test_data(test_info);
	
	// retrieve the *IDN command
	result = wsa_query_scpi(dev, "*IDN?", raw_idn);
	printf("ID: %s \n", raw_idn);
	
	printf("Product Model: %s \n", dev->descr.prod_model);
	printf("Device Model: %s \n", dev->descr.dev_model);
	printf("serial number: %s \n", dev->descr.serial_number);
	printf("Connection type: %s \n", dev->descr.intf_type);

	printf("Firmware Version: %s \n", dev->descr.fw_version);
	printf("Min Frequency: %0.2f \n", (float) dev->descr.min_tune_freq);
	printf("Max Frequency: %llu \n", dev->descr.max_tune_freq);
	printf("Instantaneous BW: %llu \n", dev->descr.inst_bw);
	printf("Frequency resolution: %0.2f \n", (float) dev->descr.freq_resolution);
	printf("Max Sample Size: %d \n",dev->descr.max_sample_size);
	printf("Min Decimation: %d \n", dev->descr.min_decimation);
	printf("Max Decimation: %d \n", dev->descr.max_decimation);

	return result;
}
