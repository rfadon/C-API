
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "stdint.h"
#include "wsa_commons.h"
#include "wsa_error.h"
#include "wsa_lib.h"
#include "wsa_api.h"


// ////////////////////////////////////////////////////////////////////////////
// Local functions                                                           //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_verify_freq(struct wsa_device *dev, uint64_t freq);


// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Establishes a connection of choice specified by the interface method to 
 * the WSA.\n At success, the handle remains open for future access by other 
 * library methods until wsa_close() is called. When unsuccessful, the WSA 
 * will be closed automatically and an error is returned.
 *
 * @param dev - A pointer to the WSA device structure to be opened.
 * @param intf_method - A char pointer to store the interface method to the 
 * WSA. \n Possible methods: \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::HISLIP" \n
 * - With USB, use: "USB" (check if supported with the WSA version used). \n
 *
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * Situations that will generate an error are:
 * - the selected connection type does not exist for the WSA product version.
 * - the WSA is not detected (has not been connected or powered up).
 * -
 */
int16_t wsa_open(struct wsa_device *dev, char *intf_method)
{
	int16_t result = 0;		// result returned from a function

	// Start the WSA connection
	// NOTE: API will always assume SCPI syntax
	if ((result = wsa_connect(dev, SCPI, intf_method)) < 0) {
		//printf("ERROR: Failed to connect to the WSA at %s.\n", wsa_addr);
		doutf(1, "Error WSA_ERR_ETHERNETCONNECTFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_ETHERNETCONNECTFAILED));
		return WSA_ERR_ETHERNETCONNECTFAILED;
	}

	return 0;
}


/**
 * Closes the device handle if one is opened and stops any existing data 
 * capture.
 *
 * @param dev - A pointer to a WSA device structure to be closed.
 *
 * @return none
 */
void wsa_close(struct wsa_device *dev)
{
	wsa_disconnect(dev);
}


/**
 * Verify if the IP address or host name given is valid for the WSA.
 * 
 * @param ip_addr - A char pointer to the IP address or host name to be 
 * verified.
 * 
 * @return 1 if the IP is valid, 0 if invalid (?), or a negative number 
 * on error.
 */
int16_t wsa_check_addr(char *ip_addr) 
{
	if (wsa_verify_addr(ip_addr) == INADDR_NONE) {
		doutf(1, "Error WSA_ERR_INVIPHOSTADDRESS: %s.\n", 
			wsa_get_err_msg(WSA_ERR_INVIPHOSTADDRESS));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

	// TODO add hook to check then if the address is actually for the WSA
	// such as some handshaking
	// do this in the client level?
	else
		return 1;
}


/**
 * Count and print out the IPs of connected WSAs to the network? or the PC???
 * For now, will list the IPs for any of the connected devices to a PC?
 *
 * @param wsa_list - A double char pointer to store (WSA???) IP addresses 
 * connected to a network???.
 *
 * @return Number of connected WSAs (or IPs for now) on success, or a 
 * negative number on error.
 */
// TODO: This section is to be replaced w/ list connected WSAs
int16_t wsa_list(char **wsa_list) 
{
	int16_t result = 0;			// result returned from a function

	result = wsa_list_devs(wsa_list);

	return result;
}



/**
 * Indicates if the WSA is still connected to the PC.
 *
 * @param dev - A pointer to the WSA device structure to be verified for 
 * the connection.
 * @return 1 if it is connected, 0 if not connected, or a negative number 
 * if errors.
 */
int16_t wsa_is_connected(struct wsa_device *dev) 
{
	struct wsa_resp query;		// store query results

	// TODO check version & then do query
	query = wsa_send_query(dev, "*STB?"); // ???

	//TODO Handle the response here
	
	return 0;
}



//TODO Check with Jacob how to distinct between onboard vs real time data 
// capture ????

///////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
///////////////////////////////////////////////////////////////////////////////

/**
 * Retrieves the center frequency that the WSA is running at.
 *
 * @param dev - A pointer to the WSA device structure.
 * @return The frequency in Hz, or a negative number on error.
 */
int64_t wsa_get_freq(struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, ":INPUT:CENTER?\n");

	// TODO Handle the query output here 
	//if (query.status > 0)
	//	return atof(query.result);
	return 0;
}


// TODO Mentioned here that to do onboard capture will need to call
// start_onbard_capture()
/**
 * Sets the WSA to the desired center frequency, \b cfreq.
 * @remarks \b wsa_set_freq() will return error if trigger mode is already
 * running.  Use wsa_set_run_mode() with FREERUN to change.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cfreq - The center frequency to set, in Hz
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_freq(struct wsa_device *dev, uint64_t cfreq) // get vco vsn?
{
	int16_t result = 0;
	char temp_str[30];

	if ((result = wsa_verify_freq(dev, cfreq)) < 0)
		return result;

	sprintf(temp_str, ":INPUT:CENTER <%llu>\n", cfreq);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_ETHERNETNOTAVBL: %s.\n", 
			wsa_get_err_msg(WSA_ERR_ETHERNETNOTAVBL));
		return WSA_ERR_ETHERNETNOTAVBL;
	}

	return result;
}


// A Local function:
// Verify if the frequency is valid (within allowed range)
int16_t wsa_verify_freq(struct wsa_device *dev, uint64_t freq)
{
	// verify the frequency value
	if (freq < dev->descr.min_tune_freq || freq > dev->descr.max_tune_freq)
	{
		doutf(1, "Error WSA_ERR_FREQOUTOFBOUND: %s.\n", 
			wsa_get_err_msg(WSA_ERR_FREQOUTOFBOUND));
		return WSA_ERR_FREQOUTOFBOUND;
	}
	
	// TODO resolution for different WSA!
	else if ((freq - ((freq / WSA_RFE0560_FREQRES) * WSA_RFE0560_FREQRES)) > 0)
	{
		doutf(1, "Error WSA_ERR_INVFREQRES: %s.\n", 
			wsa_get_err_msg(WSA_ERR_INVFREQRES));
		return WSA_ERR_INVFREQRES;
	}

	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Gets the absolute maximum RF input level (dBm) for the WSA at 
 * the given gain setting.\n
 * Operating the unit at the absolute maximum may cause damage to the device.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain setting of \b wsa_gain type at which the absolute 
 * maximum amplitude input level is to be retrieved.
 *
 * @return The absolute maximum RF input level in dBm or negative error number.
 */
float wsa_get_abs_max_amp(struct wsa_device *dev, wsa_gain gain)
{
	// TODO Check version of WSA & return the correct info here
	if (strcmp(dev->descr.prod_name, WSA4000) == 0) {		
		if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			switch (gain) {
			case (HIGH):
				return WSA_RFE0560_ABS_AMP_HIGH;
				break;
			case (MEDIUM):
				return WSA_RFE0560_ABS_AMP_MEDIUM;
				break;
			case (LOW):
				return WSA_RFE0560_ABS_AMP_LOW;
				break;
			case (ULOW):
				return WSA_RFE0560_ABS_AMP_ULOW;
				break;
			default:
				return WSA_ERR_INVGAIN;
				break;
			}
		}
		else
			return WSA_ERR_UNKNOWNRFEVSN;
	}
	else {
		// Should never reach here
		return WSA_ERR_UNKNOWNPRODVSN;
	}
}


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Reads a frame of data. \e Each frame consists of a header, and I and Q 
 * buffers of data of length determine by the \b sample_size parameter.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_frame_header structure to store 
 * information for the frame.
 * @param i_buf - A 16-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the sample_size.
 * @param q_buf - A 16-bit signed integer pointer for the unscaled 
 * Q data buffer with size specified by the sample_size.
 * @param sample_size - A 64-bit unsigned integer sample size (i.e. {I, Q} 
 * sample pairs) per data frame to be captured. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of data samples read upon success, or a negative 
 * number on error.
 */
int64_t wsa_read_pkt (struct wsa_device *dev, struct wsa_frame_header *header, 
			int16_t *i_buf, int16_t *q_buf, const uint64_t sample_size)
{
	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Gets the current gain setting of the WSA.
 *
 * @param dev - A pointer to the WSA device structure.
 * @return The gain setting of wsa_gain type, or a negative number on error.
 */
wsa_gain wsa_get_gain (struct wsa_device *dev)
{
	wsa_gain gain = LOW;

	return gain;
}

/**
 * Sets the \b gain (sensitivity) level for the radio front end of the WSA.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain setting of type wsa_gain to set for WSA. \n
 * Valid gain settings are HIGH, MEDIUM, LOW, ULOW.
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_gain (struct wsa_device *dev, wsa_gain gain)
{
	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Gets which antenna port is currently in used with the RFE board.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return The antenna port number on success, or a negative number on error.
 */
int16_t wsa_get_antenna(struct wsa_device *dev)
{
	return 0;
}


/**
 * Sets the antenna port to be used for the RFE board.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer port number to used. \n
 * Available ports: 1, 2, 3.
 * \b Note: When calibration mode is enabled through wsa_run_cal_mode(), these 
 * antenna ports will not be available.  The seletected port will resume when 
 * the calibration mode is set to off.
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_antenna(struct wsa_device *dev, uint8_t port_num)
{
	return 0;
}


/**
 * Gets the current mode of the RFE's internal BPF.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 1 (on), 0 (off), or a negative number on error.
 */
int16_t wsa_get_bpf(struct wsa_device *dev)
{
	return 0;
}


/**
 * Sets the RFE's internal band pass filter (BPF) on or off (bypassing).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_bpf(struct wsa_device *dev, uint8_t mode)
{
	return 0;
}


/**
 * Gets the current mode of the RFE's internal LPF.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 1 (on), 0 (off), or a negative number on error.
 */
int16_t wsa_get_lpf(struct wsa_device *dev)
{
	return 0;
}


/**
 * Sets the internal low pass filter (LPF) on or off (bypassing).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lpf(struct wsa_device *dev, uint8_t option)
{
	return 0;
}


/**
 * Checks if the RFE's internal calibration has finished or not.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 1 if the calibration is still running or 0 if completed, 
 * or a negative number on error.
 */
int16_t wsa_check_cal_mode(struct wsa_device *dev)
{
	return 0;
}


/**
 * Runs the RFE'S internal calibration mode or cancel it. \n
 * While the calibration mode is running, no other commands should be 
 * running until the calibration is finished by using wsa_query_cal_mode(), 
 * or could be cancelled
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer mode of selection: 1 - Run, 0 - Cancel.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_run_cal_mode(struct wsa_device *dev, uint8_t mode)
{
	return 0;
}