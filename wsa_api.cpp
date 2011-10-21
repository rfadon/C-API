/**
 * @mainpage Introduction
 *
 * This documentation, compiled using Doxygen, describes in details the 
 * wsa_api library.  The wsa_api provides
 * functions to set/get particular settings or acquire data from the WSA.  
 * The wsa_api encodes the commands into SCPI syntax scripts, which 
 * are sent to a WSA through the wsa_lib library.  Subsequently, it decodes 
 * any responses or packets coming back from the WSA through the wsa_lib.
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
#include "wsa_commons.h"


// Defines some private prameters
#define WSA_RFE0440 "RFE0440"
#define MAX_ANT_PORT 2

#define SCPI_OSR_CALI 0x0001
#define SCPI_OSR_SETT 0x0002
#define SCPI_OSR_SWE  0x0008
#define SCPI_OSR_MEAS 0x0010
#define SCPI_OSR_TRIG 0x0020
#define SCPI_OSR_CORR 0x0080
#define SCPI_OSR_DATA 0x0100 //?


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
	result = wsa_connect(dev, SCPI, intf_method);
	if (result < 0) {
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
		doutf(DMED, "Error WSA_ERR_INVIPHOSTADDRESS: %s \"%s\".\n", 
			wsa_get_error_msg(WSA_ERR_INVIPHOSTADDRESS), ip_addr);
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
 * Returns a message string associated with the given error code \b err_code.
 * 
 * @param err_code - The negative WSA error code, returned from a WSA function.
 *
 * @return A char pointer to the error message string.
 */
const char *wsa_get_err_msg(int16_t err_code)
{
	return wsa_get_error_msg(err_code);
}



/**
 * Read command line(s) stored in the given \b file_name and send each line
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
static uint16_t frame_count = 0;

/**
 * Reads a frame of data. \e Each frame consists of a header, and a 
 * buffer of data of length determine by the \b sample_size parameter
 * (i.e. sizeof(\b data_buf) = \b sample_size * 4 bytes per sample).
 * 
 * Each I and Q samples is 16-bit (2-byte) wide, signed 2-complement.  The raw 
 * data_buf contains alternatively 2-byte Q follows by 2-byte I, so on.  In 
 * another words, the I & Q samples are distributed in the raw data_buf 
 * as follow:
 *
 * @code 
 *		data_buf = QIQIQIQI... = <2 bytes Q><2bytes I><...>
 * @endcode
 *
 * The bytes can be decoded as follow:
 * @code
 *	Let takes the first 4 bytes of the data_buf, then:
 * 
 *		int16_t I[0] = data_buf[3] << 8 + data_buf[2];
 *		int16_t Q[0] = data_buf[1] << 8 + data_buf[0];
 *
 *  And so on for N number of samples:
 *
 *		int16_t I[i] = data_buf[i+3] << 8 + data_buf[i+2];
 *		int16_t Q[i] = data_buf[i+1] << 8 + data_buf[i];
 *  for i = 0, 1, 2, ..., (N - 1).
 * @endcode
 *
 * Alternatively, the data_buf can be passed to wsa_frame_decode() to have I
 * and Q splited up and stored into separate int16_t buffers.  Or use
 * wsa_get_frame_int() to do both tasks at once.  Those 2 functions are 
 * useful when delaying in data acquisition time between frames is not a 
 * factor. In addition, the wsa_frame_decode() function is useful for later 
 * needs of decoding the data bytes when a large amount of raw data 
 * (multiple frames) has been captured for instance. 
 *
 * @param dev - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_frame_header structure to store 
 * information for the frame.
 * @param data_buf - A char pointer buffer to store the raw I and Q data in
 * in bytes. Its size is determined by the number of 32-bit \b sample_size 
 * words multiply by 4 (i.e. sizeof(\b data_buf) = \b sample_size * 4 bytes per 
 * sample, which is automatically done by the function).
 * @param sample_size - A 32-bit unsigned integer sample size (i.e. number of
 * {I, Q} sample pairs) per data frame to be captured. \n
 * The size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of data samples read upon success, or a 16-bit negative 
 * number on error.
 */
int32_t wsa_read_frame_raw(struct wsa_device *dev, struct wsa_frame_header 
		*header, char *data_buf, const int32_t sample_size)
{
	int16_t result = 0, loop = 0;
	int16_t frame_num = 0;
	uint32_t samples_count = 0;
	
	if ((sample_size < 128) || (sample_size > dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;

	// Query WSA for data using the selected connect type
	// Allow upto 5 times of trying to get data???
	do {
		result = wsa_send_command(dev, "TRACE:IQ?\n");
		if (result < 0) {
			doutf(DMED, "Error WSA_ERR_READFRAMEFAILED: %s.\n", 
				wsa_get_error_msg(WSA_ERR_READFRAMEFAILED));
			return WSA_ERR_READFRAMEFAILED;
		}

		// get data & increment counters
		result = wsa_get_frame(dev, header, data_buf, sample_size, 5000);
		samples_count += header->sample_size;
		loop++;

		// verify output
		if (result < 0) {
			printf("Error getting data... trying again #%d\n", loop);
		}
		// increment the buffer location
		else if (samples_count < sample_size) {
			data_buf += header->sample_size;
			doutf(DHIGH, "%d, ", samples_count);
		}
		else 
			break;

		if (loop >= 5)
			break;
	} while (1);

	if (result < 0)
		return WSA_ERR_READFRAMEFAILED;
	
	// TODO: Verify continuity of frame count. use global variable?
	//if (result != frame_count) {
	//	printf("Warning: The frame count does not seem continuous. Some"
	//	" frames might have been dropped.\n");
	
	//// TODO Removed this next line once verified:
	// printf("Previous frame count was %d, current count is %d", 
	//	frame_count, result);
	//	
	//	// reset to the new # here
	//	frame_count = result;
	//}

	// Increment the count for the next in coming frame
	if (frame_count == 15)
		frame_count = 0;
	else 
		frame_count++;

	return sample_size;
}


/**
 * Reads a frame of raw data and return pointers to the decoded 16-bit integer
 * I & Q buffers. \e Each frame consists of a header, and I and Q 
 * buffers of data of length determine by the \b sample_size parameter.
 * This function also checks for the continuity of the frames coming from the
 * WSA.  Warning will be issued if the frame count (tracked local to the 
 * function) is not continuous from the previous read but will still return
 * the frame.
 *
 * @remark wsa_read_frame_int() simplily invokes wsa_read_frame_raw() follow 
 * by wsa_frame_decode() for each frame read.  However, if timing between
 * each data acquisition frames is important and needs to be minimized, 
 * it might be more advantageous to use wsa_read_frame_raw() to gather 
 * multiple of frames first and then invokes wsa_frame_decode() separately.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_frame_header structure to store 
 * information for the frame.
 * @param i_buf - A 16-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the sample_size.
 * @param q_buf - A 16-bit signed integer pointer for the unscaled 
 * Q data buffer with size specified by the sample_size.
 * @param sample_size - A 32-bit unsigned integer sample size (i.e. {I, Q} 
 * sample pairs) per data frame to be captured. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of data samples read upon success, or a negative 
 * number on error.
 */
int32_t wsa_read_frame_int(struct wsa_device *dev, struct wsa_frame_header *header, 
			int16_t *i_buf, int16_t *q_buf, const int32_t sample_size)
{	
	int32_t result = 0;
	char *dbuf;

	if ((sample_size < 128) || (sample_size > dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;
	
	// allocate the data buffer
	dbuf = (char *) malloc(sample_size * 4 * sizeof(char));

	result = wsa_read_frame_raw(dev, header, dbuf, sample_size);
	// TODO handle result < 0
	
	result = wsa_decode_frame(dbuf, i_buf, q_buf, sample_size);
	// TODO handle result < 0

	/*int i, j=0;
	for (i = 0; i < sample_size * 4; i += 4) {		
		if ((j % 4) == 0) printf("\n");
		printf("%04x,%04x ", i_buf[j], q_buf[j]);
		
		j++;
	}*/

	free(dbuf);

	return result;
}

/**
 * Decodes the raw \b data_buf buffer containing frame(s) of I & Q data bytes 
 * and returned the I and Q buffers of data with the size determined by the 
 * \b sample_size parameter.  
 *
 * Note: the \b data_buf size is assumed as \b sample_size * 4 bytes per sample
 *
 * @param data_buf - A char pointer buffer containing the raw I and Q data in
 * in bytes to be decoded into separate I and Q buffers. Its size is assumed to
 * be the number of 32-bit sample_size words multiply by 4 (i.e. 
 * sizeof(data_buf) = sample_size * 4 bytes per sample).
 * @param i_buf - A 16-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the \b sample_size.
 * @param q_buf - A 16-bit signed integer pointer for the unscaled, 
 * Q data buffer with size specified by the \b sample_size.
 * @param sample_size - A 32-bit unsigned integer number of {I, Q} 
 * sample pairs to be decoded from \b data_buf. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of samples decoded, or a 16-bit negative 
 * number on error.
 */
int32_t wsa_frame_decode(char *data_buf, int16_t *i_buf, int16_t *q_buf, 
						 const int32_t sample_size)
{
	int32_t result;
	
	// TODO need to check for the max value too? but maybe not if 
	// multiple frames allow
	if (sample_size < 128)
		return WSA_ERR_INVSAMPLESIZE;

	result = wsa_decode_frame(data_buf, i_buf, q_buf, sample_size);

	return result;
}


/**
 * Sets the number of samples per frame to be received
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param sample_size - The sample size to set.
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sample_size(struct wsa_device *dev, int32_t sample_size)
{
	int16_t result;
	char temp_str[50];

	if ((sample_size < 128) || (sample_size > dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;

	sprintf(temp_str, "TRACE:IQ:POINTS %ld\n", sample_size);

	// set the ss using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_SIZESETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_SIZESETFAILED));
		return WSA_ERR_SIZESETFAILED;
	}

	return 0;
}


/**
 * Gets the number of samples per frame.
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return The sample size if success, or a negative number on error.
 */
int32_t wsa_get_sample_size(struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	query = wsa_send_query(dev, "TRACE:IQ:POINTS?\n");

	// TODO Handle the query output here 
	if (query.result > 0) {
		//printf("Got %ld bytes: \"%s\" %ld\n", query.status, query.result, 
		//	(int32_t) atof(query.result));
		return (int32_t) atof(query.result);
	}
	else if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}

	return WSA_ERR_QUERYNORESP;
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

	query = wsa_send_query(dev, "FREQ:CENT?\n");

	// TODO Handle the query output here 
	if (query.result > 0) {
		//printf("Got %lld bytes: \"%s\" %lld\n", query.status, query.result, 
		//	(int64_t) atof(query.result));
		return (int64_t) atof(query.result);
	}
	else if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}

	return WSA_ERR_QUERYNORESP;
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
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq) // get vco vsn?
{
	int16_t result = 0;
	char temp_str[50];

	result = wsa_verify_freq(dev, cfreq);
	if (result < 0)
		return result;

	sprintf(temp_str, "FREQ:CENT %lld Hz\n", cfreq);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}

	return 0;
}


// A Local function:
// Verify if the frequency is valid (within allowed range)
int16_t wsa_verify_freq(struct wsa_device *dev, uint64_t freq)
{
	int64_t residue; 
	// verify the frequency value
	if (freq < dev->descr.min_tune_freq || freq > dev->descr.max_tune_freq)	{
		return WSA_ERR_FREQOUTOFBOUND;
	}
	
	// TODO resolution for different WSA!
	residue = freq - ((freq / dev->descr.freq_resolution) * 
		dev->descr.freq_resolution);
	if (residue > 0) {
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

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	query = wsa_send_query(dev, "INPUT:GAIN:IF?\n");

	if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}
		
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

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, "INPUT:GAIN:IF %.02f dB\n", gain);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_IFGAINSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_IFGAINSETFAILED));
		return WSA_ERR_IFGAINSETFAILED;
	}

	return 0;
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

	query = wsa_send_query(dev, "INPUT:GAIN:RF?\n");

	if (query.status <= 0) {
		printf("No query response received.\n");
		return (wsa_gain) WSA_ERR_QUERYNORESP;
	}
	
	// Convert to wsa_gain type
	if (strstr(query.result, "HIGH") != NULL) {
		gain = WSA_GAIN_HIGH;
	}
	else if (strstr(query.result, "MED") != NULL) {
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
	char temp_str[50];

	if (gain > WSA_GAIN_VLOW || gain < WSA_GAIN_HIGH)
		return WSA_ERR_INVRFGAIN;

	strcpy(temp_str, "INPUT:GAIN:RF ");
	switch(gain) {
		case(WSA_GAIN_HIGH):	strcat(temp_str, "HIGH"); break;
		case(WSA_GAIN_MEDIUM):	strcat(temp_str, "MED"); break;
		case(WSA_GAIN_LOW):		strcat(temp_str, "LOW"); break;
		case(WSA_GAIN_VLOW):	strcat(temp_str, "VLOW"); break;
		default:		strcat(temp_str, "ERROR"); break;
	}
	strcat(temp_str, "\n");

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_RFGAINSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_RFGAINSETFAILED));
		return WSA_ERR_RFGAINSETFAILED;
	}

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
	struct wsa_resp query;		// store query results

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	query = wsa_send_query(dev, "INPUT:ANTENNA?\n");

	// TODO Handle the query output here 
	if (query.status > 0)
		return atoi(query.result);
	else if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}
		
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
int16_t wsa_set_antenna(struct wsa_device *dev, int16_t port_num)
{
	int16_t result = 0;
	char temp_str[30];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (port_num < 1 || port_num > MAX_ANT_PORT)
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, "INPUT:ANTENNA %d\n", port_num);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_ANTENNASETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_ANTENNASETFAILED));
		return WSA_ERR_ANTENNASETFAILED;
	}

	return 0;
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
	int temp = 0;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	// TODO: Handle any other bits info in the OSR also... 
	// as this is a destructive read
	query = wsa_send_query(dev, "INP:FILT:PRES:STATE?\n");

	// Handle the query output here 
	if (query.status > 0) {
		temp = atoi(query.result);
		return temp;
	}
	else if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}
		
	return WSA_ERR_QUERYNORESP;
}


/**
 * Sets the RFE's preselect band pass filter (BPF) stage on or off (bypassing).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_bpf(struct wsa_device *dev, int16_t mode)
{
	int16_t result = 0;
	char temp_str[50];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (mode < 0 || mode > 1)
		return WSA_ERR_INVFILTERMODE;

	sprintf(temp_str, "INPUT:FILT:PRES:STATE %d\n", mode);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_FILTERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FILTERSETFAILED));
		return WSA_ERR_FILTERSETFAILED;
	}

	return 0;
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
//	query = wsa_send_query(dev, "INPUT:FILTER:ANTIALIAS:STATE?\n");
//
//	// TODO Handle the query output here 
//	if (query.status > 0)
//		//return atof(query.result);
//		printf("Got %lld bytes: \"%s\"\n", query.status, query.result);
//	else
//		printf("No query response received.\n");
//
//	return 0;
//}


////**
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
//	sprintf(temp_str, "INPUT:FILTER:ANTIALIAS:STATE %d\n", mode);
//
//	// set the freq using the selected connect type
//	result = wsa_send_command(dev, temp_str);
//	if (result < 0) {
//		doutf(DMED, "Error WSA_ERR_FILTERSETFAILED: %s.\n", 
//			wsa_get_error_msg(WSA_ERR_FILTERSETFAILED));
//		return WSA_ERR_FILTERSETFAILED;
//	}
//
//	return 0;
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
	int16_t temp = 0;

	// TODO: create a read OSR register w/ the bit you want to check
	// TODO: Handle any other bits info in the OSR also... 
	// as this is a destructive read
	query = wsa_send_query(dev, "STAT:OPER?\n");

	// TODO Handle the query output here 
	if (query.status > 0) {
		temp = atoi(query.result);
		return (temp & SCPI_OSR_CALI);
	}
	else if (query.status <= 0) {
		printf("No query response received.\n");
		return WSA_ERR_QUERYNORESP;
	}

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
int16_t wsa_run_cal_mode(struct wsa_device *dev, int16_t mode)
{
	int16_t result = 0;
	char temp_str[30];

	if (mode < 0 || mode > 1)
		return WSA_ERR_INVCALIBRATEMODE;

	sprintf(temp_str, "CALIBRATE:RFE:STATE %d\n", mode);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_CALIBRATESETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_CALIBRATESETFAILED));
		return WSA_ERR_CALIBRATESETFAILED;
	}

	return 0;
}
