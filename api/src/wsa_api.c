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
#include "wsa_client.h"


#define MAX_RETRIES_READ_FRAME 5



// ////////////////////////////////////////////////////////////////////////////
// Local functions                                                           //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_verify_freq(struct wsa_device *dev, int64_t freq);

// Verify if the frequency is valid (within allowed range)
int16_t wsa_verify_freq(struct wsa_device *dev, int64_t freq)
{
	
	// verify the frequency value
	if (freq < dev->descr.min_tune_freq || freq > dev->descr.max_tune_freq)
		return WSA_ERR_FREQOUTOFBOUND;

	return 0;
}



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
int16_t wsa_do_scpi_command_file(struct wsa_device *dev, char *file_name)
{
	return wsa_send_command_file(dev, file_name);
}

/**
 * Send a scpi command to the wsa and query for a response
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A char pointer to the scpi command
 * @param response - A char pointer to hold the response from the wsa
 *
 */
int16_t wsa_query_scpi(struct wsa_device *dev, char *command, char *response)
{
	struct wsa_resp query;		
	
	
	sprintf(command,"%s\n",command);
	
	wsa_send_query(dev, command, &query);
	strcpy(response, query.output);

	return (int16_t) query.status;
}

/**
 * Send a scpi command to the wsa
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A pointer to the scpi command
 *
 */
int16_t wsa_send_scpi(struct wsa_device *dev, char *command)
{
	int16_t result;
	sprintf(command,"%s\n",command);
	
	result = wsa_send_command(dev, command);
	
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
//int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
//						  float *value)
//{
//	// TODO Check version of WSA & return the correct info here
//	if (strcmp(gain,WSA_GAIN_VLOW_STRING) != 0 &&
//	strcmp(gain,WSA_GAIN_LOW_STRING) != 0 &&
//	strcmp(gain,WSA_GAIN_MED_STRING) != 0 &&
//	strcmp(gain,WSA_GAIN_HIGH_STRING) != 0)
//		return WSA_ERR_INVRFGAIN;	
//	
//	else 
//		*value = dev->descr.abs_max_amp[gain];
//	
//	return 0;
//}


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Request read data access from the WSA
 *
 * @param dev - A pointer to the WSA device structure
 * @param status - An int16_t pointer storing the read access request result,
 *                 1 if the access is granted, 0 if denied
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_system_request_acq_access(struct wsa_device *dev, int16_t* status)
{
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, "SYST:LOCK:REQ? ACQ\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	if (strcmp(query.output, "1") == 0)
		*status = 1;
	else if (strcmp(query.output, "0") == 0)
		*status = 0;

	return 0;
}

/**
 * Determine the current status of the WSA acquistion lock
 *
 * @param dev - A pointer to the WSA device structure
 * @param status - An int16_t pointer storing the acquisition lock status, 
 * 1 - have the acquisition access, 0 - does not have the access
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_system_acq_status(struct wsa_device *dev, int16_t *status) 
{
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, ":SYST:LOCK:HAVE? ACQ\n", &query);	
	if (query.status <= 0)
		return (int16_t) query.status;

	if (strcmp(query.output, "1") == 0)
		*status = 1;
	else if (strcmp(query.output, "0") == 0)
		*status = 0;

	return 0;
}


/**
 * Flush the current data in the WSA's internal buffer.
 *
 * @param dev - A pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_flush_data(struct wsa_device *dev) 
{
	int16_t result = 0;
	int32_t size = 0;
	char status[40];

	// check if the wsa is already sweeping
	result = wsa_get_sweep_status(dev, status);
	if (result < 0)
		return result;

	if (strcmp(status, WSA4000_SWEEP_STATE_RUNNING) == 0) 
		return WSA_ERR_SWEEPALREADYRUNNING;

	result = wsa_send_command(dev, "SYSTEM:FLUSH\n");
	doutf(DHIGH, "In wsa_flush_data: %d - %s.\n", result, wsa_get_error_msg(result));
	
	return result;
}


/**
 * Flush the data in the wsa and clean the remaining data in
 * the data socket socket
 *
 * @param dev - A pointer to the WSA device structure.
 *
 */
int16_t wsa_clean_data_socket(struct wsa_device *dev)
{
	int16_t result = 0;
	int32_t bytes_received = 0;
	uint8_t *packet;
    int32_t packet_size = WSA4000_MAX_CAPTURE_BLOCK;
	uint32_t timeout = 360;
    clock_t start_time;
    clock_t end_time;
	
	start_time = clock();
	end_time = 1000 + start_time;

	packet = (uint8_t *) malloc(packet_size * sizeof(uint8_t));
	if (packet == NULL)
	{
		doutf(DHIGH, "In wsa_clean_data_socket: failed to allocate memory\n");
		return WSA_ERR_MALLOCFAILED;
	}
	// read the left over packets from the socket
	while(clock() <= end_time) 
	{
		result = wsa_sock_recv_data(dev->sock.data, 
									packet, 
									packet_size, 
									timeout,	
									&bytes_received);
	}

	free(packet);

	return 0;
}

/**
 * Abort the current data capturing process and put the WSA into the manual mode
 * (i.e. no sweep or triggering or streaming)
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_system_abort_capture(struct wsa_device *dev)
{
	int16_t result = 0;
	int32_t size = 0;

	result = wsa_send_command(dev, "SYSTEM:ABORT\n");
	doutf(DHIGH, "In wsa_system_abort_capture: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Returns the WSA's current capture mode 
 * \n
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An char containing the wsa's capture mode.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_get_capture_mode(struct wsa_device * const dev, char *mode)
{
	struct wsa_resp query;
	wsa_send_query(dev, "SYST:CAPT:MODE?\n", &query);
	
	if (strcmp(query.output, WSA4000_BLOCK_CAPTURE_MODE) == 0 || 
		strcmp(query.output, WSA4000_STREAM_CAPTURE_MODE) == 0 ||
		strcmp(query.output, WSA4000_SWEEP_CAPTURE_MODE) == 0)
		strcpy(mode, query.output);
	
	else
		return WSA_ERR_RESPUNKNOWN;

	return 0;
}

/**
 * Aborts the current data capturing process (sweep mode/stream mode) and puts the WSA system into
 * capture block mode
 * \n
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_abort_capture(struct wsa_device * const dev)
{
	int16_t result = 0;

	result = wsa_send_command(dev, "SYSTEM:ABORT\n");
	doutf(DHIGH, "Error in wsa_abort_capture: %d - %s\n", result, wsa_get_error_msg(result));

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
 * the method \b wsa_read_vrt_packet.
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_capture_block(struct wsa_device * const dev)
{
	int16_t result = 0;

	result = wsa_send_command(dev, "TRACE:BLOCK:DATA?\n");
	doutf(DHIGH, "Error in wsa_capture_block: %d - %s\n", result, wsa_get_error_msg(result));

	return result;
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
 * @param dev - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_vrt_packet_header structure to store 
 *		the VRT header information
 * @param trailer - A pointer to \b wsa_vrt_packet_trailer structure to store 
 *		the VRT trailer information
 * @param receiver - A point to \b wsa_reciever packet structure to store the
 *      VRT receiver context information
 * @param digitizer - A point to \b wsa_digitizer packet structure to store the
 *      VRT digitizer context information
 * @param extension - a pointer to \b wsa_extension_packet strucuture to store
 *		the custom Context data
 * @param i_buffer - A 16-bit signed integer pointer for the unscaled, 
 *		I data buffer with size specified by samples_per_packet.
 * @param q_buffer - A 16-bit signed integer pointer for the unscaled 
 *		Q data buffer with size specified by samples_per_packet.
 * @param samples_per_packet - A 16-bit unsigned integer sample size (i.e. number of
 *		{I, Q} sample pairs) per VRT packet to be captured.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_read_vrt_packet (struct wsa_device * const dev, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet * const digitizer,
		struct wsa_extension_packet * const sweep_info,
		int16_t * const i_buffer, 
		int16_t * const q_buffer,
		int32_t samples_per_packet)		
{
	uint8_t *data_buffer;
	int16_t result = 0;
	int64_t frequency = 0;

	// allocate the data buffer
	data_buffer = (uint8_t *) malloc(samples_per_packet * BYTES_PER_VRT_WORD * sizeof(uint8_t));
	if (data_buffer == NULL)
	{
		doutf(DHIGH, "In wsa_read_vrt_packet: failed to allocate memory\n");
		return WSA_ERR_MALLOCFAILED;
	}
			
	result = wsa_read_vrt_packet_raw(dev, header, trailer, receiver, digitizer, sweep_info, data_buffer);
	doutf(DMED, "wsa_read_vrt_packet_raw returned %hd\n", result);
	if (result < 0)
	{
		doutf(DHIGH, "Error in wsa_read_vrt_packet: %s\n", wsa_get_error_msg(result));
		if (result == WSA_ERR_NOTIQFRAME)
			wsa_system_abort_capture(dev);

		free(data_buffer);
		return result;
	} 
	
	if (header->stream_id == IF_DATA_STREAM_ID) 
	{
		// Note: don't rely on the value of result
		result = (int16_t) wsa_decode_frame(data_buffer, i_buffer, q_buffer, samples_per_packet);
	}
	
	free(data_buffer);

	return 0;
}


/**
 * Set the number of samples per packet for the manual capture block
 * 
 * @param dev - A pointer to the WSA device structure
 * @param samples_per_packet - An integer to store the sample size to be set
 *
 * @return 0 if success, or a negative number on error
 */
int16_t wsa_set_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];

	if ((samples_per_packet < WSA4000_MIN_SPP) || 
		(samples_per_packet > WSA4000_MAX_SPP) || ((samples_per_packet % WSA4000_SPP_MULTIPLE) != 0))
		return WSA_ERR_INVSAMPLESIZE;
	
	sprintf(temp_str, "TRACE:SPPACKET %hu\n", samples_per_packet);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_samples_per_packet: %d - %s.\n", result, wsa_get_error_msg(result));
		
	return result;
}


/**
 * Gets the number of samples per packet that will be returned in each
 * VRT packet when \b wsa_read_vrt_packet is called
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param samples_per_packet - A int32_t pointer to store the samples per packet
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet)
{
	struct wsa_resp query;		// store query results
	long temp;
	wsa_send_query(dev, "TRACE:SPPACKET?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA4000_MIN_SPP) || 
		(temp > WSA4000_MAX_SPP))
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*samples_per_packet = (int32_t) temp;

	return 0;
}

/**
 * Sets the number of VRT packets per each capture block.
 * The number of samples in each packet is set by
 * the method wsa_set_samples_per_packet.
 * After capturing the block with the method wsa_capture_block,
 * read back the data by calling wsa_read_vrt_packet
 * \b packets_per_block number of times
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - number of packets
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_packets_per_block(struct wsa_device *dev, int32_t packets_per_block)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];
	
	if (packets_per_block < WSA4000_MIN_PPB)
		return WSA_ERR_INVNUMBER;
	else if (packets_per_block > WSA4000_MAX_PPB) 
		return WSA_ERR_INVCAPTURESIZE;

	sprintf(temp_str, "TRACE:BLOCK:PACKETS %u\n", packets_per_block);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_packets_per_block: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Gets the number of VRT packets to be captured
 * when \b wsa_capture_block is called
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - A uint32_t pointer to store the number of packets
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, "TRACE:BLOCK:PACKETS?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*packets_per_block = (int32_t) temp;

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

	wsa_send_query(dev, ":SENSE:DEC?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// convert & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// make sure the returned value is valid
	if (((temp != 1) && (temp < dev->descr.min_decimation)) || 
		(temp > dev->descr.max_decimation)) 
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*rate = (int32_t) temp;

	return 0;
}


/**
 * Set the decimation rate. 
 * Rate supported: 0, 4 - 1023. Rate of 0 is equivalent to no decimation.
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param rate - An integer number storing the decimation rate to be set
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];

	// TODO get min & max rate
	if (((rate != 1) && (rate < dev->descr.min_decimation)) || 
		(rate > dev->descr.max_decimation))
		return WSA_ERR_INVDECIMATIONRATE;

	sprintf(temp_str, "SENSE:DEC %d \n", rate);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_decimation: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


///////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
///////////////////////////////////////////////////////////////////////////////

/**
 * Retrieve the center frequency that the WSA is running at.
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
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq) 
	{
		printf("Error: WSA returned '%s'.\n", query.output);
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
 * @param dev - A pointer to the WSA device structure
 * @param cfreq - An int64_t type storing the center frequency to set, in Hz
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Frequency out of range.
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	result = wsa_verify_freq(dev, cfreq);
	if (result < 0)
		return result;

	sprintf(temp_str, "FREQ:CENT %lld Hz\n", cfreq);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_freq: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Retrieves the frequency shift value
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fshift - A float pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_freq_shift(struct wsa_device *dev, float *fshift)
{
	struct wsa_resp query;		// store query results
	double temp;
	double range = (double) dev->descr.inst_bw;

	wsa_send_query(dev, "FREQ:SHIFT?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}


	if (temp < -range || temp > range) 
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*fshift = (float) temp;

	return 0;
}


/**
 * Sets frequency shift value
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param fshift - The frequency shift value to set, in Hz
 *
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Frequency out of range.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_freq_shift(struct wsa_device *dev, float fshift)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	int64_t range = dev->descr.inst_bw;

	// verify the value bwn -125 to 125MHz, "inclusive"
	if (fshift < (-range) || fshift > range)
		return WSA_ERR_FREQOUTOFBOUND;

	sprintf(temp_str, "FREQ:SHIFt %f Hz\n", fshift);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_freq_shift: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
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
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (((int32_t) temp < dev->descr.min_if_gain) || 
		((int32_t) temp > dev->descr.max_if_gain))
	{
		printf("Error: WSA returned '%ld'.\n", temp);
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
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, "INPUT:GAIN:IF %d dB\n", gain);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_gain_if: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Gets the current quantized RF front end gain setting of the RFE.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - A char pointer to store the current RF gain setting.
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_gain_rf(struct wsa_device *dev, char *gain)
{
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, "INPUT:GAIN:RF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(gain,query.output);

	if (strcmp(gain, WSA_GAIN_VLOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_LOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_MED_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_HIGH_STRING) != 0)
		return WSA_ERR_INVRFGAIN;

	return 0;
}

/**
 * Sets the quantized \b gain (sensitivity) level for the RFE of the WSA. \n
 * Valid RF gain settings are HIGH, MEDium, LOW, and VLOW
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - A char pointer containing the rf gain setting\n
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_gain_rf(struct wsa_device *dev, char *gain)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(gain, WSA_GAIN_VLOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_LOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_MED_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_HIGH_STRING) != 0)
		return WSA_ERR_INVRFGAIN;

	sprintf(temp_str, "INPUT:GAIN:RF %s\n", gain);

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_gain_rf: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
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
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if (temp < 1 || temp > WSA_RFE0560_MAX_ANT_PORT) 
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*port_num = (int32_t) temp;

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
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (port_num < 1 || port_num > WSA_RFE0560_MAX_ANT_PORT) // TODO replace the max
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, "INPUT:ANTENNA %d\n", port_num);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_antenna: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
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

	wsa_send_query(dev, "INP:FILT:PRES?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (temp < 0 || temp > 1) 
	{
		printf("Error: WSA returned '%ld'.\n", temp);
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
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (mode < 0 || mode > 1)
		return WSA_ERR_INVFILTERMODE;

	sprintf(temp_str, "INPUT:FILT:PRES %d\n", mode);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_bpf_mode: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


// ////////////////////////////////////////////////////////////////////////////
// DEVICE SETTINGS					                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_fw_ver(struct wsa_device *dev, char *fw_ver)
{
	struct wsa_resp query;		// store query results
	char *strtok_result;
	int16_t i = 0;
	
	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;
	
	wsa_send_query(dev, "*IDN?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	strtok_result = strtok(query.output, ",");
	strtok_result = strtok(NULL, ",");
	strtok_result = strtok(NULL, ",");
    strtok_result = strtok(NULL, ",");

	strcpy(fw_ver,strtok_result);

	return 0;
}



// ////////////////////////////////////////////////////////////////////////////
// TRIGGER CONTROL SECTION                                                   //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Sets the WSA to use basic a level trigger
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param start_freq - The lowest frequency at which a signal should be detected
 * @param stop_freq - The highest frequency at which a signal should be detected
 * @param amplitude - The minimum amplitutde of a signal that will satisfy the trigger
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	result = wsa_verify_freq(dev, start_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STARTOOB;
	else if (result < 0)
		return result;

	result = wsa_verify_freq(dev, stop_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STOPOOB;
	else if (result < 0)
		return result;

	sprintf(temp_str, ":TRIG:LEVEL %lld,%lld,%ld\n", start_freq, stop_freq, amplitude);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_trigger_level: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Retrieves the basic level trigger settings
 *
 * @param dev - A pointer to the WSA device structure.
 * @param start_freq - A long integer pointer to store the start frequency in Hz.
 * @param stop_freq - A long integer pointer to store the stop frequency in Hz.
 * @param amplitude - A long integer pointer to store the signal amplitude in dBm.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude)
{
	struct wsa_resp query;		// store query results
	double temp;
	char *strtok_result;

	wsa_send_query(dev, ":TRIG:LEVEL?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	// Convert the 1st number & make sure no error
	strtok_result = strtok(query.output, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*start_freq = (int64_t) temp;
	
	// Convert the 2nd number & make sure no error
	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq) 
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*stop_freq = (int64_t) temp;
	
	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*amplitude = (int32_t) temp;

	return 0;
}


/**
 * Set the current trigger mode of the WSA
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param trigger_type - Trigger mode of selection.
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_type(struct wsa_device *dev, char *trigger_type)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if((strcmp(trigger_type, WSA4000_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA4000_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA4000_PULSE_TRIGGER_TYPE) == 0))
		
		sprintf(temp_str, "TRIGGER:TYPE %s \n", trigger_type);
	else
		return WSA_ERR_INVTRIGGERMODE;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_trigger_type: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Gets the WSA's capture mode to triggered (trigger on)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param type - A char containing the trigger mode. 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_type(struct wsa_device *dev, char *type)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "TRIGGER:TYPE?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	if((strcmp(query.output, WSA4000_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA4000_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA4000_PULSE_TRIGGER_TYPE) == 0))
		strcpy(type, query.output);
	else
		return WSA_ERR_INVTRIGGERMODE;

	return 0;
}


/**
 * Set the WSA's the delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger delay (in nanoseconds, must be multiple of 8) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_delay(struct wsa_device *dev, int32_t delay)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (delay > WSA4000_TRIGGER_DELAY_MIN && 
		delay < WSA4000_TRIGGER_DELAY_MAX && 
		delay % WSA4000_TRIGGER_DELAY_MULTIPLE == 0)
		
		sprintf(temp_str, "TRIGGER:DELAY %d \n", delay);
	
	else
		return WSA_ERR_INVTRIGGERDELAY;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_trigger_delay: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Retrieve the WSA's the delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger delay (in nanoseconds)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_delay(struct wsa_device *dev, int32_t *delay)
{
	struct wsa_resp query;
	long temp;
	wsa_send_query(dev, "TRIGGER:DELAY?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if (temp < WSA4000_TRIGGER_DELAY_MIN || 
		temp > WSA4000_TRIGGER_DELAY_MAX || 
		temp % WSA4000_TRIGGER_DELAY_MULTIPLE != 0)
		return WSA_ERR_INVTRIGGERDELAY;
	else
		*delay = (int32_t) temp;
		return 0;

}


/**
 * Set the WSA's current synchronization state
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_mode - The synchronization mode (master/slave) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_sync(struct wsa_device *dev, char *sync_mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (strcmp(sync_mode, WSA4000_MASTER_TRIGGER) == 0 || 
		strcmp(sync_mode,  WSA4000_SLAVE_TRIGGER) == 0) 
		
		sprintf(temp_str, "TRIGGER:SYNC %s \n", sync_mode);
	
	else
		return WSA_ERR_INVTRIGGERSYNC;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_trigger_sync: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Retrieve the WSA's current synchronization state
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_mode - The  trigger synchronization mode (master/slave)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_sync(struct wsa_device *dev, char *sync_mode)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "TRIGGER:SYNC?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	strcpy(sync_mode, query.output);
	
	if (strcmp(sync_mode, WSA4000_MASTER_TRIGGER) != 0 && 
		strcmp(sync_mode,  WSA4000_SLAVE_TRIGGER) != 0) 

		return WSA_ERR_INVTRIGGERSYNC;
	else
		return 0;

}


//////////////////////////////////////////////////////////////////////////////
// PLL REFERENCE CONTROL SECTION                                            //
//////////////////////////////////////////////////////////////////////////////

/**
 * Gets the PLL Reference Source
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param pll_ref - An integer pointer to store the current PLL Source
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_reference_pll(struct wsa_device *dev, char *pll_ref)
{
	struct wsa_resp query;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0560) != 0)
	    return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "SOURCE:REFERENCE:PLL?\n", &query);	
	if (query.status <= 0)
		return (int16_t) query.status;

	strcpy(pll_ref, query.output);

	return 0;
}

/**
 * Set the Reference PLL source to either INTernal or EXTernal
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param pll_ref - An integer used to store the value of the reference source to be set
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_reference_pll(struct wsa_device *dev, char *pll_ref)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(pll_ref, "INT") == 0 || strcmp(pll_ref, "EXT") == 0)
		sprintf(temp_str, "SOURCE:REFERENCE:PLL %s\n", pll_ref); 
	else
		return WSA_ERR_INVPLLREFSOURCE;
	
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_reference_pll: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Reset the Reference PLL source to internal type
 *
 * @param dev - A pointer to the WSA device structure
 * 
 * @return 0 upon successful or negative value if errorred
 */
int16_t wsa_reset_reference_pll(struct wsa_device *dev)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	sprintf(temp_str, "SOURCE:REFERENCE:PLL:RESET\n");
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_reset_reference_pll: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * get the Reference PLL lock status of the digital card
 *
 * @param dev - A pointer to the WSA device structure.
 * @param lock_ref - An integer pointer to store the lock status
 *
 * @return 1 if locked, 0 if unlocked, else a negative value
 */
int16_t wsa_get_lock_ref_pll(struct wsa_device *dev, int32_t *lock_ref)
{
	struct wsa_resp query;
	int16_t result = 0;
	double temp;

	wsa_send_query(dev, "LOCK:REFerence?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*lock_ref = (int32_t) temp;

	return 0;
}

/**
 * Get the RFE's PLL lock status
 *
 * @param dev - A pointer to the WSA device structure.
 * @param lock_rf - An integer pointer to store the RF lock status
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_lock_rf(struct wsa_device *dev, int32_t *lock_rf)
{
    struct wsa_resp query;
    int16_t result = 0;
    double temp;

    wsa_send_query(dev, "LOCK:RF?\n", &query);
    if (query.status <= 0)
        return (int16_t) query.status;

    if (to_double(query.output, &temp) < 0)
    {
        printf("Error: WSA returned '%ld'.\n", temp);
        return WSA_ERR_RESPUNKNOWN;
    }

    *lock_rf = (int32_t) temp;

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// STREAM CONTROL SECTION                                                    //
///////////////////////////////////////////////////////////////////////////////

/**
 * Initiates the capture, storage and streaming of IQ data in the WSA
 * \n
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_stream_start(struct wsa_device * const dev)
{
	int16_t result = 0;
	char capture_mode[MAX_STR_LEN];
	
	// retrieve wsa capture mode
	result = wsa_get_capture_mode(dev, capture_mode);
	if (result < 0)
		return result;
	
	if (strcmp(capture_mode, WSA4000_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMALREADYRUNNING;

	else if (strcmp(capture_mode, WSA4000_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMWHILESWEEPING;

	result = wsa_send_command(dev, "TRACE:STREAM:START\n");
	doutf(DHIGH, "Error in wsa_stream_start: %d - %s\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Initiates the capture, storage and streaming of IQ data in the WSA
 * \n
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_stream_start_id(struct wsa_device * const dev, int64_t stream_start_id)
{
	int16_t result = 0;
	char capture_mode[MAX_STR_LEN];
	char temp_str[MAX_STR_LEN];
	// retrieve wsa capture mode
	result = wsa_get_capture_mode(dev, capture_mode);
	if (result < 0)
		return result;
	
	if (strcmp(capture_mode, WSA4000_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMALREADYRUNNING;

	else if (strcmp(capture_mode, WSA4000_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMWHILESWEEPING;
	
	if (stream_start_id < 0 || stream_start_id > UINT_MAX)
		return WSA_ERR_INVSTREAMSTARTID;

	sprintf(temp_str, "TRACE:STREAM:START %lld \n", stream_start_id);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "Error in wsa_stream_start_id: %d - %s\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * stops stream mode in the WSA, and read remaining data in the socket.
 * \n
 * 
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_stream_stop(struct wsa_device * const dev)
{
    int16_t result = 0;

	char status[MAX_STR_LEN];

	result = wsa_get_capture_mode(dev, status);
	if (result < 0)
		return result;

	// check if the wsa is already sweeping
	if (strcmp(status, WSA4000_STREAM_CAPTURE_MODE) != 0)
		return WSA_ERR_STREAMNOTRUNNING;

	result = wsa_send_command(dev, "TRACE:STREAM:STOP\n");
	if (result < 0)
	{
		doutf(DHIGH, "Error in wsa_stream_stop: %d - %s\n", result, wsa_get_error_msg(result));
		return result;
	}

	printf("Clearing socket buffer... ");
	
	//flush remaining data in the wsa
	result = wsa_flush_data(dev); 
	if (result < 0)
		return result;
	
	// clean remaining data in the data socket socket
	result = wsa_clean_data_socket(dev);
	if (result < 0)
		return result;

	printf("done.\n");
	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// SWEEP CONTROL SECTION                                                     //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Get the antenna port value currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer pointer to store the antenna port value
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_antenna(struct wsa_device *dev, int32_t *port_num) 
{
	struct wsa_resp query;		// store query results
	long temp;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "SWEEP:ENTRY:ANTENNA?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if (temp < 1 || temp > WSA_RFE0560_MAX_ANT_PORT) 
	{
		printf("Error: WSA returned '%ld'.\n", temp); 
		return WSA_ERR_RESPUNKNOWN;
	}

	*port_num = (int32_t) temp;

	return 0;
}


/**
 * Set the antenna port of the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param port_num - An integer storing the antenna port value to be set
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_antenna(struct wsa_device *dev, int32_t port_num) 
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (port_num < 1 || port_num > WSA_RFE0560_MAX_ANT_PORT) // TODO replace the max
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, "SWEEP:ENTRY:ANTENNA %d\n", port_num);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_antenna: %d - %s.\n", result, wsa_get_error_msg(result));
		
	return result;
}


/**
 * Get the IF gain currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param gain - An integer to store the IF gain value
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_gain_if(struct wsa_device *dev, int32_t *gain)
{
	struct wsa_resp query;		// store query results
	long int temp;

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	wsa_send_query(dev, "SWEEP:ENTRY:GAIN:IF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (temp < dev->descr.min_if_gain || temp > dev->descr.max_if_gain) 
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*gain = (int32_t) temp;

	return 0;
}


/**
 * Set the IF gain to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param gain - An integer to store the IF gain value
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_set_sweep_gain_if(struct wsa_device *dev, int32_t gain)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, "SWEEP:ENTRY:GAIN:IF %d\n", gain);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_gain_if: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Get the RF gain currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - An enum wsa_gain pointer to store the gain value
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_gain_rf(struct wsa_device *dev, char *gain)
{	
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, "SWEEP:ENTRY:GAIN:RF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	strcpy(gain,query.output);
	if (strcmp(gain,WSA_GAIN_VLOW_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_LOW_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_MED_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_HIGH_STRING) != 0)
	return WSA_ERR_INVRFGAIN;


	return 0;
}

/**
 * Set the RF gain to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - A char pointer containing the rf gain setting to set for WSA. \n
 * Valid gain settings are:
 * - 'HIGH'
 * - 'MED'
 * - 'LOW' 
 * - 'VLOW'
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_gain_rf(struct wsa_device *dev, char *gain)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(gain,WSA_GAIN_VLOW_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_LOW_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_MED_STRING) != 0 &&
		strcmp(gain,WSA_GAIN_HIGH_STRING) != 0)
		return WSA_ERR_INVRFGAIN;


	sprintf(temp_str, "SWEEP:ENTRY:GAIN:RF %s \n", gain);


	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_gain_rf: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Gets the number of samples per packet currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param samples_per_packet - An integer pointer to store the samples per packet value
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sweep_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, "SWEEP:ENTRY:SPPACKET?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA4000_MIN_SPP) || 
		(temp > WSA4000_MAX_SPP))
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*samples_per_packet = (uint32_t) temp;

	return 0;
}

/**
 * Set the number of samples per packet to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param samples_per_packet - An integer value storing the sample size to be set
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sweep_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];

	if ((samples_per_packet < WSA4000_MIN_SPP) || 
		(samples_per_packet > WSA4000_MAX_SPP) || 
		((samples_per_packet % WSA4000_SPP_MULTIPLE) != 0))
		return WSA_ERR_INVSAMPLESIZE;

	sprintf(temp_str, "SWEEP:ENTRY:SPPACKET %hu\n", samples_per_packet);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_samples_per_packet: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Get the packets per block currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - A uint32_t pointer to store the number of packets per block
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sweep_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, "SWEEP:ENTRY:PPBLOCK?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*packets_per_block = (int32_t) temp;

	return 0;
}

/**
 * Set the number of packets per block to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - number of packets
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sweep_packets_per_block(struct wsa_device *dev, int32_t packets_per_block)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];

	if (packets_per_block < WSA4000_MIN_PPB)
		return WSA_ERR_INVNUMBER;
	else if (packets_per_block > WSA4000_MAX_PPB) 
		return WSA_ERR_INVCAPTURESIZE;

	sprintf(temp_str, "SWEEP:ENTRY:PPBLOCK %d\n", packets_per_block);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_packets_per_block: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Get the decimation rate  currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param rate - An integer pointer to the decimation rate of integer type.
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sweep_decimation(struct wsa_device *dev, int32_t *rate)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, ":SWEEP:ENTRY:DECIMATION?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// convert & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// make sure the returned value is valid
	if (((temp != 1) && (temp < dev->descr.min_decimation)) || 
		(temp > dev->descr.max_decimation))
	{
		printf("Error: WSA returned '%ld'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}
	*rate = (int32_t) temp;

	return 0;
}

/**
 * Set the decimation rate from the user's sweep list. 
 * Rate supported: 0, 4 - 1024. Rate of 0 is equivalent to no decimation.
 *
 * @param dev - A pointer to the WSA device structure
 * @param rate - An integer value storing the rate to be set
 *
 * @return 0 if success, or a negative number on error
 */
int16_t wsa_set_sweep_decimation(struct wsa_device *dev, int32_t rate)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];

	// TODO get min & max rate
	if (((rate != 1) && (rate < dev->descr.min_decimation)) || 
		(rate > dev->descr.max_decimation))
		return WSA_ERR_INVDECIMATIONRATE;

	sprintf(temp_str, ":SWEEP:ENTRY:DECIMATION %d\n", rate);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_decimation: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Retrieve the sweep frequency range currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param start_freq - A long integer pointer to store the start frequency in Hz
 * @param stop_freq - A long integer pointer to store the stop frequency in Hz
 *
 * @return 0 on successful or a negative number on error
 */
int16_t wsa_get_sweep_freq(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq)
{
	struct wsa_resp query;	// store query results
	double temp;
	char *strtok_result;

	wsa_send_query(dev, "SWEEP:ENTRY:FREQ:CENTER?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	strtok_result = strtok(query.output, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*start_freq = (int64_t) temp;
	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*stop_freq = (int64_t) temp;

	return 0;
}
/**
 * Set the center frequency to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param start_freq - an int64_t value storing the start center frequency to be set, in Hz
 * @param stop_freq - an int64_t value storing the stop center frequency to be set, in Hz
 *
 * @return 0 on success, or a negative number on error
 * @par Errors:
 * - Frequency out of range.
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_sweep_freq(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	// verify start freq value
	result = wsa_verify_freq(dev, start_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STARTOOB;
	else if (result < 0)
		return result;

	// verify start freq value
	result = wsa_verify_freq(dev, stop_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STOPOOB;
	else if (result < 0)
		return result;
		
	// make sure stop_freq is larger than start
	if (stop_freq <= start_freq)
		return  WSA_ERR_INVSTOPFREQ;
		
	sprintf(temp_str, "SWEEP:ENTRY:FREQ:CENT %lld Hz, %lld Hz\n", start_freq, stop_freq);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_freq: %d - %s.\n", result, wsa_get_error_msg(result));
		
	return result;
}

/**
 * Retrieve the frequency shift value currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param fshift - A float pointer to store the frequency shift value in Hz
 *
 * @return 0 on successful or a negative number on error
 */
int16_t wsa_get_sweep_freq_shift(struct wsa_device *dev, float *fshift)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(dev, "SWEEP:ENTRY:FREQ:SHIFT?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*fshift = (float) temp;

	return 0;
}
/**
 * Sets the frequency shift value currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param fshift - A float value to store the frequency shift value to be set in Hz
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_set_sweep_freq_shift(struct wsa_device *dev, float fshift)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	int64_t range = dev->descr.inst_bw;

	// verify the value bwn -125 to 125MHz, "inclusive"
	if (fshift < (-range) || fshift > range)
		return WSA_ERR_FREQOUTOFBOUND;

	sprintf(temp_str, "SWEEP:ENTRY:FREQ:SHIFt %f Hz\n", fshift);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_freq_shift: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Set the sweep frequency step size to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param step - An int64_t value storing the frequency step value in Hz to be set
 *
 * @return 0 on successful or a negative number on error
 */
int16_t wsa_set_sweep_freq_step(struct wsa_device *dev, int64_t step)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	result = wsa_verify_freq(dev, step);
	if (result < 0)
		return result;

	sprintf(temp_str, "SWEEP:ENTRY:FREQ:STEP %lld Hz\n", step);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_freq_step: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Retrieve the sweep frequency step currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fstep - A int64_t pointer to store the sweep frequency step in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_freq_step(struct wsa_device *dev, int64_t *fstep)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(dev, "SWEEP:ENTRY:FREQ:STEP?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*fstep = (int64_t) temp;

	return 0;
}

/**
 * Set the dwell time to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param seconds - An integer to store the seconds value.
 * @param microseconds - An integer to store the microseconds value.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_set_sweep_dwell(struct wsa_device *dev, int32_t seconds, int32_t microseconds)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if ((seconds < 0) || (microseconds < 0))
	return WSA_ERR_INVDWELL;

	sprintf(temp_str, "SWEEP:ENTRY:DWELL %u,%u\n", seconds, microseconds);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_dwell: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Retrieve the dwell time in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param seconds - An integer pointer to store the seconds value.
 * @param microseconds - An integer pointer to store the microseconds value.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_dwell(struct wsa_device *dev, int32_t *seconds, int32_t *microseconds)
{
	struct wsa_resp query;		// store query results
	double temp = 5;
	char *strtok_result;

	wsa_send_query(dev, "SWEEP:ENTRY:DWELL?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the 1st number & make sure no error
	strtok_result = strtok(query.output, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*seconds = (int32_t) temp;
	
	// Convert the 2nd number & make sure no error
	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*microseconds = (int32_t) temp;
	
	return 0;
}


/**
 * Set up the level trigger values to the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param start_freq - The lower bound frequency range in Hz
 * @param stop_freq - The upper bound frequency range in Hz
 * @param amplitude - The minimum amplitude threshold of a signal that a
 *        trigger to occur, in dBm
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	result = wsa_verify_freq(dev, start_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STARTOOB;

	result = wsa_verify_freq(dev, stop_freq);

	if (result == WSA_ERR_FREQOUTOFBOUND)
		return WSA_ERR_STOPOOB;

	if (stop_freq <= start_freq)
		return WSA_ERR_INVSTOPFREQ;

	sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:LEVEL %lld,%lld,%ld\n", start_freq, stop_freq, amplitude);
	result = wsa_send_command(dev, temp_str);

	doutf(DHIGH, "In wsa_set_sweep_trigger_level: %d - %s.\n", result, wsa_get_error_msg(result));
	return result;
}


/**
 * Retrieve the level trigger settings currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure.
 * @param start_freq - A long integer pointer to store the start frequency in Hz.
 * @param stop_freq - A long integer pointer to store the stop frequency in Hz.
 * @param amplitude - A long integer pointer to store the signal amplitude in dBm.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude)
{
	struct wsa_resp query;		// store query results
	double temp;
	char *strtok_result;
	
	wsa_send_query(dev, "SWEEP:ENTRY:TRIGGER:LEVEL?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the 1st number & make sure no error
	strtok_result = strtok(query.output, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*start_freq = (int64_t) temp;

	// Convert the 2nd number & make sure no error
	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*stop_freq = (int64_t) temp;

	// Convert the 3rd number & make sure no error
	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}	
	*amplitude = (int32_t) temp;

	return 0;
}


/**
 * Set the sweep trigger type to NONE, LEVEL or PULSE.  Setting NONE is equivalent to
 * disabling the trigger. Default NONE.
 *
 * @param dev - A pointer to the WSA device structure
 * @param trigger_type - A char pointer to the trigger type to be set. 
 *        Currently support: NONE, LEVEL or PULSE
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_type(struct wsa_device *dev, char *trigger_type)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if((strcmp(trigger_type, WSA4000_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA4000_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA4000_PULSE_TRIGGER_TYPE) == 0))
		sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:TYPE %s \n", trigger_type);
	else
		return WSA_ERR_INVTRIGGERMODE;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_trigger_type: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}


/**
 * Retrieve the trigger type currently set in the sweep entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param type - A char pointer to the trigger type to be returned
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_get_sweep_trigger_type(struct wsa_device *dev, char *trigger_type)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "SWEEP:ENTRY:TRIGGER:TYPE?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	if((strcmp(query.output, WSA4000_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA4000_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA4000_PULSE_TRIGGER_TYPE) == 0))
		strcpy(trigger_type,query.output);
	
	else
		return WSA_ERR_INVTRIGGERMODE;
	return 0;
}

/**
 * Set the sweep entry's the delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger delay (in nanoseconds, must be multiple of 8) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_delay(struct wsa_device *dev, int32_t delay)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (delay > WSA4000_TRIGGER_DELAY_MIN && 
		delay < WSA4000_TRIGGER_DELAY_MAX && 
		delay % WSA4000_TRIGGER_DELAY_MULTIPLE == 0)
		
		sprintf(temp_str, "SWEEP:LIST:TRIGGER:DELAY %d \n", delay);
	
	else
		return WSA_ERR_INVTRIGGERDELAY;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_sweep_trigger_delay: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Retrieve sweep entry's delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger delay (in nanoseconds)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_delay(struct wsa_device *dev, int32_t *delay)
{
	struct wsa_resp query;
	long temp;
	wsa_send_query(dev, "SWEEP:LIST:TRIGGER:DELAY?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if (temp < WSA4000_TRIGGER_DELAY_MIN || 
		temp > WSA4000_TRIGGER_DELAY_MAX || 
		temp % WSA4000_TRIGGER_DELAY_MULTIPLE != 0)
		return WSA_ERR_INVTRIGGERDELAY;
	else
		*delay = (int32_t) temp;
		return 0;

}

/**
 * Set the WSA's current synchronization state in the sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_mode - The synchronization mode (master/slave) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_sync(struct wsa_device *dev, char *sync_mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (strcmp(sync_mode, WSA4000_MASTER_TRIGGER) == 0 || 
		strcmp(sync_mode,  WSA4000_SLAVE_TRIGGER) == 0) 
		
		sprintf(temp_str, "SWEEP:LIST:TRIGGER:SYNC %s \n", sync_mode);
	
	else
		return WSA_ERR_INVTRIGGERSYNC;

	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_set_trigger_sync: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}
/**
 * Retrieve the WSA's current synchronization state in the sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_mode - The  trigger synchronization mode (master/slave)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_sync(struct wsa_device *dev, char *sync_mode)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "SWEEP:LIST:TRIGGER:SYNC?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	strcpy(sync_mode, query.output);
	
	if (strcmp(sync_mode, WSA4000_MASTER_TRIGGER) != 0 || 
		strcmp(sync_mode,  WSA4000_SLAVE_TRIGGER) != 0) 

		return WSA_ERR_INVTRIGGERSYNC;
	else
		return 0;

}


/**
 * Retrieve the current sweep list's status
 *
 * @param dev - A pointer to the WSA device structure.
 * @param status - An integer pointer to store status,
 * 1 - running, 0 - stopped
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_status(struct wsa_device *dev, char *status)
{
	struct wsa_resp query;	// store query results

	wsa_send_query(dev, "SWEEP:LIST:STATUS?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// check if the wsa returns an invalid sweep status
	if ((strcmp(query.output, WSA4000_SWEEP_STATE_STOPPED) != 0) &&
		(strcmp(query.output, WSA4000_SWEEP_STATE_RUNNING) != 0))
		return WSA_ERR_SWEEPMODEUNDEF;

	strcpy(status, query.output);

	return 0;
}


/**
 * Retrieve the current sweep list's size (i.e. the number of entries in the list)
 *
 * @param dev - A pointer to the WSA device structure
 * @param size - An integer pointer to store the size
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_get_sweep_entry_size(struct wsa_device *dev, int32_t *size)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(dev, "SWEEP:ENTRY:COUNT?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*size = (int32_t) temp;
		
	return 0;
}

/**
 * Delete an entry in the sweep list
 *
 * @param dev - A pointer to the WSA device structure
 * @param id - An integer containing the ID of the entry to be deleted
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_entry_delete(struct wsa_device *dev, int32_t id) 
{
	int16_t result;
	int32_t size;
	char temp_str[MAX_STR_LEN];
	
	// check if id is out of bounds
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return WSA_ERR_SWEEPENTRYDELETEFAIL;
	if (id < 0 || id > size)
		return WSA_ERR_SWEEPIDOOB;

	sprintf(temp_str, "SWEEP:ENTRY:DELETE %u\n", id);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_sweep_entry_delete: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Delete all entries in the sweep list
 *
 * @param dev - A pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_entry_delete_all(struct wsa_device *dev)
{
	int16_t result;
	char temp_str[MAX_STR_LEN];
	
	sprintf(temp_str, "SWEEP:ENTRY:DELETE ALL\n");
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_sweep_entry_delete: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * Copy the settings of a sweep entry with the specified ID in the sweep list 
 * to the entry template
 *
 * @param dev - A pointer to the WSA device structure
 * @param id - An integer representing the ID of the entry to be copied
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_entry_copy(struct wsa_device *dev, int32_t id) 
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	int32_t size = 0;

	result = wsa_get_sweep_entry_size(dev, &size);

	// check if the id is out of bounds
	if (id < 0 || id > size)
		return WSA_ERR_SWEEPIDOOB;

	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;
	if (size == 0)
		return  WSA_ERR_SWEEPLISTEMPTY;

	sprintf(temp_str, "SWEEP:ENTRY:COPY %u\n", id);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_sweep_entry_copy: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * start sweep mode
 *
 * @param dev - A pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_start(struct wsa_device *dev)
{	
	int16_t result = 0;
	char status[MAX_STR_LEN];
	int32_t size = 0;


	result = wsa_get_capture_mode(dev, status);
	if (result < 0)
		return result;

	// check if the wsa is already sweeping
	if (strcmp(status, WSA4000_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_SWEEPALREADYRUNNING;
	
	// check if the wsa is streaming
	if (strcmp(status, WSA4000_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_SWEEPWHILESTREAMING;
	
	// check if the sweep list is empty
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;	
	if (size <= 0) 
		return WSA_ERR_SWEEPLISTEMPTY;
	
	result = wsa_send_command(dev, "SWEEP:LIST:START\n");
	doutf(DHIGH, "In wsa_sweep_start: %d - %s.\n", result, wsa_get_error_msg(result));
	
	return result;
}


/**
 * start sweep mode with a specified sweep_id
 *
 * @param dev - A pointer to the WSA device structure
 * @param sweep_start_id - A  sweep start id to be sent in an
 * extension packet after a sweep has started
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_start_id(struct wsa_device *dev, int64_t sweep_start_id) 
{	
	int16_t result = 0;
	char status[MAX_STR_LEN];
	int32_t size = 0;
	char temp_str[MAX_STR_LEN];

	result = wsa_get_capture_mode(dev, status);
	if (result < 0)
		return result;

	// check if the wsa is already sweeping
	if (strcmp(status, WSA4000_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_SWEEPALREADYRUNNING;
	
	// check if the wsa is streaming
	if (strcmp(status, WSA4000_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_SWEEPWHILESTREAMING;

	// check if the sweep list is empty
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;	
	if (size <= 0) 
		return WSA_ERR_SWEEPLISTEMPTY;
	
	// make sure the stream id is valid
	if (sweep_start_id < 0 || sweep_start_id > UINT_MAX)
		return WSA_ERR_INVSWEEPSTARTID;

	sprintf(temp_str, "SWEEP:LIST:START %lld \n", sweep_start_id);
	result = wsa_send_command(dev, temp_str);

	doutf(DHIGH, "In wsa_sweep_start_id: %d - %s.\n", result, wsa_get_error_msg(result));
	
	return result;
}

/**
 * stop sweep mode, and read remaining data in the 
 * socket
 * @param dev - A pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_sweep_stop(struct wsa_device *dev) 
{

    int16_t result = 0;
	char status[MAX_STR_LEN];

	result = wsa_get_capture_mode(dev, status);
	if (result < 0)
		return result;

	// check if the wsa is already sweeping
	if (strcmp(status, WSA4000_SWEEP_CAPTURE_MODE) != 0)
		return WSA_ERR_SWEEPNOTRUNNING;
	
	result = wsa_send_command(dev, "SWEEP:LIST:STOP\n");
	doutf(DHIGH, "In wsa_sweep_stop: %d - %s.\n", result, wsa_get_error_msg(result));
	if (result < 0)
		return result;
	
	printf("Clearing socket buffer... ");
	
	// flush remaining data in the wsa
	result = wsa_flush_data(dev); 
	if (result < 0)
		return result;

	// clean remaining data in the data socket socket
	result = wsa_clean_data_socket(dev);
	if (result < 0)
		return result;

	printf("done.\n");
	
	return 0;
}

/**
 * Resume sweeping through the current sweep list starting
 * from the entry ID where the sweep engine stopped at
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_resume(struct wsa_device *dev) 
{
	int16_t result = 0;
	char status[40];
	int32_t size = 0;

	result = wsa_get_sweep_status(dev, status);
	if (result < 0)
		return result;
	if (strcmp(status, WSA4000_SWEEP_STATE_RUNNING) == 0) 
		return WSA_ERR_SWEEPALREADYRUNNING;

	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;
	if (size <= 0)
		return WSA_ERR_SWEEPLISTEMPTY;

	result = wsa_send_command(dev, "SWEEP:LIST:RESUME\n");
	doutf(DHIGH, "In wsa_sweep_resume: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
}

/**
 * This command is equivalent to reset the current settings in the entry template 
 * to default values
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_entry_new(struct wsa_device *dev) 
{
	int16_t result = 0;
	
	result = wsa_send_command(dev, "SWEEP:ENTRY:NEW\n");
	doutf(DHIGH, "In wsa_sweep_entry_new: %d - %s.\n", result, wsa_get_error_msg(result));
	
	return result;
} 


/**
 * Save the sweep entry to a specified ID location in the sweep list.
 * If the ID is 0, the entry will go to the end of the list.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param id - ID position where the entry will be saved in the list.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_entry_save(struct wsa_device *dev, int32_t id) 
{
	int16_t result;
	char temp_str[MAX_STR_LEN];
	int32_t size = 0;

	// check if id is out of bounds
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;

	if (id < 0 || id > size)
		return WSA_ERR_SWEEPIDOOB;

	sprintf(temp_str, "SWEEP:ENTRY:SAVE %u\n", id);
	result = wsa_send_command(dev, temp_str);
	doutf(DHIGH, "In wsa_sweep_entry_save: %d - %s.\n", result, wsa_get_error_msg(result));

	return result;
	
}


/**
 * Return the settings of the entry with the specified ID
 *
 * @param dev - A pointer to the WSA device structure.
 * @param id - the sweep entry ID in the wsa's list to read
 * @param sweep_list - a pointer structure to store the entry settings
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_entry_read(struct wsa_device *dev, int32_t id, struct wsa_sweep_list * const sweep_list)
{
	char temp_str[MAX_STR_LEN];
	struct wsa_resp query;		// store query results
	double temp;
	int32_t size = 0;
	int16_t result;
	char *strtok_result;
	
	// check if id is out of bounds
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;

	if (id < 0 || id > size)
		return WSA_ERR_SWEEPIDOOB;

	sprintf(temp_str, "SWEEP:ENTRY:READ? %d\n", id);
	wsa_send_query(dev, temp_str, &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	// *****
	// Convert the numbers & make sure no error
	// ****

	strtok_result = strtok(query.output, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->start_freq = (int64_t) temp;
	
	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->stop_freq = (int64_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;	
	sweep_list->fstep= (int64_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->fshift = (float) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;	
	sweep_list->decimation_rate = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;	
	sweep_list->ant_port = (int32_t) temp;
	
	// Convert to wsa_gain type
	strtok_result = strtok(NULL, ",");
	strcpy(sweep_list->gain_rf,strtok_result);

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->gain_if = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->samples_per_packet = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->packets_per_block = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;
	sweep_list->dwell_seconds = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	if (to_double(strtok_result, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;	
	sweep_list->dwell_microseconds = (int32_t) temp;

	strtok_result = strtok(NULL, ",");	
	if (strstr(strtok_result, "LEVEL") != NULL)
	{
		strcpy(sweep_list->trigger_type,strtok_result);

		strtok_result = strtok(NULL, ",");
		if (to_double(strtok_result, &temp) < 0)
			return WSA_ERR_RESPUNKNOWN;
		sweep_list->trigger_start_freq = (int64_t) temp;
		
		strtok_result = strtok(NULL, ",");
		if (to_double(strtok_result, &temp) < 0)
			return WSA_ERR_RESPUNKNOWN;
		sweep_list->trigger_stop_freq = (int64_t) temp;

		strtok_result = strtok(NULL, ",");
		if (to_double(strtok_result, &temp) < 0)
			return WSA_ERR_RESPUNKNOWN;	
		sweep_list->trigger_amplitude = (int32_t) temp;
	}
	else if (strstr(strtok_result, "NONE") != NULL)
	{
		strcpy(sweep_list->trigger_type,strtok_result);
	}

	return 0;
}

