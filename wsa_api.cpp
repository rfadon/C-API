/**
 * @mainpage Introduction
 *
 * This documentation, compiled using Doxygen, describes in details the 
 * wsa_api library.  The wsa_api provides
 * functions to set/get particular settings or acquire data from the WSA.  
 * The wsa_api encodes the commands into SCPI syntax scripts, which 
 * are sent to a WSA through the wsa_lib library.  Subsequently, it decodes 
 * any responses or packet coming back from the WSA through the wsa_lib.
 * Thus, the API helps to abstract away SCPI syntax from the user.
 *
 * Data frames passing back from the wsa_lib are in VRT format.  This 
 * API will extract the information and the actual data frames within
 * the VRT packet and makes them available in structures and buffers 
 * for users.
 *
 * @section limitation Limitations in v1.0
 * The following features are not yet supported with the CLI:
 *  - DC correction.  Need Nikhil to clarify on that.
 *  - IQ correction.  Same as above.
 *  - Automatic finding of a WSA box(s) on a network.
 *  - Set sample sizes. 1024 size for now.
 *  - Triggers.
 *  - Gain calibrarion. TBD with triggers.
 *  - USB interface method - might never be available.
 *
 * @section usage How to use the library
 * The wsa_api is designed using mixed C/C++ languages.  To use the 
 * library, you need to include the header file, wsa_api.h, in files that 
 * will use any of its functions to access a WSA, and a link to 
 * the wsa_api.lib.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "stdint.h"
#include "wsa_error.h"
#include "wsa_lib.h"
#include "wsa_api.h"

#define MAX_ANT_PORT 2


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
 * - the interface method does not exist for the WSA product version.
 * - the WSA is not detected (has not been connected or powered up).
 * -
 */
int16_t wsa_open(struct wsa_device *dev, char *intf_method)
{
	int16_t result = 0;		// result returned from a function

	// Start the WSA connection
	// NOTE: API will always assume SCPI syntax
	if ((result = wsa_connect(dev, SCPI, intf_method)) < 0) {
		return result;
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
 * @return 1 if the IP is valid, or a negative number on error.
 */
int16_t wsa_check_addr(char *ip_addr) 
{
	if (wsa_verify_addr(ip_addr) == INADDR_NONE) {
		doutf(1, "Error WSA_ERR_INVIPHOSTADDRESS: %s \"%s\".\n", 
			wsa_get_err_msg(WSA_ERR_INVIPHOSTADDRESS), ip_addr);
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


/**
 * Read command line(s) stored in the given \b file_name and set each line
 * to the WSA.
 *
 * @remarks 
 * - Assuming each command line is for a single function followed by
 * a new line.
 * - Currently read only SCPI commands. Other types of commands, TBD.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param file_name - A pointer to the file name
 *
 * @return Number of command lines at success, or a negative error number.
 */
int16_t wsa_set_command_file(struct wsa_device *dev, char *file_name)
{
	int16_t result = 0;
	result = wsa_send_command_file(dev, file_name);
	
	return result;
}


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Gets the absolute maximum RF input level (dBm) for the WSA at 
 * the given gain setting.\n
 * Operating the WSA device at the absolute maximum may cause damage to the 
 * device.
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
	if (gain < WSA_GAIN_VLOW || gain > WSA_GAIN_HIGH) {		
		return WSA_ERR_INVRFGAIN;
	}
	else {
		// Should never reach here
		return dev->descr.abs_max_amp[gain];
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

	query = wsa_send_query(dev, ":SENSE:FREQ:CENTER?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");

	//return (int64_t) atol(query.result);
	return 0;
}


// TODO Mentioned here that to do onboard capture will need to call
// start_onbard_capture()
/**
 * Sets the WSA to the desired center frequency, \b cfreq.
 * @remarks \b wsa_set_freq() will return error if trigger mode is already
 * running.
 * See the \b descr component of \b wsa_dev structure for maximum/minimum
 * frequency values.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cfreq - The center frequency to set, in Hz
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Frequency out of range.
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_freq(struct wsa_device *dev, uint64_t cfreq) // get vco vsn?
{
	int16_t result = 0;
	char temp_str[50];

	if ((result = wsa_verify_freq(dev, cfreq)) < 0)
		return result;

	sprintf(temp_str, ":SENSE:FREQ:CENTER %llu Hz\n", cfreq);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}

	return result;
}


// A Local function:
// Verify if the frequency is valid (within allowed range)
int16_t wsa_verify_freq(struct wsa_device *dev, uint64_t freq)
{
	int64_t residue; 
	// verify the frequency value
	if (freq < dev->descr.min_tune_freq || freq > dev->descr.max_tune_freq)	{
		doutf(1, "Error WSA_ERR_FREQOUTOFBOUND: %s.\n", 
			wsa_get_err_msg(WSA_ERR_FREQOUTOFBOUND));
		return WSA_ERR_FREQOUTOFBOUND;
	}
	
	// TODO resolution for different WSA!
	residue = freq - ((freq / dev->descr.freq_resolution) * 
		dev->descr.freq_resolution);
	if (residue > 0) {
		doutf(1, "Error WSA_ERR_INVFREQRES: %s.\n", 
			wsa_get_err_msg(WSA_ERR_INVFREQRES));
		return WSA_ERR_INVFREQRES;
	}

	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Gets the current IF gain value of the RFE in dB.
 *
 * @param dev - A pointer to the WSA device structure.
 * @return The gain value in dB, or a large negative number on error.
 */
float wsa_get_gain_if (struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, ":INPUT:GAIN:IF?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");
		
	return (float) atof(query.result);
}


/**
 * Sets the gain value in dB for the variable IF gain stages of the RFE, which 
 * is additive to the primary RF quantized gain stages (wsa_set_gain_rf()).
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain level in dB.
 * @remarks See the \b descr component of \b wsa_dev structure for 
 * maximum/minimum IF gain values. ???
 *
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Gain level out of range.
 */
int16_t wsa_set_gain_if (struct wsa_device *dev, float gain)
{
	int16_t result = 0;
	char temp_str[50];

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, ":INPUT:GAIN:IF %.02f dB\n", gain);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_IFGAINSETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_IFGAINSETFAILED));
		return WSA_ERR_IFGAINSETFAILED;
	}

	return result;
}


/**
 * Gets the current quantized RF front end gain setting of the RFE.
 *
 * @param dev - A pointer to the WSA device structure.
 * @return The gain setting of wsa_gain type, or a negative number on error.
 */
wsa_gain wsa_get_gain_rf (struct wsa_device *dev)
{
	wsa_gain gain = (wsa_gain) NULL;
	struct wsa_resp query;		// store query results

	
	query = wsa_send_query(dev, ":INPUT:GAIN:RF?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");

	
	// Convert to wsa_gain type
	if (strstr(query.result, "HIGH") != NULL) {
		gain = WSA_GAIN_HIGH;
	}
	else if (strstr(query.result, "MEDIUM") != NULL) {
		gain = WSA_GAIN_MEDIUM;
	}
	else if (strstr(query.result, "VLOW") != NULL) {
		gain = WSA_GAIN_VLOW;
	}
	else if (strstr(query.result, "LOW") != NULL) {
		gain = WSA_GAIN_LOW;
	}
	else
		gain = (wsa_gain) NULL;

	return gain;
}

/**
 * Sets the quantized \b gain (sensitivity) level for the RFE of the WSA.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain setting of type wsa_gain to set for WSA. \n
 * Valid gain settings are:
 * - WSA_GAIN_HIGH
 * - WSA_GAIN_MEDIUM
 * - WSA_GAIN_LOW 
 * - WSA_GAIN_VLOW
 * 
 * @return 0 on success, or a negative number on error.
  * @par Errors:
 * - Gain setting not allow.
 */
int16_t wsa_set_gain_rf (struct wsa_device *dev, wsa_gain gain)
{
	int16_t result = 0;
	char temp_str[30];

	if (gain > WSA_GAIN_VLOW || gain < WSA_GAIN_HIGH)
		return WSA_ERR_INVRFGAIN;

	strcpy(temp_str, ":INPUT:GAIN:RF ");
	switch(gain) {
		case(WSA_GAIN_HIGH):	strcat(temp_str, "HIGH"); break;
		case(WSA_GAIN_MEDIUM):	strcat(temp_str, "MEDIUM"); break;
		case(WSA_GAIN_LOW):		strcat(temp_str, "LOW"); break;
		case(WSA_GAIN_VLOW):	strcat(temp_str, "VLOW"); break;
		default:		strcat(temp_str, "ERROR"); break;
	}

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_RFGAINSETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_RFGAINSETFAILED));
		return WSA_ERR_RFGAINSETFAILED;
	}

	return result;
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
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, ":INPUT:ANTENNA?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");
		
	return 0;
}


/**
 * Sets the antenna port to be used for the RFE board.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer port number to used. \n
 * Available ports: 1, 2.  Or see product datasheet for ports availability.
 * \b Note: When calibration mode is enabled through wsa_run_cal_mode(), these 
 * antenna ports will not be available.  The seletected port will resume when 
 * the calibration mode is set to off.
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_antenna(struct wsa_device *dev, uint8_t port_num)
{
	int16_t result = 0;
	char temp_str[30];

	if (port_num < 1 || port_num > MAX_ANT_PORT)
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, ":INPUT:ANTENNA %d\n", port_num);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_ANTENNASETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_ANTENNASETFAILED));
		return WSA_ERR_ANTENNASETFAILED;
	}

	return result;
}


/**
 * Gets the current mode of the RFE's preselect BPF stage.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 1 (on), 0 (off), or a negative number on error.
 */
int16_t wsa_get_bpf(struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, ":INPUT:FILTER:PRESELECT:STATE?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");
		
	return 0;
}


/**
 * Sets the RFE's preselect band pass filter (BPF) stage on or off (bypassing).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_bpf(struct wsa_device *dev, uint8_t mode)
{
	int16_t result = 0;
	char temp_str[50];

	if (mode < 0 || mode > 1)
		return WSA_ERR_INVFILTERMODE;

	sprintf(temp_str, ":INPUT:FILTER:PRESELECT:STATE %d\n", mode);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_FILTERSETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_FILTERSETFAILED));
		return WSA_ERR_FILTERSETFAILED;
	}

	return result;
}


// /**
// * Gets the current mode of the RFE's internal anti-aliasing LPF.
// * 
// * @param dev - A pointer to the WSA device structure.
// *
// * @return 1 (on), 0 (off), or a negative number on error.
// */
//int16_t wsa_get_lpf(struct wsa_device *dev)
//{
//	struct wsa_resp query;		// store query results
//
//	query = wsa_send_query(dev, ":INPUT:FILTER:ANTIALIAS:STATE?\n");
//
//	// TODO Handle the query output here 
//	if (query.status > 0)
//		//return atof(query.result);
//		printf("Got %llu bytes: \"%s\"", query.status, query.result);
//	else
//		printf("No query response received.\n");
//
//	return 0;
//}


// /**
// * Sets the internal anti-aliasing low pass filter (LPF) on or off (bypassing).
// * 
// * @param dev - A pointer to the WSA device structure.
// * @param mode - An integer mode of selection: 0 - Off, 1 - On.
// *
// * @return 0 on success, or a negative number on error.
// */
//int16_t wsa_set_lpf(struct wsa_device *dev, uint8_t mode)
//{
//	int16_t result = 0;
//	char temp_str[50];
//
//	if (mode < 0 || mode > 1)
//		return WSA_ERR_INVFILTERMODE;
//
//	sprintf(temp_str, ":INPUT:FILTER:ANTIALIAS:STATE %d\n", mode);
//
//	// set the freq using the selected connect type
//	if ((result = wsa_send_command(dev, temp_str)) < 0) {
//		doutf(1, "Error WSA_ERR_FILTERSETFAILED: %s.\n", 
//			wsa_get_err_msg(WSA_ERR_FILTERSETFAILED));
//		return WSA_ERR_FILTERSETFAILED;
//	}
//
//	return result;
//}


/**
 * Checks if the RFE's internal calibration has finished or not.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 1 if the calibration is still running or 0 if completed, 
 * or a negative number on error.
 */
int16_t wsa_query_cal_mode(struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, "CALIBRATE:RFE:STATE?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		//return atof(query.result);
		printf("Got %llu bytes: \"%s\"", query.status, query.result);
	else
		printf("No query response received.\n");

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
	int16_t result = 0;
	char temp_str[30];

	if (mode < 0 || mode > 1)
		return WSA_ERR_INVCALIBRATEMODE;

	sprintf(temp_str, "CALIBRATE:RFE:STATE %d\n", mode);

	// set the freq using the selected connect type
	if ((result = wsa_send_command(dev, temp_str)) < 0) {
		doutf(1, "Error WSA_ERR_CALIBRATESETFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_CALIBRATESETFAILED));
		return WSA_ERR_CALIBRATESETFAILED;
	}

	return result;
}
