
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>


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
	// initialize WSA settings

	uint64_t fstart = 20000000;
	uint64_t fstop = 40000000;
	uint32_t rbw = 100000;
	int32_t attenuator = 0;
	uint32_t sweep_size = 0;
	int16_t wsa_system_abort_capture(struct wsa_device *dev);
	int16_t wsa_flush_data(struct wsa_device *dev);
	int16_t result;
	int16_t acq_result;
	char mode[255] = "SH"; // RFE input mode
	float *spectral_data;

	// initialize sweep device
	struct wsa_sweep_device wsa_Sweep_device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_Sweep_device;
	struct wsa_power_spectrum_config *pscfg;
	sprintf(intf_str, "TCPIP::%s", wsa_addr);
	printf("Initializing WSA settings... \n");	
    dev = &wsa_dev; // create device pointer
	result = wsa_open(dev, intf_str); 
	result =  wsa_send_scpi(dev, "*RST");
	result = wsa_system_abort_capture(dev);
	result = wsa_flush_data(dev);
	result = wsa_system_request_acq_access(dev, &acq_result);

	// initialize sweep device
	printf("initializing sweep dev \n");
	wsa_sweep_dev = wsa_sweep_device_new(dev);
	
	printf("allocating memory for fft \n");
	// allocate memory for our ffts to go in
	result=  wsa_power_spectrum_alloc(wsa_sweep_dev, fstart, fstop, rbw, mode, &pscfg);
	
	if (result < 0)
	{
		wsa_power_spectrum_free(pscfg);
		wsa_sweep_device_free(wsa_sweep_dev);
		return result;
	}
	printf("configuring sweep \n");
	wsa_configure_sweep(wsa_sweep_dev, pscfg);
	printf("allocating buffer \n");
	spectral_data = (float *) malloc(sizeof(float) *pscfg->buflen);
	printf("capturing data \n");
	wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &spectral_data);
	for (i = 0; i < (int) (pscfg->buflen); i++){
		//if (spectral_data[i] > -50.0)
			printf("%0.2f, %0.2f \n",(float) ((((uint32_t) i) * pscfg->rbw) + fstart) / 1000000, spectral_data[i]);
	}

	return 0;
}
