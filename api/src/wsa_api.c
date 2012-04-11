/**
 * @mainpage Introduction
 *
 * This documentation describes in details the wsa_api library.  The wsa_api 
 * provides functions to set/get particular settings or to acquire 
 * data from the WSA.  \n
 * The wsa_api encodes the commands into SCPI syntax scripts, which 
 * are sent to a WSA through the wsa_lib library.  Subsequently, it decodes 
 * any responses or packets coming back from the WSA through the wsa_lib.
 * Thus, the API abstracts away the control protocol (such as SCPI) from 
 * the user.
 *
 * Data frames passing back from the wsa_lib are in VRT format.  This 
 * API will extract the information and the actual data frames within
 * the VRT packet and makes them available in structures and buffers 
 * for users.
 *
 *
 * @section limitation Limitations in Release v1.1
 * The following features are not yet supported with the CLI:
 *  - VRT trailer extraction. Bit fields are yet to be defined.
 *  - Data streaming. Currently only supports block mode.
 *  - DC correction.  
 *  - IQ correction.  
 *  - Automatic finding of a WSA box(s) on a network.
 *  - Triggers.
 *  - Gain calibrarion. TBD with triggers.
 *  - USB interface method.
 *
 * @section usage How to use the library
 * The wsa_api is designed using mixed C/C++ languages.  To use the 
 * library, you need to include the header file, wsa_api.h, in files that 
 * will use any of its functions to access a WSA, and a link to 
 * the wsa_api.lib.  \b wsa_api also depends on the others *.h files provided
 * in the \b include folder
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "thinkrf_stdint.h"
#include "wsa_error.h"
#include "wsa_commons.h"
#include "wsa_lib.h"
#include "wsa_api.h"


#define MAX_RETRIES_READ_FRAME 5


static int64_t _scaled_frequency = 2000000ULL;



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
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::37001" \n
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

	return result;
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
 * Verify if the given IP address or host name is valid for the WSA.  Default 
 * ports used are 37001 and 37000 for command and data ports, respectively.
 * 
 * @param ip_addr - A char pointer to the IP address or host name to be 
 * verified.
 * 
 * @return 0 if the IP is valid, or a negative number on error.
 */
int16_t wsa_check_addr(char *ip_addr) 
{
	int16_t result = 0;

	// Check with command port
	result = wsa_verify_addr(ip_addr, "37001");  //TODO make this dynamic
	if (result < 0)
		return result;

	// check with data port
	result = wsa_verify_addr(ip_addr, "37000");
	if (result < 0)
		return result;

	return 0;
}

/**
 * Verify if the given IP address or host name at the given port is valid for 
 * the WSA.
 * 
 * @param ip_addr - A char pointer to the IP address or host name to be 
 * verified.
 * @param port - A char pointer to the port to
 * 
 * @return 0 if the IP is valid, or a negative number on error.
 */
int16_t wsa_check_addrandport(char *ip_addr, char *port) 
{
	return wsa_verify_addr(ip_addr, port);
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
 * @param value - A float pointer to store the absolute maximum RF input 
 * level in dBm for the given RF gain
 *
 * @return 0 on successful or negative error number.
 */
int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
						  float *value)
{
	// TODO Check version of WSA & return the correct info here
	if (gain < WSA_GAIN_VLOW || gain > WSA_GAIN_HIGH) {		
		return WSA_ERR_INVRFGAIN;
	}
	else {
		*value = dev->descr.abs_max_amp[gain];
	}

	return 0;
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
 * @remarks This function does not set the \b sample_size to WSA at each 
 * capture in order to minimize the delay between captures.  The number of 
 * samples per frame (\b sample_size) must be set using wsa_set_sample_size()
 * at least once during the WSA powered on.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_frame_header structure to store 
 * information for the frame.
 * @param data_buf - A uint8 pointer buffer to store the raw I and Q data in
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
		*header, uint8_t* data_buf, const int32_t sample_size)
{
	int16_t result = 0;
	int16_t loop_counter = 0;
	int16_t loop_done = 0;
	int32_t samples_count = 0;
	
	
	if ((sample_size < WSA4000_MIN_SAMPLE_SIZE) || 
		(sample_size > (int32_t) dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;

	// Query WSA for data using the selected connect type
	// Allow up to 5 times of trying to get data
	while (!loop_done) {
		result = wsa_send_command(dev, "TRACE:IQ?\n");
		if (result < 0) {
			doutf(DHIGH, "Error in wsa_read_frame_raw returned by wsa_send_command: %s.\n", 
				wsa_get_error_msg(result));
			return result;
		}

		// get data & increment counters
		result = wsa_read_frame(dev, header, data_buf, sample_size, 2000);
		if (result < 0) {
			loop_counter++;
			doutf(DHIGH, "Error getting data... trying again #%d\n", loop_counter);

			if (loop_counter >= MAX_RETRIES_READ_FRAME) {
				loop_done = 1;
			}
		}
		else {
			samples_count += (int32_t) header->sample_size;
			// increment the buffer location
			if (samples_count < sample_size) {
				data_buf += header->sample_size;
				doutf(DHIGH, "%d, ", samples_count);
			}
			else { 
				loop_done = 1;
			}
		}
	}

	if (result < 0)
		return result;
	
	// TODO: Verify continuity of frame count once available in VRT

	// Increment the count for the next in coming frame
	if (frame_count == 15)
		frame_count = 0;
	else 
		frame_count++;

	return samples_count;
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
 * @remark 1. wsa_read_frame_int() simplily invokes wsa_read_frame_raw() follow 
 * by wsa_frame_decode() for each frame read.  However, if timing between
 * each data acquisition frames is important and needs to be minimized, 
 * it might be more advantageous to use wsa_read_frame_raw() to gather 
 * multiple of frames first and then invokes wsa_frame_decode() separately.
 * \n \n
 * 2. This function does not set the \b sample_size to WSA at each 
 * capture in order to minimize the delay between captures.  The number of 
 * samples per frame (\b sample_size) must be set using wsa_set_sample_size()
 * at least once during the WSA powered on.
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
	uint8_t* dbuf;

	if ((sample_size < WSA4000_MIN_SAMPLE_SIZE) || 
		(sample_size > (int32_t) dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;
	
	// allocate the data buffer
	dbuf = (uint8_t*) malloc(sample_size * 4 * sizeof(uint8_t));

	result = wsa_read_frame_raw(dev, header, dbuf, sample_size);
	if (result < 0)
		return result;
	
	result = wsa_frame_decode(dev, dbuf, i_buf, q_buf, sample_size);

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
 * @param dev - A pointer to the WSA device structure.
 * @param data_buf - A uint8 pointer buffer containing the raw I and Q data in
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
int32_t wsa_frame_decode(struct wsa_device *dev, uint8_t* data_buf, int16_t *i_buf, 
						 int16_t *q_buf, const int32_t sample_size)
{
	int32_t result;
	int64_t freq;
	uint32_t temp;
	
	// TODO need to check for the max value too? but maybe not if 
	// multiple frames allow
	if (sample_size < WSA4000_MIN_SAMPLE_SIZE)
		return WSA_ERR_INVSAMPLESIZE;
		
	// A "temporary" (hope so) fix for certain bands that required iq swapped
	result = wsa_get_freq(dev, &freq);
	if (result < 0)
		return result;

	// to avoid warnings of comparing to large #
	temp = (uint32_t) (freq / 1000);
	if ((temp < 90000) ||
		((temp >= 450000) && (temp < 4300000)) ||
		(temp >= 7450000))
		// then swap i & q
		result = wsa_decode_frame(data_buf, q_buf, i_buf, sample_size);
	else
		result = wsa_decode_frame(data_buf, i_buf, q_buf, sample_size);

	return result;
}


/**
 * Instruct the WSA to capture a block of signal data
 * and store it in internal memory.
 * \n
 * Before calling this method, set the size of the block
 * using the methods \b wsa_set_samples_per_packet
 * and \b wsa_set_packets_per_block
 * \n
 * After this method returns, read the data using
 * the method \b wsa_read_iq_packet.
 *
 * @param device - A pointer to the WSA device structure.
 *
 * @return  0 on success or a negative value on error
 */
int16_t wsa_capture_block(struct wsa_device* const device)
{
	int16_t return_status = 0;
	int64_t frequency = 0;

	// ===============================================================
	// TEMPORARY UNTIL THE SET METHODS ARE IMPLEMENTED

	return_status = wsa_send_command(device, "TRACE:BLOCK:PACKETS 1\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in wsa_capture_block: %s\n", wsa_get_error_msg(return_status));
		return return_status;
	}

	return_status = wsa_get_freq(device, &frequency);
	if (return_status < 0)
	{
		doutf(DHIGH, "Error in wsa_capture_block: wsa_get_freq returned %hd (%s)\n", 
			return_status, 
			wsa_get_error_msg(return_status));
		return return_status;
	}

	_scaled_frequency = frequency / 1000;

	// END TEMPORARY
	// ==================================================================

	return_status = wsa_send_command(device, "TRACE:BLOCK:DATA?\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in wsa_capture_block: %s\n", wsa_get_error_msg(return_status));
		return return_status;
	}

	return 0;
}


/**
 * Reads one VRT packet containing raw IQ data. 
 * Each packet consists of a header, a data payload, and a trailer.
 * The number of samples expected in the packet is indicated by
 * the \b samples_per_packet parameter.
 *
 * Note that to read a complete capture block, you must call this
 * method as many times as you set using the method \b wsa_set_packets_per_block
 *
 * Each I and Q sample is a 16-bit (2-byte) signed 2-complement integer.
 * The \b i_buffer and \b q_buffer pointers will be populated with
 * the decoded IQ payload data.
 *
 * For example, if the VRT packet contains the payload
 * @code 
 *		I1 Q1 I2 Q2 I3 Q3 I4 Q4 ... = <2 bytes I><2 bytes Q><...>
 * @endcode
 *
 * then \b i_buffer and \b q_buffer will contain:
 * @code 
 *		i_buffer[0] = I1
 *		i_buffer[1] = I2
 *		i_buffer[2] = I3
 *		i_buffer[3] = I4
 *		...
 *
 *		q_buffer[0] = Q1
 *		q_buffer[1] = Q2
 *		q_buffer[2] = Q3
 *		q_buffer[3] = Q4
 *		...
 * @endcode
 *
 * @remarks This function does not set the \b samples_per_packet on the WSA.
 * It is the caller's responsibility to configure the WSA with the correct 
 * \b samples_per_packet before initiating the capture by calling the method
 * \b wsa_set_samples_per_packet
 *
 * @param device - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_frame_header structure to store 
 *		the VRT header information
 * @param i_buffer - A 16-bit signed integer pointer for the unscaled, 
 *		I data buffer with size specified by samples_per_packet.
 * @param q_buffer - A 16-bit signed integer pointer for the unscaled 
 *		Q data buffer with size specified by samples_per_packet.
 * @param samples_per_packet - A 16-bit unsigned integer sample size (i.e. number of
 *		{I, Q} sample pairs) per VRT packet to be captured.
 *
 * @return  0 on success or a negative value on error
 */
int16_t wsa_read_iq_packet (struct wsa_device* const device, 
		struct wsa_frame_header* const header, 
		int16_t* const i_buffer, 
		int16_t* const q_buffer,
		const uint16_t samples_per_packet)
{
	uint8_t* data_buffer = 0;
	int16_t return_status = 0;

	// allocate the data buffer
	data_buffer = (uint8_t*) malloc(samples_per_packet * BYTES_PER_VRT_WORD * sizeof(uint8_t));

	return_status = wsa_read_iq_packet_raw(device, header, data_buffer, samples_per_packet);
	doutf(DMED, "In wsa_read_iq_packet: wsa_read_iq_packet_raw returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in wsa_read_iq_packet: %s\n", wsa_get_error_msg(return_status));
		free(data_buffer);
		return return_status;
	}

	// =====================================================
	// TEMPORARY FIX
	//
	// A temporary fix until IQ is swapped in FPGA

	if ((_scaled_frequency < 90000) ||
		((_scaled_frequency >= 450000) && (_scaled_frequency < 4300000)) ||
		(_scaled_frequency >= 7450000))
	{
		// then swap I & Q
		// Note: don't rely on the value of return_status
		return_status = (int16_t) wsa_decode_frame(data_buffer, q_buffer, i_buffer, samples_per_packet);
	}
	else
	{
		// NEED TO KEEP THIS LINE AFTER THE TEMPORARY FIX IS REMOVED:
		// Note: don't rely on the value of return_status
		return_status = (int16_t) wsa_decode_frame(data_buffer, i_buffer, q_buffer, samples_per_packet);
	}

	// END TEMPORARY FIX
	// ==============================================

	free(data_buffer);

	return 0;
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

	if ((sample_size < WSA4000_MIN_SAMPLE_SIZE) || 
		(sample_size > (int32_t) dev->descr.max_sample_size))
		return WSA_ERR_INVSAMPLESIZE;

	sprintf(temp_str, "TRACE:IQ:POINTS %d\n", sample_size);

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
 * @param sample_size - An integer pointer to store the sample size.
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sample_size(struct wsa_device *dev, int32_t *sample_size)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, "TRACE:IQ:POINTS?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// Verify the validity of the return value
	if (temp < WSA4000_MIN_SAMPLE_SIZE || 
		temp > (long) dev->descr.max_sample_size) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*sample_size = (int32_t) temp;

	return 0;
}


/**
 * Sets the number of samples per packet to be received
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param samples_per_packet - The sample size to set.
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_samples_per_packet(struct wsa_device *dev, uint16_t samples_per_packet)
{
	int16_t result;
	char temp_str[50];

	if ((samples_per_packet < WSA4000_MIN_SAMPLES_PER_PACKET) || 
		(samples_per_packet > WSA4000_MAX_SAMPLES_PER_PACKET))
	{
		return WSA_ERR_INVSAMPLESIZE;
	}

	sprintf(temp_str, "TRACE:SPPACKET %hu\n", samples_per_packet);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In wsa_set_samples_per_packet: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_SIZESETFAILED;
	}

	return 0;
}


/**
 * Gets the decimation rate currently set in the WSA. If rate is 0, it means
 * decimation is off (or no decimation).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param rate - A pointer to the decimation rate of integer type.
 *
 * @return The sample size if success, or a negative number on error.
 */
int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, "CALC:DEC?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// convert & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// make sure the returned value is valid
	if (((temp != 0) && (temp < dev->descr.min_decimation)) || 
		(temp > dev->descr.max_decimation)) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*rate = (int32_t) temp;

	return 0;
}


/**
 * Sets the decimation rate. 
 * @note: The rate here implies that at every given 'rate' (samples), 
 * only one sample is stored. Ex. if rate = 100, for a trace 
 * frame of 1000, only 10 samples is stored???
 *
 * Rate supported: 0, 4 - 1024. Rate of 0 is equivalent to no decimation.
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param rate - The decimation rate to set.
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate)
{
	int16_t result;
	char temp_str[50];

	// TODO get min & max rate
	if (((rate != 0) && (rate < dev->descr.min_decimation)) || 
		(rate > dev->descr.max_decimation))
		return WSA_ERR_INVDECIMATIONRATE;

	sprintf(temp_str, "CALC:DEC %d", rate);
	
	// set the rate using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_SIZESETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_SIZESETFAILED));
		return WSA_ERR_SETFAILED;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
///////////////////////////////////////////////////////////////////////////////

/**
 * Retrieves the center frequency that the WSA is running at.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cfreq - A long integer pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_freq(struct wsa_device *dev, int64_t *cfreq)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(dev, "FREQ:CENT?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq) {
		printf("Error: WSA returned %s.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*cfreq = (int64_t) temp;

	return 0;
}


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
	if (result == WSA_ERR_SETFAILED){
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		return result;

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
 * @param gain - An integer pointer to store the IF gain value.
 *
 * @return The gain value in dB, or a large negative number on error.
 */
int16_t wsa_get_gain_if(struct wsa_device *dev, int32_t *gain)
{
	struct wsa_resp query;		// store query results
	long int temp;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "INPUT:GAIN:IF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	
	// Verify the validity of the return value
	if (temp < dev->descr.min_if_gain || temp > dev->descr.max_if_gain) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*gain = (int32_t) temp;

	return 0;
}


/**
 * Sets the gain value in dB for the variable IF gain stages of the RFE, which 
 * is additive to the primary RF quantized gain stages (wsa_set_gain_rf()).
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain level in dB.
 * @remarks See the \b descr component of \b wsa_dev structure for 
 * maximum/minimum IF gain values.
 *
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Gain level out of range.
 */
int16_t wsa_set_gain_if(struct wsa_device *dev, int32_t gain)
{
	int16_t result = 0;
	char temp_str[50];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, "INPUT:GAIN:IF %d dB\n", gain);

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
 * @param gain - A \b wsa_gain pointer type to store the current RF gain value.
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_gain_rf(struct wsa_device *dev, enum wsa_gain *gain)
{
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, "INPUT:GAIN:RF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	// Convert to wsa_gain type
	if (strstr(query.output, "HIGH") != NULL) {
		*gain = WSA_GAIN_HIGH;
	}
	else if (strstr(query.output, "MED") != NULL) {
		*gain = WSA_GAIN_MED;
	}
	else if (strstr(query.output, "VLOW") != NULL) {
		*gain = WSA_GAIN_VLOW;
	}
	else if (strstr(query.output, "LOW") != NULL) {
		*gain = WSA_GAIN_LOW;
	}
	else
		*gain = (enum wsa_gain) NULL;

	return 0;
}

/**
 * Sets the quantized \b gain (sensitivity) level for the RFE of the WSA.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - The gain setting of type wsa_gain to set for WSA. \n
 * Valid gain settings are:
 * - WSA_GAIN_HIGH
 * - WSA_GAIN_MED
 * - WSA_GAIN_LOW 
 * - WSA_GAIN_VLOW
 * 
 * @return 0 on success, or a negative number on error.
  * @par Errors:
 * - Gain setting not allow.
 */
int16_t wsa_set_gain_rf (struct wsa_device *dev, enum wsa_gain gain)
{
	int16_t result = 0;
	char temp_str[50];

	if (gain > WSA_GAIN_VLOW || gain < WSA_GAIN_HIGH)
		return WSA_ERR_INVRFGAIN;

	strcpy(temp_str, "INPUT:GAIN:RF ");
	switch(gain) {
		case(WSA_GAIN_HIGH):	strcat(temp_str, "HIGH"); break;
		case(WSA_GAIN_MED):	strcat(temp_str, "MED"); break;
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
 * @param port_num - An integer pointer to store the antenna port number.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_antenna(struct wsa_device *dev, int32_t *port_num)
{
	struct wsa_resp query;		// store query results
	long temp;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "INPUT:ANTENNA?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// Verify the validity of the return value
	if (temp < 1 || temp > WSA_RFE0560_MAX_ANT_PORT) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*port_num = (int16_t) temp;

	return 0;
}


/**
 * Sets the antenna port to be used for the RFE board.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer port number to set. \n
 * Available ports: 1, 2.  Or see product datasheet for ports availability.
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_antenna(struct wsa_device *dev, int32_t port_num)
{
	int16_t result = 0;
	char temp_str[30];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (port_num < 1 || port_num > WSA_RFE0560_MAX_ANT_PORT) // TODO replace the max
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
 * @param mode - An integer pointer to store the current BPF mode: 
 * 1 = on, 0 = off.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_bpf_mode(struct wsa_device *dev, int32_t *mode)
{
	struct wsa_resp query;		// store query results
	long temp;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "INP:FILT:PRES:STATE?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	
	// Verify the validity of the return value
	if (temp < 0 || temp > 1) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}
		
	*mode = (int16_t) temp;

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
int16_t wsa_set_bpf_mode(struct wsa_device *dev, int32_t mode)
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

