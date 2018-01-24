//*****************************************************************************
// An sweep example with one sweep entry
//
// Note: wsaSweep.cpp - Defines the entry point for the console application.
//*****************************************************************************

#include "stdafx.h"
extern "C" {
        #include "wsa_api.h"
        #include "wsa_error.h"
        #include "wsa_sweep_device.h"
        #include <stdio.h>
}
#include <iostream>
#include <time.h>


int _tmain(int argc, _TCHAR* argv[])
{        
    // store wsa ip address
    char wsa_addr[255];    
    char intf_str[255];
    int16_t result;
    int16_t acq_status = 0;

    struct wsa_device wsa_device;
    struct wsa_device *wsadev = &wsa_device;
    struct wsa_sweep_device wsa_sweep_device;
    struct wsa_sweep_device *wsasweepdev = &wsa_sweep_device;

    // initialize fstart/fstop and rbw (Hz)
    uint64_t fstart = 2400000000;
    uint64_t fstop = 2500000000;
    uint32_t rbw = 50000;

    // initialize attenuation
    unsigned int attenuation = 0;

    // initialize config buffer
    struct wsa_power_spectrum_config *pscfg;
    
    // initialize power spectrum buffer
    float *psbuf;

    // grab device IP from user
    printf("Enter an IP address: ");
    scanf("%s", wsa_addr);    
    sprintf(intf_str, "TCPIP::%s", wsa_addr);

    // connect to R5500
    result = wsa_open(wsadev, intf_str);

    // reset R5500 state
    result = wsa_system_abort_capture(wsadev);
    wsa_flush_data(wsadev);
    result = wsa_system_request_acq_access(wsadev, &acq_status);

    // create the sweep device
    wsasweepdev = wsa_sweep_device_new(wsadev);

    // configure the attenuation
    wsa_sweep_device_set_attenuator(wsasweepdev, attenuation);

    // allocate memory for our ffts to go in
    result = wsa_power_spectrum_alloc(wsasweepdev, fstart, fstop, rbw, "SHN", &pscfg);
    
    // configure the sweep (note this only needs to be done once)
    wsa_configure_sweep(wsasweepdev, pscfg);

    // capture some spectrum
    wsa_capture_power_spectrum(wsasweepdev, pscfg, &psbuf);

    // print the spectral data
    for(int i = 0; i < pscfg->buflen; i++) {
        printf("%0.2f \n", pscfg->buf[i]);
    }

    // close R5500 connection
    wsa_close(wsadev);

    return 0;
}
