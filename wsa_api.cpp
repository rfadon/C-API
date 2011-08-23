
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "stdint.h"
#include "wsa_lib.h"
#include "wsa_commons.h"
#include "wsa_api.h"
#include "wsa_error.h"


/**
 * Initialized the the wsa_device structure
 *
 * @param dev - a wsa device structure.
 */
int16_t wsa_dev_init(wsa_device *dev)
{
	dev->descr.inst_bw = 0;
	dev->descr.max_pkt_size = 0;
	dev->descr.max_tune_freq = 0;
	dev->descr.min_tune_freq = 0;
	strcpy(dev->descr.prod_name, "");
	strcpy(dev->descr.prod_serial, ""); 
	strcpy(dev->descr.prod_version, "");
	strcpy(dev->descr.rfe_name, "");
	strcpy(dev->descr.rfe_version, "");
	strcpy(dev->descr.fw_version, "");

	//dev->run_mode = (wsa_run_mode) 0;
	//dev->trig_list = (wsa_trig *) malloc(sizeof(wsa_trig));

	return 0;
}


/**
 * Establishes a connection of choice specified by the interface method to 
 * the WSA.\n At success, the handle remains open for future access by other 
 * library methods until wsa_close() is called. When unsuccessful, the WSA 
 * will be closed automatically and an error is returned.
 *
 * @param dev - a WSA device structure to be opened.
 * @param intf_method - The interface method to the WSA. \n
 * Possible methods: \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::HISLIP" \n
 * - With USB, use: "USB" (check if supported with the WSA version used). \n
 *
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * Situations that will generate an error are:
 * - the selected connection type does not exist for the WSA product version.
 * - the WSA is not detected (has not been connected or powered up)
 * -
 */
int16_t wsa_open(wsa_device *dev, char *intf_method)
{
	int16_t result = 0;		// result returned from a function

	// Initialize wsa_device structure
	if (wsa_dev_init(dev) < 0) {
		doutf(1, "Error WSA_ERR_USBINITFAILED: "
			"%s.\n", wsa_get_err_msg(WSA_ERR_INITFAILED));
		return WSA_ERR_OPENFAILED;
	}

	// Connect base on the interface type
	if (strncmp(intf_method, "USB", 3) == 0) {
		// TODO: add to this section if ever use USB.
		doutf(1, "Error WSA_ERR_USBNOTAV: %s.\n", 
			wsa_get_err_msg(WSA_ERR_USBNOTAV));
		return WSA_ERR_OPENFAILED;	
	}

	else if (strncmp(intf_method, "TCPIP", 5) == 0) {	
		// Start the WSA connection
		if ((result = wsa_connect(dev, SCPI, intf_method)) < 0) {
			//printf("ERROR: Failed to connect to the WSA at %s.\n", wsa_addr);
			doutf(1, "Error WSA_ERR_ETHERNETCONNECTFAILED: %s.\n", 
				wsa_get_err_msg(WSA_ERR_ETHERNETCONNECTFAILED));
			return WSA_ERR_ETHERNETCONNECTFAILED;
		}

		// TODO: get & update the versions & wsa model
		// TODO will need to replace with reading from reg or eeprom?
		sprintf(dev->descr.prod_name, "%s", WSA4000);
		strcpy(dev->descr.prod_serial, "TO BE DETERMINED"); // temp for now
		sprintf(dev->descr.prod_version, "v1.0"); // temp value
		sprintf(dev->descr.rfe_name, "%s", WSA_RFE0560);
		sprintf(dev->descr.rfe_version, "v1.0"); // temp
		strcpy(dev->descr.fw_version, "v1.0");
	}
	else {
		doutf(1, "Error WSA_ERR_INVCONNTYPE: %s.\n", 
			wsa_get_err_msg(WSA_ERR_INVCONNTYPE));
		return WSA_ERR_OPENFAILED;
	}

	// 3rd, set some values base on the model
	// TODO read from regs/eeprom instead???
	if (strcmp(dev->descr.prod_name, WSA4000) == 0) {
		dev->descr.max_pkt_size = WSA4000_MAX_PKT_SIZE;
		dev->descr.inst_bw = WSA4000_INST_BW;
		
		if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			dev->descr.max_tune_freq = WSA_RFE0560_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
		}
	}

	return 0;
}

