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
#include <stdlib.h>//library for general functions like random number generators integer arithmatic
#include <string.h>//functions to manage strings
#include <math.h>// sins cosines abs
#include <time.h>//used to manipulate header and time information

#include "thinkrf_stdint.h"//declare all the different types of signed and unsigned integers
#include "wsa_error.h"//contains a list of all the errors
#include "wsa_commons.h"//common functions used
#include "wsa_lib.h"
#include "wsa_api.h"//calls on itself


#define MAX_RETRIES_READ_FRAME 5



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
 * @param header - A pointer to \b wsa_vrt_packet_header structure to store 
 *		the VRT header information
 * @param trailer - A pointer to \b wsa_vrt_packet_trailer structure to store 
 *		the VRT trailer information
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
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		struct wsa_reciever_packet* const reciever,
		struct wsa_digitizer_packet* const digitizer,
		int16_t* const i_buffer, 
		int16_t* const q_buffer,
		const uint16_t samples_per_packet,
		uint8_t* context_is)
{
	uint8_t* data_buffer = 0;
	int16_t return_status = 0;
	uint8_t context_present = 0;
	int64_t frequency = 0;

	// allocate the data buffer
	data_buffer = (uint8_t*) malloc(samples_per_packet * BYTES_PER_VRT_WORD * sizeof(uint8_t));

	return_status = wsa_read_iq_packet_raw(device, header, trailer, reciever, digitizer, data_buffer, samples_per_packet, &context_present);
	
	doutf(DMED, "In wsa_read_iq_packet: wsa_read_iq_packet_raw returned %hd\n", return_status);

	if (return_status < 0)
	{
	
		doutf(DHIGH, "Error in wsa_read_iq_packet: %s\n", wsa_get_error_msg(return_status));
		free(data_buffer);
		return return_status;
	} 
	
	if (context_present == 1) {
		*context_is = 1;
		free(data_buffer);
		return 0;

	} else if (context_present == 2) {
		*context_is = 2;
		free(data_buffer);
		return 0;
	} else if (context_present ==0) {



	// Note: don't rely on the value of return_status
	return_status = (int16_t) wsa_decode_frame(data_buffer, i_buffer, q_buffer, samples_per_packet);
	*context_is = 0;
	free(data_buffer);
	
	return 0;
	}
}




int16_t wsa_read_iq_packet_matlab (struct wsa_device* const device, 
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		int16_t* const i_buffer, 
		int16_t* const q_buffer,
		 uint16_t samples_per_packet,
		uint8_t* context_is, int32_t* rec_indicator_field, int32_t* rec_reference_point, int64_t* rec_frequency, int16_t* rec_gain_if, int16_t* rec_gain_rf, int32_t* rec_temperature,
	int32_t* dig_indicator_field, int64_t* dig_bandwidth, int32_t* dig_reference_level, int64_t* dig_rf_frequency_offset)
{

	int16_t return_status = 0;
	uint8_t context_present = 0;
	int32_t indicator_fieldr = 0;
	int32_t reference_point = 0;
	int64_t frequency = 0;
	int16_t gain_if = 0;
	int16_t gain_rf = 0;
	int32_t temperature = 0;
	int32_t indicator_fieldd = 0;
	int64_t bandwidth = 0;
	int32_t reference_level = 0;
	int64_t rf_frequency_offset = 0;
	uint8_t* data_buffer = 0;

	struct wsa_reciever_packet* reciever;
	struct wsa_digitizer_packet* digitizer;
	samples_per_packet = 1024;
	// allocate the data buffer

	data_buffer = (uint8_t*) malloc(samples_per_packet * BYTES_PER_VRT_WORD * sizeof(uint8_t));



	reciever = (struct wsa_reciever_packet*) malloc(sizeof(struct wsa_reciever_packet));

	digitizer = (struct wsa_digitizer_packet*) malloc(sizeof(struct wsa_digitizer_packet));

	return_status = wsa_read_iq_packet_raw(device, header, trailer, reciever, digitizer, data_buffer, samples_per_packet, &context_present);
	

	

	*rec_indicator_field =(int) reciever->indicator_field;
	*rec_reference_point =(int) reciever->reference_point;
	*rec_frequency =(long long) reciever->frequency;
	*rec_gain_if = (unsigned short) reciever->gain_if;
	*rec_gain_rf = (unsigned short) reciever->gain_rf;
	*rec_temperature = (int)reciever->temperature;
	*dig_indicator_field = (int) digitizer->indicator_field;
	*dig_bandwidth = (long long)digitizer-> bandwidth;
	*dig_reference_level = (int)digitizer->reference_level;
	*dig_rf_frequency_offset = (long long) digitizer->rf_frequency_offset;

	


	
	doutf(DMED, "In wsa_read_iq_packet: wsa_read_iq_packet_raw returned %hd\n", return_status);

	if (return_status < 0)
	{
	
		doutf(DHIGH, "Error in wsa_read_iq_packet: %s\n", wsa_get_error_msg(return_status));
		free(data_buffer);
		free(reciever);
		free(digitizer);
		return return_status;
	} 
	
	if (context_present == 1) {
		*context_is = 1;
		free(data_buffer);
		free(reciever);
		free(digitizer);
		return 0;
	} else if (context_present == 2) {
		*context_is = 2;
		free(data_buffer);
		free(reciever);
		free(digitizer);
		return 0;
	} else if (context_present == 0) {



	// Note: don't rely on the value of return_status
	return_status = (int16_t) wsa_decode_frame(data_buffer, i_buffer, q_buffer, samples_per_packet);
	*context_is = 0;
	free(data_buffer);
	free(reciever);
	free(digitizer);
	return 0;
	}
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
 * Gets the number of samples that will be returned in each
 * VRT packet when \b wsa_read_iq_packet is called
 * 
 * @param device - A pointer to the WSA device structure.
 * @param samples_per_packet - A uint16_t pointer to store the samples per packet
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_samples_per_packet(struct wsa_device* device, uint16_t* samples_per_packet)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(device, "TRACE:SPPACKET?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
	{
		printf("query status is: %hu \n", query.status);
		return (int16_t) query.status;
	}

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA4000_MIN_SAMPLES_PER_PACKET) || 
		(temp > WSA4000_MAX_SAMPLES_PER_PACKET))
	{
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*samples_per_packet = (uint16_t) temp;

	return 0;
}


/**
 * Sets the number of VRT packets per each capture block.
 * The number of samples in each packet is set by
 * the method wsa_set_samples_per_packet.
 * After capturing the block with the method wsa_capture_block,
 * read back the data by calling wsa_read_iq_packet
 * \b packets_per_block number of times
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - number of packets
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_packets_per_block(struct wsa_device *dev, uint32_t packets_per_block)
{
	int16_t result;
	char temp_str[50];

	sprintf(temp_str, "TRACE:BLOCK:PACKETS %u\n", packets_per_block);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In wsa_set_packets_per_block: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_INVCAPTURESIZE;
	}

	return 0;
}


/**
 * Gets the number of VRT packets to be captured
 * when \b wsa_capture_block is called
 * 
 * @param device - A pointer to the WSA device structure.
 * @param packets_per_block - A uint32_t pointer to store the number of packets
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_packets_per_block(struct wsa_device* device, uint32_t* packets_per_block)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(device, "TRACE:BLOCK:PACKETS?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}

	*packets_per_block = (uint32_t) temp;

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
	
		*cfreq = (int64_t) temp/MHZ;
		
	   

	

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
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq)
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
	// verify the frequency value
	if (freq < dev->descr.min_tune_freq || freq > dev->descr.max_tune_freq)	{
		return WSA_ERR_FREQOUTOFBOUND;
	}

	return 0;
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

	wsa_send_query(dev, "FREQ:SHIFT?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// Verify the validity of the return value TODO
	if (temp < 0 || temp > dev->descr.inst_bw) {
		printf("Error: WSA returned %s.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*fshift = (float) temp;

	return 0;
}


/**
 * Sets frequency shift value
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param fshift - The center frequency to set, in Hz
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Frequency out of range.
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_freq_shift(struct wsa_device *dev, float fshift)
{
	int16_t result = 0;
	char temp_str[50];
	int64_t range = dev->descr.inst_bw;

	// verify the value bwn -125 to 125MHz, "exclusive"
	if (fshift <= (-range) || fshift >= range)	{
		return WSA_ERR_FREQOUTOFBOUND;
	}

	sprintf(temp_str, "FREQ:SHIFt %f Hz\n", fshift);

	// set the freq shift using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result == WSA_ERR_SETFAILED) {
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		return result;
	
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

	wsa_send_query(dev, "INP:FILT:PRES?\n", &query);
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

	sprintf(temp_str, "INPUT:FILT:PRES %d\n", mode);

	// set the freq using the selected connect type
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_FILTERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FILTERSETFAILED));
		return WSA_ERR_FILTERSETFAILED;
	}

	return 0;
}


// ////////////////////////////////////////////////////////////////////////////
// DEVICE SETTINGS					                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_firm_v(struct wsa_device *dev)
{
	struct wsa_resp query;		// store query results

	int16_t i = 0;
	
	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;
	
	wsa_send_query(dev, "*IDN?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	printf("\t\t- Firmware Version: ");
	for(i=44; i<49;i++){
		
		printf("%c", query.output[i]);

	}

	printf("\n");

	return 0;
}



// ////////////////////////////////////////////////////////////////////////////
// TRIGGER CONTROL SECTION                                                   //
// ////////////////////////////////////////////////////////////////////////////






/**
 * Sets the WSA to use basic a level trigger
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param start_frequency - The lowest frequency at which a signal should be detected
 * @param stop_frequency - The highest frequency at which a signal should be detected
 * @param amplitude - The minimum amplitutde of a signal that will satisfy the trigger
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_level(struct wsa_device *dev, int64_t start_frequency, int64_t stop_frequency, int64_t amplitude)
{
	int16_t result = 0;
	char temp_str[50];

	result = wsa_verify_freq(dev, start_frequency);
	if (result == WSA_ERR_FREQOUTOFBOUND)
	{
		return WSA_ERR_STARTOOB;
	}
	else if (result < 0)
	{
		return result;
	}

	result = wsa_verify_freq(dev, stop_frequency);
	if (result == WSA_ERR_FREQOUTOFBOUND)
	{
		return WSA_ERR_STOPOOB;
	}
	else if (result < 0)
	{
		return result;
	}

	sprintf(temp_str, ":TRIG:LEVEL %lld,%lld,%lld\n", start_frequency, stop_frequency, amplitude);

	result = wsa_send_command(dev, temp_str);
	if (result == WSA_ERR_SETFAILED)
	{
		doutf(DMED, "Error WSA_ERR_TRIGGERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_TRIGGERSETFAILED));
		return result;
	}
	else if (result < 0) 
	{
		return result;
	}

	return 0;
}


/**
 * Retrieves the basic level trigger settings
 *
 * @param dev - A pointer to the WSA device structure.
 * @param start_frequency - A long integer pointer to store the start frequency in Hz.
 * @param stop_frequency - A long integer pointer to store the stop frequency in Hz.
 * @param amplitude - A long integer pointer to store the signal amplitude in dBm.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_trigger_level(struct wsa_device* dev, int64_t* start_frequency, int64_t* stop_frequency, int64_t* amplitude)
{
	struct wsa_resp query;		// store query results
	double temp;
	char* strtok_result;

	wsa_send_query(dev, ":TRIG:LEVEL?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}
		
	strtok_result = strtok(query.output, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq) 
	{
		printf("Error: WSA returned %s.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}	
	
	*start_frequency = (int64_t) temp;
		
	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	// Verify the validity of the return value
	if (temp < dev->descr.min_tune_freq || temp > dev->descr.max_tune_freq) 
	{
		printf("Error: WSA returned %s.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}	
	
	*stop_frequency = (int64_t) temp;
	
	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*amplitude = (int64_t) temp;

	return 0;
}


/**
 * Sets the WSA's capture mode to triggered (trigger on)
 * or freerun (trigger off).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param enable - Trigger mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_enable(struct wsa_device* dev, int32_t enable)
{
	int16_t result = 0;
	char temp_str[50];

	if (enable < 0 || enable > 1) {
		return WSA_ERR_INVTRIGGERMODE;
	}

	sprintf(temp_str, ":TRIGGER:ENABLE %d\n", enable);

	result = wsa_send_command(dev, temp_str);


	 if (result < 0) {
		doutf(DMED, "Error WSA_ERR_TRIGGERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_TRIGGERSETFAILED));
		return result;
	}



	return 0;
}


/**
 * Gets the current capture mode of the WSA
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param enable - An integer pointer to store the current mode: 
 * 1 = triggered (trigger on), 0 = freerun (trigger off).
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_enable(struct wsa_device* dev, int32_t* enable)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(dev, ":TRIG:ENABLE?\n", &query);
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (temp < 0 || temp > 1) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}
		
	*enable = (int16_t) temp;

	return 0;
}



// ////////////////////////////////////////////////////////////////////////////
// Sweep Functions				                                             //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Get the antenna port in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer pointer to store the antenna location: 
 * 1 = antenna1, 2 = antenna2.
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
 * Set the antenna in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param port_num - An integer to store the antenna location: 
 * 1 = antenna1, 2 = antenna2.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_antenna(struct wsa_device *dev, int32_t port_num) 
{
	int16_t result = 0;
	char temp_str[30];

	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (port_num < 1 || port_num > WSA_RFE0560_MAX_ANT_PORT) // TODO replace the max
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, "SWEEP:ENTRY:ANTENNA %d\n", port_num);

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
 * Get the if gain in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - An integer  to store the gain value: 
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
 * Set the if gain in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - An integer  to store the gain value: 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_gain_if(struct wsa_device *dev, int32_t gain)
{
	
	int16_t result = 0;
	char temp_str[50];
	
	if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0)
		return WSA_ERR_INVRFESETTING;

	if (gain < dev->descr.min_if_gain || gain > dev->descr.max_if_gain)
		return WSA_ERR_INVIFGAIN;

	sprintf(temp_str, "SWEEP:ENTRY:GAIN:IF %d\n", gain);

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
 * Get the rf gain in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - An integer  to store the gain value: 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_gain_rf(struct wsa_device *dev, enum wsa_gain *gain)
{	
	
	struct wsa_resp query;		// store query results

	wsa_send_query(dev, "SWEEP:ENTRY:GAIN:RF?\n", &query);
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
 * Set the rf gain in the user's sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gain - An integer  to store the gain value: 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_gain_rf(struct wsa_device *dev, enum wsa_gain gain)
{
	int16_t result = 0;
	char temp_str[50];

	if (gain > WSA_GAIN_VLOW || gain < WSA_GAIN_HIGH)
		return WSA_ERR_INVRFGAIN;

	strcpy(temp_str, "SWEEP:ENTRY:GAIN:RF ");
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
   
   
   

/**
 * Gets the number of samples per packet in the
 * user's sweep entry
 * 
 * @param device - A pointer to the WSA device structure.
 * @param samples_per_packet - A uint16_t pointer to store the samples per packet
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sweep_samples_per_packet(struct wsa_device* device, uint16_t* samples_per_packet)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(device, "SWEEP:ENTRY:SPPACKET?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA4000_MIN_SAMPLES_PER_PACKET) || 
		(temp > WSA4000_MAX_SAMPLES_PER_PACKET))
	{
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*samples_per_packet = (uint16_t) temp;

	return 0;
}


/**
 * Sets the number of samples per packet in the user's
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param samples_per_packet - The sample size to set.
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sweep_samples_per_packet(struct wsa_device* device, uint16_t samples_per_packet)
{
	int16_t result;
	char temp_str[50];

	if ((samples_per_packet < WSA4000_MIN_SAMPLES_PER_PACKET) || 
		(samples_per_packet > WSA4000_MAX_SAMPLES_PER_PACKET))
	{
		return WSA_ERR_INVSAMPLESIZE;
	}

	sprintf(temp_str, "SWEEP:ENTRY:SPPACKET %hu\n", samples_per_packet);

	result = wsa_send_command(device, temp_str);
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
 * Gets the packets per block in the user's
 * sweep entry
 * 
 * @param device - A pointer to the WSA device structure.
 * @param packets_per_block - A uint32_t pointer to store the number of packets
 *
 * @return 0 if successful, or a negative number on error.
 */
int16_t wsa_get_sweep_packets_per_block(struct wsa_device* device, uint32_t* packets_per_block)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(device, "SWEEP:ENTRY:PPBLOCK?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}

	// Convert the number & make sure no error
	if (to_int(query.output, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}

	*packets_per_block = (uint32_t) temp;

	return 0;
}

/**
 * Sets the number of packets per block in the user's
 * sweep entry 
 *
 *
 * @param dev - A pointer to the WSA device structure.
 * @param packets_per_block - number of packets
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sweep_packets_per_block(struct wsa_device* device, uint32_t packets_per_block)
{
	int16_t result;
	char temp_str[50];

	sprintf(temp_str, "SWEEP:ENTRY:PPBLOCK %u\n", packets_per_block);

	result = wsa_send_command(device, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In wsa_set_packets_per_block: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_INVCAPTURESIZE;
	}

	return 0;
}


/**
 * Gets the decimation rate currently set in the user's
 * sweep list 
 *
 * @param dev - A pointer to the WSA device structure.
 * @param rate - A pointer to the decimation rate of integer type.
 *
 * @return The sample size if success, or a negative number on error.
 */
int16_t wsa_get_sweep_decimation(struct wsa_device* device, int32_t* rate)
{
	struct wsa_resp query;		// store query results
	long temp;

	wsa_send_query(device, "SWEEP:ENTRY:DECIMATION?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// convert & make sure no error
	if (to_int(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	// make sure the returned value is valid
	if (((temp != 0) && (temp < device->descr.min_decimation)) || 
		(temp > device->descr.max_decimation)) {
		printf("Error: WSA returned %ld.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*rate = (int32_t) temp;

	return 0;
}

/**
 * Sets the decimation rate from the user's sweep list. 
 * @note: The rate here implies that at every given 'rate' (samples), 
 * only one sample is stored. Ex. if rate = 100, for a trace 
 * frame of 1000, only 10 samples is stored???
 *
 * Rate supported: 0, 4 - 1024. Rate of 0 is equivalent to no decimation.
 * 
 * @param device - A pointer to the WSA device structure.
 * @param rate - The decimation rate to set.
 *
 * @return 0 if success, or a negative number on error.
 */
int16_t wsa_set_sweep_decimation(struct wsa_device* device, int32_t rate)
{
	int16_t result;
	char temp_str[50];

	// TODO get min & max rate
	if (((rate != 0) && (rate < device->descr.min_decimation)) || 
		(rate > device->descr.max_decimation))
		return WSA_ERR_INVDECIMATIONRATE;

	sprintf(temp_str, "SWEEP:ENTRY:DECIMATION %d", rate);
	
	// set the rate using the selected connect type
	result = wsa_send_command(device, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_SIZESETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_SIZESETFAILED));
		return WSA_ERR_SETFAILED;
	}

	return 0;
}

/**
 * Retrieves the center frequency from the.
 * from the user's sweep entry
 * @param dev - A pointer to the WSA device structure.
 * @param freq - A long integer pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_freq(struct wsa_device* device, int64_t* start_freq, int64_t* stop_freq)
{
	struct wsa_resp query;		// store query results
	double temp;
	char* strtok_result;

	wsa_send_query(device, "SWEEP:ENTRY:FREQ:CENTER?\n", &query);
	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	strtok_result = strtok(query.output, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}


	*start_freq = (int64_t) temp;

	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	*stop_freq = (int64_t) temp;
	
	return 0;
}
/**
 * Sets the center frequency in the user's
 * sweep entry.
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param freq - The center frequency to set, in Hz
 * @return 0 on success, or a negative number on error.
 * @par Errors:
 * - Frequency out of range.
 * - Set frequency when WSA is in trigger mode.
 * - Incorrect frequency resolution (check with data sheet).
 */
int16_t wsa_set_sweep_freq(struct wsa_device* device, int64_t start_frequency, int64_t stop_frequency )
{
	int16_t result = 0;
	char temp_str[50];

	
	if (result < 0)
		return result;

	sprintf(temp_str, "SWEEP:ENTRY:FREQ:CENT %lld Hz, %lld Hz\n",start_frequency,stop_frequency);
	printf("got to freq\n");
	// set the freq using the selected connect type
	result = wsa_send_command(device, temp_str);

	if (result == WSA_ERR_SETFAILED){
	
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		
		return result;

	return 0;
}

/**
 * Retrieves the frequency shift value from the user's 
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fshift - A float pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_freq_shift(struct wsa_device* device, float* fshift)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(device, "SWEEP:ENTRY:FREQ:SHIFT?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	

	*fshift = (float) temp;

	return 0;
}
/**
 * Sets the frequency shift value in the user's
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fshift - A float pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_set_sweep_freq_shift(struct wsa_device* device, float fshift)
{
	int16_t result = 0;
	char temp_str[50];
	int64_t range = device->descr.inst_bw;

	// verify the value bwn -125 to 125MHz, "exclusive"
	if (fshift <= (-range) || fshift >= range)	{
		return WSA_ERR_FREQOUTOFBOUND;
	}

	sprintf(temp_str, "SWEEP:ENTRY:FREQ:SHIFt %f Hz\n", fshift);

	// set the freq shift using the selected connect type
	result = wsa_send_command(device, temp_str);
	if (result == WSA_ERR_SETFAILED) {
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		return result;
	
	return 0;
}

/* Sets the frequency step value in the user's
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fstep - A float pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_set_sweep_freq_step(struct wsa_device* device, int64_t step)
{
	int16_t result = 0;
	char temp_str[50];

	
	sprintf(temp_str, "SWEEP:ENTRY:FREQ:STEP %lld Hz\n", step);
	
	// set the freq step using the selected connect type
	result = wsa_send_command(device, temp_str);
	if (result == WSA_ERR_SETFAILED){
		
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED);
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		return result;

	return 0;
}
/**
 * Retrieves the frequency step value from the user's 
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param fstep - A float pointer to store the frequency in Hz.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_freq_step(struct wsa_device* device, int64_t* fstep)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(device, "SWEEP:ENTRY:FREQ:STEP?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;


	
	
	*fstep = (int64_t) temp;
	return 0;
}
/* Sets the dwell value in the user's
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param dwell_seconds_value - A float pointer to store the integer portion.
  * @param dwell_useconds_value - A float pointer to store the decimal portion.
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_set_sweep_dwell(struct wsa_device* device, int32_t dwell_seconds_value, int32_t dwell_useconds_value)
{
	int16_t result = 0;
	char temp_str[50];

	sprintf(temp_str, "SWEEP:ENTRY:DWELL %u,%u\n", dwell_seconds_value, dwell_useconds_value);
	printf("the temp string is: %s \n", temp_str);
	// set the freq using the selected connect type
	result = wsa_send_command(device, temp_str);
	if (result == WSA_ERR_SETFAILED){
		doutf(DMED, "Error WSA_ERR_FREQSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_FREQSETFAILED));
		return WSA_ERR_FREQSETFAILED;
	}
	else if (result < 0) 
		return result;

	return 0;
}

/* retrieves the dwell value in the user's
 * sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param dwell_seconds_value - A float pointer to store the integer portion.
  * @param dwell_useconds_value - A float pointer to store the decimal portion.
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_dwell(struct wsa_device* device, int32_t* dwell_seconds_value, int32_t* dwell_useconds_value)
{
	struct wsa_resp query;		// store query results
	double temp =5;
	char* strtok_result;
	wsa_send_query(device, "SWEEP:ENTRY:DWELL?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	

	strtok_result = strtok(query.output, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}


	*dwell_seconds_value = (int32_t) temp;

	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	*dwell_useconds_value = (int32_t) temp;
	
	return 0;
	

}


/**
 * Sets the user's sweep entry to use basic a level trigger
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param start_frequency - The lowest frequency at which a signal should be detected
 * @param stop_frequency - The highest frequency at which a signal should be detected
 * @param amplitude - The minimum amplitutde of a signal that will satisfy the trigger
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_level(struct wsa_device *dev, int64_t start_frequency, int64_t stop_frequency, int64_t amplitude)
{
	
	int16_t result = 0;
	char temp_str[50];
	result = wsa_verify_freq(dev, start_frequency);
	if (result == WSA_ERR_FREQOUTOFBOUND)
	{
	
		return WSA_ERR_STARTOOB;
	}
	else if (result < 0)
	{
		return result;
	}

	result = wsa_verify_freq(dev, stop_frequency);
	if (result == WSA_ERR_FREQOUTOFBOUND)
	{
		return WSA_ERR_STOPOOB;
	}
	else if (result < 0)
	{
		return result;
	}

	sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:LEVEL %lld,%lld,%lld\n", start_frequency, stop_frequency, amplitude);

	result = wsa_send_command(dev, temp_str);
	if (result == WSA_ERR_SETFAILED)
	{
		doutf(DMED, "Error WSA_ERR_TRIGGERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_TRIGGERSETFAILED));
		return WSA_ERR_TRIGGERSETFAILED;
	}
	else if (result < 0) 
	{
		return result;
	}

	return 0;
}


/**
 * Retrieves the user's sweep entry basic level trigger settings
 *
 * @param dev - A pointer to the WSA device structure.
 * @param start_frequency - A long integer pointer to store the start frequency in Hz.
 * @param stop_frequency - A long integer pointer to store the stop frequency in Hz.
 * @param amplitude - A long integer pointer to store the signal amplitude in dBm.
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_level(struct wsa_device* dev, int64_t* start_frequency, int64_t* stop_frequency, int64_t* amplitude)
{
	
	struct wsa_resp query;		// store query results
	double temp;
	char* strtok_result;
	wsa_send_query(dev, "SWEEP:ENTRY:TRIGGER:LEVEL?\n", &query);
	 
	// Handle the query output here 
	if (query.status <= 0)
	{
		
		return (int16_t) query.status;
	}
	
	strtok_result = strtok(query.output, ",");

	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	// Verify the validity of the return value
	
	
	*start_frequency = (int64_t) temp;

	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	// Verify the validity of the return value

	
	*stop_frequency = (int64_t) temp;
	
	strtok_result = strtok(NULL, ",");
	// Convert the number & make sure no error
	if (to_double(strtok_result, &temp) < 0)
	{
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*amplitude = (int64_t) temp/1000;

	return 0;
}


/**
 * Sets the user's sweep entry capture mode to triggered (trigger on)
 * or freerun (trigger off).
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param enable - Trigger mode of selection: 0 - Off, 1 - On.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_type(struct wsa_device* dev, int32_t enable)
{
	
	int16_t result = 0;
	char temp_str[50];
	if (enable < 0 || enable > 1) {
		return WSA_ERR_INVTRIGGERMODE;
	}

	if ( enable == 0) {
	sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:TYPE NONE\n");

	} else if ( enable == 1) {
		sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:TYPE LEVEL\n");
	}


	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_TRIGGERSETFAILED: %s.\n", 
			wsa_get_error_msg(WSA_ERR_TRIGGERSETFAILED));
		return WSA_ERR_TRIGGERSETFAILED;
	}

	return 0;
}


/**
 * retrieves the current user's sweep entry capture mode of the WSA
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param enable - An integer pointer to store the current mode: 
 * 1 = triggered (trigger on), 0 = freerun (trigger off).
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_type(struct wsa_device* dev, int32_t* enable)
{

	
	struct wsa_resp query;
	long temp;
	wsa_send_query(dev, "SWEEP:ENTRY:TRIGGER:TYPE?\n", &query);
	if (query.status <= 0)
	{
		return (int16_t) query.status;
	}
	 if (strcmp(query.output, "LEVEL") == 0) {
		 *enable = 1;
	
	 } else if
		 (strcmp(query.output, "NONE") == 0) {
		  *enable = 0;
		
	 }
	return 0;
}

/**
 * retrieves the current sweep list's iteration
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param iterat - An integer pointer to store the number of iterations 
 * that the sweep will go
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_iteration(struct wsa_device* device, int32_t* iterat)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(device, "SWEEP:LIST:ITERATION?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	
	*iterat = (int32_t) temp;
	return 0;
}
/**
 * sets the current sweeps iteraiton
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param iterat - An integer pointer to store the number of iterations 
 * that the sweep will go
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_iteration(struct wsa_device* device, int32_t iterat)
{

	int16_t result;
	char temp_str[50];
	printf("got here \n");
	if ((iterat < WSA4000_MIN_SWEEP_ITERATION) || 
		(iterat > WSA4000_MAX_SWEEP_ITERATION))
	{
		return WSA_ERR_INVSAMPLESIZE;
	}
	printf("got to error 1\n");
	sprintf(temp_str, "SWEEP:LIST:ITERATION %u\n", iterat);

	result = wsa_send_command(device, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In iterat: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_SIZESETFAILED;
	}

	return 0;
}
/**
 * retrieves the current sweep list's status
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param status - An integer pointer to store status
 *1 - running, 0 - stopped
 * 
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_status(struct wsa_device* device, int32_t* status)
{
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(device, "SWEEP:LIST:STATUS?\n", &query);

	
	
	 if (strcmp(query.output, "STOPPED") == 0) {
		 *status = 0;
	
	 } else if (strcmp(query.output, "RUNNING") == 0) {
		  *status = 1;
		
	 }
	return 0;
}


/**
 * retrieves the current sweep list's size
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param size - An integer pointer to store the size
 *
 * 
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_list_size(struct wsa_device* device, int32_t* size)
{
	struct wsa_resp query;		// store query results
	double temp;
	wsa_send_query(device, "SWEEP:ENTRY:COUNT?\n", &query);

	// Handle the query output here 
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (to_double(query.output, &temp) < 0)
		return WSA_ERR_RESPUNKNOWN;

	
		*size = (int32_t) temp;
		return 0;
}


/**
 * delete an entry in the sweep list
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param position - An integer containing the positon of the entry to be deleted
 *
 * 
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_list_delete(struct wsa_device *dev, int32_t position) {
	int16_t result;
	char temp_str[50];



	sprintf(temp_str, "SWEEP:ENTRY:DELETE %hu\n", position);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In delete: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_SWEEPDELETEFAIL;
	}

	return 0;
}


/**
 * copy an entry in the sweep list to the user's entry
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param position - An integer containing the positon of the entry to be copied
 *
 * 
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_list_copy(struct wsa_device *dev, int32_t position) {
	int16_t result;
	char temp_str[50];
	printf("got to copy\n");
	sprintf(temp_str, "SWEEP:ENTRY:COPY %hu\n", position);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In copy: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_SWEEPCOPYFAIL;
	}

	return 0;
}

/**
 * start sweeping through the current sweep list
 * 
 * @param dev - A pointer to the WSA device structure.
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_start(struct wsa_device *device) {
	
	int16_t return_status = 0;

	return_status = wsa_send_command(device, "SWEEP:LIST:START\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in sweep start: %s\n", wsa_get_error_msg(return_status));
		return WSA_ERR_SWEEPSTARTFAIL;
	}

	return 0;
}


/**
 * stop sweeping
 * 
 * @param dev - A pointer to the WSA device structure.
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_stop(struct wsa_device *device) {
	int16_t return_status = 0;

	return_status = wsa_send_command(device, "SWEEP:LIST:STOP\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);
	printf("stop the sweep \n" );
	if (return_status < 0)
	{
		doutf(DHIGH, "Error in sweep stop: %s\n", wsa_get_error_msg(return_status));
		return WSA_ERR_SWEEPSTOPFAIL;
	}

	return 0;
}


/**
 * resume sweeping through the current sweep list
 * from the list position which was stopped
 *
 * @param dev - A pointer to the WSA device structure.
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_resume(struct wsa_device *device) {
	int16_t return_status = 0;

	return_status = wsa_send_command(device, "SWEEP:LIST:RESUME\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in sweep resume: %s\n", wsa_get_error_msg(return_status));
		return WSA_ERR_SWEEPRESUMEFAIL;
	}

	return 0;
}
/**
 * start a new user's sweep entry
 * (note this will erase the current entry)
 *
 * @param dev - A pointer to the WSA device structure.
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_entry_new(struct wsa_device *device) {

	int16_t return_status = 0;
	
		printf("got here\n");
	return_status = wsa_send_command(device, "SWEEP:ENTRY:NEW\n");
	doutf(DMED, "In wsa_capture_block: wsa_send_command returned %hd\n", return_status);

	if (return_status < 0)
	{
		doutf(DHIGH, "Error in sweep new: %s\n", wsa_get_error_msg(return_status));
		return WSA_ERR_SWEEPNEWFAIL;
	}

	return 0;
}
/**
 * save the user's sweep entry to the sweep list
 * 
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param position - Position where the entry will be saved in the list 
 *@return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_entry_save(struct wsa_device *device, int32_t position) {
	int16_t result;
	char temp_str[50];
	printf("got to save");
	sprintf(temp_str, "SWEEP:ENTRY:SAVE %u\n", position);

	result = wsa_send_command(device, temp_str);
	if (result < 0) 
	{
		doutf(DHIGH, "In save: wsa_send_command returned %hd: %s.\n",
			result,
			wsa_get_error_msg(result));
		return WSA_ERR_SWEEPSAVEFAIL;
	}

	return 0;

}

/**
 * save the user's sweep entry to the sweep list
 * 
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param position - Position where the entry will be saved in the list 
 *@return 0 on success, or a negative number on error.
 */
int16_t wsa_sweep_list_read(struct wsa_device *device, int32_t position){
	int16_t result;
	char temp_str[50];
	struct wsa_resp query;		// store query results

	sprintf(temp_str, "SWEEP:ENTRY:READ? %hu\n", position);
	
	result = wsa_send_query(device, temp_str, &query);
	
	printf("output: %s \n ", query.output);

	return 0;

}


// ////////////////////////////////////////////////////////////////////////////
// Context Packet test Function                                               //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Retrieve Reciever Context Packets
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param Reference Point 
 * @param RF Reference Frequency
 * @param Gain
 * @param Temperature
 * @return 0 on success, or a negative number on error.
 */

int16_t wsa_get_context_digitizer(struct wsa_device* const dev, int32_t packets_per_block, int32_t samples_per_packet)

{

	
	int16_t result = 0;	
	clock_t sec = 5;
	int32_t i = 0;

	for (i = 0; i<100 ; i++) {

	int64_t freq = 2414*MHZ;
	clock_t start_time = clock();
	clock_t end_time = sec * 1000 + start_time;
	result = wsa_set_freq(dev, freq);	
	while(clock() != end_time);


	freq =5000*MHZ;
	 start_time = clock();
	 end_time = sec * 1000 + start_time;
	result = wsa_set_freq(dev, freq);	
	while(clock() != end_time);
	

	
	freq =8000*MHZ;
	 start_time = clock();
	 end_time = sec * 1000 + start_time;
	result = wsa_set_freq(dev, freq);	
	while(clock() != end_time);

	
	
	
	}
}




