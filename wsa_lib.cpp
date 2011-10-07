
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wsa_commons.h"
#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_lib.h"


//*****
// LOCAL DEFINES
//*****


//TODO create a log file method
//TODO add proper error method


/**
 * Initialized the the wsa_device structure
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return None
 */
int16_t wsa_dev_init(struct wsa_device *dev)
{
	dev->descr.inst_bw = 0;
	dev->descr.max_sample_size = 0;
	dev->descr.max_tune_freq = 0;
	dev->descr.min_tune_freq = 0;
	dev->descr.freq_resolution = 0;
	dev->descr.max_if_gain = -1000;	// some impossible #
	dev->descr.min_if_gain = -1000;	// some impossible #
	for (int i = 0; i < NUM_RF_GAINS; i++)
		dev->descr.abs_max_amp[i] = -1000;	// some impossible #

	strcpy(dev->descr.prod_name, "");
	strcpy(dev->descr.prod_serial, ""); 
	strcpy(dev->descr.prod_version, "");
	strcpy(dev->descr.rfe_name, "");
	strcpy(dev->descr.rfe_version, "");
	strcpy(dev->descr.fw_version, "");

//TODO Move this section to a separate fn?
		
	// TODO: get & update the versions & wsa model
	// TODO will need to replace with reading from reg or eeprom?
	sprintf(dev->descr.prod_name, "%s", WSA4000);
	strcpy(dev->descr.prod_serial, "TO BE DETERMINED"); // temp for now
	sprintf(dev->descr.prod_version, "v1.0"); // temp value
	sprintf(dev->descr.rfe_name, "%s", WSA_RFE0560);
	sprintf(dev->descr.rfe_version, "v1.0"); // temp
	strcpy(dev->descr.fw_version, "v1.0");

	
	// 3rd, set some values base on the model
	// TODO read from regs/eeprom instead???
	if (strcmp(dev->descr.prod_name, WSA4000) == 0) {
		dev->descr.max_sample_size = WSA4000_MAX_SAMPLE_SIZE;
		dev->descr.inst_bw = (uint64_t) WSA4000_INST_BW;
		
		if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			dev->descr.max_tune_freq = WSA_RFE0560_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
			dev->descr.freq_resolution = WSA_RFE0560_FREQRES;
			dev->descr.max_if_gain = WSA_RFE0560_MAX_IF_GAIN;
			dev->descr.min_if_gain = WSA_RFE0560_MIN_IF_GAIN;
			dev->descr.abs_max_amp[WSA_GAIN_HIGH] = 
				WSA_RFE0560_ABS_AMP_HIGH;
			dev->descr.abs_max_amp[WSA_GAIN_MEDIUM] = 
				WSA_RFE0560_ABS_AMP_MEDIUM;
			dev->descr.abs_max_amp[WSA_GAIN_LOW] = 
				WSA_RFE0560_ABS_AMP_LOW;
			dev->descr.abs_max_amp[WSA_GAIN_VLOW] = 
				WSA_RFE0560_ABS_AMP_VLOW;
		}
	}

	return 0;
}


/**
 * Connect to a WSA through the specified interface method \b intf_method,
 * and communicate control commands in the format of the given command 
 * syntax.
 *
 * @param dev - A pointer to the WSA device structure to be 
 * connected/establised.
 * @param cmd_syntax - A char pointer to store standard for control 
 * commands communication to the WSA. \n 
 * Currently supported standard command syntax type is: SCPI.
 * @param intf_method - A char pointer to store the interface method to the 
 * WSA. \n Possible methods: \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::HISLIP" \n
 * - With USB, use: "USB" (check if supported with the WSA version used)
 * 
 * @return 0 on success, or a negative number on error.
 * TODO: define ERROR values with associated messages....
 */
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method)
{
	int16_t result = 0;			// result returned from a function
	char *temp_str;				// temporary store a string
	const char* wsa_addr;		// store the WSA IP address
	uint8_t is_tcpip = FALSE;	// flag to indicate a TCPIP connection method

	//*****
	// Check the syntax type & interface method & connect base on those info
	//*****
	// When the cmd_syntax is SCPI:
	if (strncmp(cmd_syntax, SCPI, 4) == 0) {
		// If it's a TCPIP connection, get the address
		if (strstr(intf_method, "TCPIP") != NULL) {
			if ((temp_str = strstr(intf_method, "::")) == NULL) {
				doutf(1, "Error WSA_ERR_INVINTFMETHOD: %s \"%s\".\n", 
					wsa_get_err_msg(WSA_ERR_INVINTFMETHOD), intf_method);
				return WSA_ERR_INVINTFMETHOD;
			}

			//Assume right after TCPIP:: is the IP address
			// TODO: there might be a danger here...
			wsa_addr = strtok(temp_str, "::");
			is_tcpip = TRUE;
		}
		
		// If it's USB
		else if (strstr(intf_method, "USB") != NULL) {
			// TODO: add to this section if ever use USB.
			doutf(1, "Error WSA_ERR_USBNOTAVBL: %s.\n", 
				wsa_get_err_msg(WSA_ERR_USBNOTAVBL));
			return WSA_ERR_USBNOTAVBL;	
		}

		// Can't determine connection method from the interface string
		else {
			doutf(1, "Error WSA_ERR_INVINTFMETHOD: %s.\n", 
				wsa_get_err_msg(WSA_ERR_INVINTFMETHOD));
			return WSA_ERR_INVINTFMETHOD;
		}
	} // end if SCPI section

	// When the cmd_syntax is not supported/recognized
	else {
		printf(" ERROR: Command syntax is not recognized/valid!\n");
		return -1;
	}
	
	
	//*****
	// Do the connection
	//*****
	if (is_tcpip) {
		result = wsa_start_client(wsa_addr, &(dev->sock).cmd, 
				&(dev->sock).data);

		if (result < 0) {
			//doutf(1, "Error WSA_ERR_ETHERNETCONNECTFAILED: %s.\n", 
			//		wsa_get_err_msg(WSA_ERR_ETHERNETCONNECTFAILED));
			return WSA_ERR_ETHERNETCONNECTFAILED;
		}

		strcpy(dev->descr.intf_type, "TCPIP");
	}
	
	// TODO Add other methods here

	// Initialize wsa_device structure with the proper values
	if (wsa_dev_init(dev) < 0) {
		doutf(1, "Error WSA_ERR_INITFAILED: "
			"%s.\n", wsa_get_err_msg(WSA_ERR_INITFAILED));
		return WSA_ERR_INITFAILED;
	}

	return 0;
}


/**
 * Close the device connection if one is started, stop any existing data 
 * capture, and perform any necessary clean ups.
 *
 * @param dev - A pointer to the WSA device structure to be closed.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_disconnect(struct wsa_device *dev)
{
	int16_t result = 0;			// result returned from a function
	
	//TODO close based on connection type
	// right now do only client
	if (strcmp(dev->descr.intf_type, "TCPIP") == 0)
		result = wsa_close_client(dev->sock.cmd, dev->sock.data);

	return result;
}


///**
// * List (print out) the IPs of connected WSAs to the network? or the PC???
// * For now, will list the IPs for any of the connected devices to a PC?
// *
// * @param wsa_list - A double char pointer to store (WSA???) IP addresses 
// * connected to a network???.
// *
// * @return Number of connected WSAs (or IPs for now) on success, or a 
// * negative number on error.
// */
// TODO: This section is to be replaced w/ list connected WSAs
int16_t wsa_list_devs(char **wsa_list) 
{
	int16_t result = 0;			// result returned from a function

	result = wsa_list_ips(wsa_list);

	return result;
}


//**
// * Open a file or print the help commands information associated with the 
// * WSA used.
// *
// * @param dev - The WSA device structure from which the help information 
// * will be provided.
// *
// * @return 0 on success, or a negative number on error.
// */
//int16_t wsa_help(struct wsa_device dev)
//{
//	// Open the generic SCPI for now
//	if (_popen("ThinkRF SCPI DS 101202.pdf", "r") == NULL) {
//		printf("ERROR: Failed to opent the SCPI file.\n");
//		return -1;
//	}
//	return 0;
//}


/**
 * Send the control command string to the WSA device specified by \b dev. 
 * The commands format must be written according to the specified 
 * standard syntax in wsa_connect().
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A char pointer to the control command string written 
 * in the format specified by the syntax standard in wsa_connect()
 *
 * @return Number of bytes sent on success, or a negative number on error.
 */
int32_t wsa_send_command(struct wsa_device *dev, char *command)
{
	int32_t bytes_txed = 0;
	uint8_t resend_cnt = 0;
	uint16_t len = strlen(command);

	// TODO: check WSA version/model # ?
	if (strcmp(dev->descr.intf_type, "USB") == 0) {	
		return WSA_ERR_USBNOTAVBL;
	}
	else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
		while (1) {
			bytes_txed = wsa_sock_send(dev->sock.cmd, command, len);
			if (bytes_txed < len) {
				if (resend_cnt > 5)
					return WSA_ERR_CMDSENDFAILED;

				printf("Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			else 
				break;
		}
	}

	return bytes_txed;
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
int16_t wsa_send_command_file(struct wsa_device *dev, char *file_name)
{
	int16_t result = 0;
	int16_t lines = 0;
	char *cmd_strs[MAX_FILE_LINES]; // store user's input words
	FILE *cmd_fptr;

	if((cmd_fptr = fopen(file_name, "r")) == NULL) {
		result = WSA_ERR_FILEREADFAILED;
		printf("ERROR %d: %s '%s'.\n", result, wsa_get_err_msg(result), 
			file_name);
		return result;
	}

	// Allocate memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STR_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	fclose(cmd_fptr);

	if (result < 0) {
		// free memory
		for (int i = 0; i < MAX_FILE_LINES; i++)
			free(cmd_strs[i]);
		return result;
	}

	// Send each command line to WSA
	lines = result;
	for (int i = 0; i < lines; i++) {
		result = wsa_send_command(dev, cmd_strs[i]);
		Sleep(20); // delay the send a little bit
		
		// If a bad command is detected, continue? Prefer not.
		if (result < 0) {
			result = WSA_ERR_CMDINVALID;
			printf("ERROR %d: %s.\n", result, wsa_get_err_msg(result));
			printf("Line %d: '%s'.\n", i + 1, cmd_strs[i]);
			break;
		}
	}
	result = lines;

	// Free memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		free(cmd_strs[i]);

	return result;
}


/**
 * Send query command to the WSA device specified by \b dev. The commands 
 * format must be written according to the specified command syntax 
 * in wsa_connect().
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A char pointer to the query command string written in 
 * the format specified by the command syntax in wsa_connect().
 *
 * @return The result stored in a wsa_resp struct format.
 */
struct wsa_resp wsa_send_query(struct wsa_device *dev, char *command)
{
	struct wsa_resp resp;
	int32_t bytes_got = 0;

	// Send the query command out
	bytes_got = wsa_send_command(dev, command);

	// Receive query result from the WSA server
	if (bytes_got > 0) {
		// TODO: check WSA version/model # ?
		if (strcmp(dev->descr.intf_type, "USB") == 0) {	
			resp.status = WSA_ERR_USBNOTAVBL;
		}
		else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
				bytes_got = wsa_sock_recv(dev->sock.cmd, resp.result, 
					MAX_STR_LEN, TIMEOUT);
		}
	}

	// TODO define what result should be
	resp.status = bytes_got;

	if (bytes_got < 0) {
		resp.result[0] = 0;
	}

	return resp;
}


//TODO: Determine if string should be returned instead.
/**
 * Querry the WSA for any error.
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0 on success, or a negative number on error.
 */
int32_t wsa_query_error(struct wsa_device *dev)
{
	printf("To be added later.\n");

	return 0;
}

/**
 * Reads a frame of data. \e Each frame consists of a header and a 
 * buffer of data of length determined by the \b sample_size parameter 
 * (i.e. sizeof(\b data_buf) = \b sample_size * 4 bytes per sample).
 * 
 * Each I and Q samples is 16-bit wide, signed 2-complement.  The raw \b
 * data_buf contains alternatively 2-byte Q follows by 2-byte I, so on.  In 
 * another words, the I & Q samples are distributed in the \b raw data_buf 
 * as follow:
 *
 * @code 
 *		data_buf = QIQIQIQI... = <2 bytes Q><2 bytes I><...>
 * @endcode
 *
 * The bytes can be decoded, as an example, as follow:
 * @code
 *	Let takes the first 4 bytes of the \b data_buf, then:
 * 
 *		int16_t I = data_buf[3] << 8 + data_buf[2];
 *		int16_t Q = data_buf[1] << 8 + data_buf[0];
 * @endcode
 * 
 * Alternatively, the \b data_buf can be passed to wsa_decode_frame() to have I
 * and Q splitted up and stored into separate int16_t buffers. The 
 * wsa_decode_frame() function is useful for later needs of decoding  the 
 * data byteswhen a massive amount of data (multiple frames) has been 
 * captured for instance.
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
 * @return A 4-bit packet count number that starts at 0, or a 16-bit negative 
 * number on error.
 */
int16_t wsa_get_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 char *data_buf, uint32_t sample_size)
{
	int32_t result = 0;
	int16_t pkt_count = -1, pkt_size;
	uint32_t bytes = (sample_size + VRT_HEADER_SIZE + VRT_TRAILER_SIZE) * 4;
	char *dbuf;
	
	// allocate the data buffer
	dbuf = (char *) malloc(bytes * sizeof(char));

	result = wsa_sock_recv(dev->sock.data, dbuf, bytes, 1000);

	if (result > 0) {
		// set sample size to the header
		header->sample_size = sample_size;

		// *****
		// Handle the 5 header words
		// *****

		// 1. Check the Pkt type & get the Stream identifier word
		//if ((dbuf[3] & 0xF0) == 1) {
		//	memcpy(&result, (dbuf + 4), 4);
		//	if (result != 0x90000003)
		//		return WSA_ERR_NOTIQFRAME;
		//}
		//else what?

		// 2. Get the 4-bit packet count number
		pkt_count = dbuf[2] & 0x0F;
		//printf("Packet #: %d %02x\n", pkt_count, dbuf[2]);

		// 3. Get the 16-bit packet size
		memcpy(&pkt_size, dbuf, 2);
		//printf("Packet size: %d %04x\n", pkt_size, pkt_size);
		// TODO: compare the sample size w/ this pkt_size, less hdr & trailer

		// TODO: how to handle TSI field?
		// 4. Get the second time stamp at the 3rd words
		memcpy(&(header->time_stamp.sec), (dbuf + 8), 4);
		//printf("second: %08x\n", header->time_stamp.sec);

		// 5. Check the TSF field, if presents (= 0x10),
		// get the picoseconds time stamp at the 4th & 5th words
		if ((dbuf[2] & 0x30) == 0x10)
			memcpy(&(header->time_stamp.psec), (dbuf + 12), 8);
		else 
			header->time_stamp.psec = 0;
		//printf("psec: %016llx\n", header->time_stamp.psec);

		// *****
		// Get the data_buf
		// *****
		memcpy(data_buf, dbuf + 20, sample_size * 4);

		// *****
		// TODO: Handle the trailer word. 
		// *****
	}

	free(dbuf);

	return pkt_count;
}


/**
 * Decodes a raw data_buf containing I & Q data and returned the I and Q 
 * buffers of data of length determine by the \b sample_size parameter.  
 * Note: the data_buf size is assumed as \b sample_size * 4 bytes per sample
 *
 * @param dev - A pointer to the WSA device structure.
 * @param data_buf - A char pointer buffer containing the raw I and Q data in
 * in bytes to be decoded into separate I and Q buffers. Its size is assumed to
 * be the number of 32-bit sample_size words multiply by 4 (i.e. 
 * sizeof(data_buf) = sample_size * 4 bytes per sample).
 * @param i_buf - A 16-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the \b sample_size.
 * @param q_buf - A 16-bit signed integer pointer for the unscaled 
 * Q data buffer with size specified by the \b sample_size.
 * @param sample_size - A 64-bit unsigned integer sample size (i.e. {I, Q} 
 * sample pairs) per data frame to be captured. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return the number of samples decoded, or a 16-bit negative 
 * number on error.
 */
int32_t wsa_decode_frame(struct wsa_device *dev, char *data_buf,
				 int16_t *i_buf, int16_t *q_buf, uint32_t sample_size)
{
	int32_t result = 0;
	uint32_t i;
	int j = 0;

	// *****
	// Split up the IQ data bytes
	// *****
	for (i = 0; i < sample_size * 4; i += 4) {
		// Gets the payload, each word = I2I1Q2Q1 bytes
		i_buf[j] = (((uint8_t) data_buf[i + 3]) << 8) + 
					((uint8_t) data_buf[i + 2]); 
		q_buf[j] = (((uint8_t) data_buf[i + 1]) << 8) + 
					(uint8_t) data_buf[i];
		
		/*if ((j % 4) == 0) printf("\n");
		printf("%04x,%04x ", i_buf[j], q_buf[j]);*/
		
		j++;
	}

	return sample_size;
}


