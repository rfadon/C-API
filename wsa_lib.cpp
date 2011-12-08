
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
//#define DUMMY_CONN 0

//TODO create a log file method
//TODO add proper error method


// *****
// Local functions:
// *****
char *wsa_query_error(struct wsa_device *dev);
int16_t _wsa_dev_init(struct wsa_device *dev);
int16_t _wsa_open(struct wsa_device *dev);
int16_t _wsa_query_stb(struct wsa_device *dev, char *output);
int16_t _wsa_query_esr(struct wsa_device *dev, char *output);


// Initialized the \b wsa_device descriptor structure
// Return 0 on success or a 16-bit negative number on error.
int16_t _wsa_dev_init(struct wsa_device *dev)
{
	// Initialized with "null" constants
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
	sprintf(dev->descr.prod_version, "v1.1"); // temp value
	sprintf(dev->descr.rfe_name, "%s", WSA_RFE0560); // TODO read from wsa
	//sprintf(dev->descr.rfe_name, "%s", WSA_RFE0440);
	sprintf(dev->descr.rfe_version, "v1.1"); // temp
	strcpy(dev->descr.fw_version, "v1.1");

	
	// 3rd, set some values base on the model
	// TODO read from regs/eeprom instead???
	if (strcmp(dev->descr.prod_name, WSA4000) == 0) {
		dev->descr.max_sample_size = WSA4000_MAX_SAMPLE_SIZE;
		dev->descr.inst_bw = (uint64_t) WSA4000_INST_BW;
		
		// if it's RFE0440
		if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0) {
			dev->descr.max_tune_freq = WSA_RFE0440_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0440_MIN_FREQ;
			dev->descr.freq_resolution = WSA_RFE0440_FREQRES;
			dev->descr.abs_max_amp[WSA_GAIN_HIGH] = 
				WSA_RFE0440_ABS_AMP_HIGH;
			dev->descr.abs_max_amp[WSA_GAIN_MED] = 
				WSA_RFE0440_ABS_AMP_MED;
			dev->descr.abs_max_amp[WSA_GAIN_LOW] = 
				WSA_RFE0440_ABS_AMP_LOW;
			dev->descr.abs_max_amp[WSA_GAIN_VLOW] = 
				WSA_RFE0440_ABS_AMP_VLOW;
		}

		// if it's RFE0560
		else if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			dev->descr.max_tune_freq = WSA_RFE0560_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
			dev->descr.freq_resolution = WSA_RFE0560_FREQRES;
			dev->descr.max_if_gain = WSA_RFE0560_MAX_IF_GAIN;
			dev->descr.min_if_gain = WSA_RFE0560_MIN_IF_GAIN;
			dev->descr.abs_max_amp[WSA_GAIN_HIGH] = 
				WSA_RFE0560_ABS_AMP_HIGH;
			dev->descr.abs_max_amp[WSA_GAIN_MED] = 
				WSA_RFE0560_ABS_AMP_MED;
			dev->descr.abs_max_amp[WSA_GAIN_LOW] = 
				WSA_RFE0560_ABS_AMP_LOW;
			dev->descr.abs_max_amp[WSA_GAIN_VLOW] = 
				WSA_RFE0560_ABS_AMP_VLOW;
		}
	}

	return 0;
}

// Open the WSA after socket connection is established
int16_t _wsa_open(struct wsa_device *dev) 
{
	int16_t result = 0;
	char output[1024];

	// set "*SRE 252" or 0xFC to enable all usable STB bits?
	// No, shouldn't do this. Will mess up users setting. At power up
	// this reg is default to all enabled any way.

	// go to read STB & handle the response
	result = _wsa_query_stb(dev, output);
	if (result < 0)
		return result;

	// Initialize wsa_device structure with the proper values
	result = _wsa_dev_init(dev);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_INITFAILED: "
			"%s.\n", _wsa_get_err_msg(WSA_ERR_INITFAILED));
		return WSA_ERR_INITFAILED;
	}
	
	return result;
}


// Handle bits status in STB register
int16_t _wsa_query_stb(struct wsa_device *dev, char *output)
{
	int16_t result = 0;
	long temp_val;
	uint8_t stb_reg = 0;
	char *temp;
	struct wsa_resp query;		// store query results

	// initialized the output buf
	strcpy(output, "");
	
	// read "*STB?" for any status bits
	query = wsa_send_query(dev, "*STB?\n");
	if (query.status <= 0)
		return (int16_t) query.status;

	if (to_int(query.output, &temp_val) < 0)
		return WSA_ERR_RESPUNKNOWN;
	stb_reg = (uint8_t) temp_val;

	if (stb_reg & SCPI_SBR_EVTAVL) {
		// loop until output is ""
		do {
			sprintf(output, "%s\n", wsa_query_error(dev));
			if (strcmp(output, "\n") == 0)
				break;
		} while(1);
	}

	if (stb_reg & SCPI_SBR_QSR) {
		//result = wsa_query_qsr(dev);
	}

	if (stb_reg & SCPI_SBR_MSGAVL) {
		// do nothing?
	}

	if (stb_reg & SCPI_SBR_ESR) {
		result = _wsa_query_esr(dev, output);
	}

	if (stb_reg & SCPI_SBR_RQS) {
		//result = wsa_query_esr(dev);
	}

	if (stb_reg & SCPI_SBR_OSR) {
		//result = wsa_query_osr(dev);
	}

	return result;
}

// Handle bits status in ESR register
int16_t _wsa_query_esr(struct wsa_device *dev, char *output)
{
	int16_t result = 0;
	long temp_val;
	uint8_t esr_reg = 0;
	char *temp;
	struct wsa_resp query;		// store query results

	// initialized the output buf
	strcpy(output, "");
	
	// read "*STB?" for any status bits
	query = wsa_send_query(dev, "*ESR?\n");
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the output
	if (to_int(query.output, &temp_val) < 0)
		return WSA_ERR_RESPUNKNOWN;
	esr_reg = (uint8_t) temp_val;

	if (!(esr_reg & SCPI_ESR_OPC)) {
		strcpy(output, "Operation incomplete.\n");
	}

	if (esr_reg & SCPI_ESR_QYE) {
	}

	if (esr_reg & SCPI_ESR_DDE) {
		// do nothing?
	}

	if (esr_reg & SCPI_ESR_EXE) {
	}

	if (esr_reg & SCPI_ESR_CME) {
	}

	if (esr_reg & SCPI_ESR_PON) {
	}

	return result;
}


// Querry the WSA for any error messages.  This is equivalent to the SCPI
// command SYSTem:ERRor?
// Return the query result stored in a char pointer.
char *wsa_query_error(struct wsa_device *dev)
{
	struct wsa_resp resp;

	resp = wsa_send_query(dev, "SYST:ERR?\n");
	if (resp.status < 0)
		return (char *) _wsa_get_err_msg(resp.status);

	if (strstr(resp.output, "No error") != NULL || strcmp(resp.output, "") == 0)
		return "";
	else {
		printf("WSA returns: %s\n", resp.output);
		return resp.output;
	}
}
	

// *****
// Global functions:
// *****


/**
 * Connect to a WSA through the specified interface method \b intf_method,
 * and communicate control commands in the format of the given command 
 * syntax.
 *
 * After successfully connected, this function will also do: \n
 *  - Check for any errors in WSA
 *  - Gather information for the WSA's descriptor
 *
 * @param dev - A pointer to the WSA device structure to be 
 * connected/establised.
 * @param cmd_syntax - A char pointer to store standard for control 
 * commands communication to the WSA. \n 
 * Currently supported standard command syntax type is: SCPI.
 * @param intf_method - A char pointer to store the interface method to the 
 * WSA. \n \n Possible methods: \n
 * - With USB, use: "USB" (check if supported with the WSA version used). \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>[::<cmd port,data port>]".\n
 * The ports' number if not entered will be defaulted to: \n
 *		- command port: 37001 \n
 *		- data port: 37000 \n
 *		.
 * However, if port forwarding method is used to forward different ports to 
 * the required ports eventually, then you can enter the ports in the format
 * and the \e \b order as specified. \n
 * Example: "TCPIP::192.168.1.1" or "TCPIP::192.168.1.1::37001,37001"
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method)
{
	int16_t result = 0;			// result returned from a function
	char *temp_str;		// temporary store a string
	char intf_type[10];
	char ports_str[20];
	char wsa_addr[200];		// store the WSA IP address
	int32_t data_port, ctrl_port;
	uint8_t is_tcpip = FALSE;	// flag to indicate a TCPIP connection method
	int colons = 0;

	// initialed the strings
	strcpy(intf_type, "");
	strcpy(wsa_addr, "");
	strcpy(ports_str, "");

	// Gets the interface strings
	temp_str = strtok(intf_method, ":");
	while (temp_str != NULL) {
		if (colons == 0)	 strcpy(intf_type, temp_str);
		else if (colons == 1)	 strcpy(wsa_addr, temp_str);
		else if (colons == 2)	 strcpy(ports_str, temp_str);
		temp_str = strtok(NULL, ":");
		colons++;
	}

	// *****
	// Check the syntax type & interface method & connect base on those info
	// *****
	// When the cmd_syntax is SCPI:
	if (strncmp(cmd_syntax, SCPI, 4) == 0) {

		// If it's a TCPIP connection, get the address
		if (strstr(intf_type, "TCPIP") != NULL) {

			// if no address available, return error
			if(strlen(wsa_addr) == 0) {
				doutf(DMED, "Error WSA_ERR_INVINTFMETHOD: %s \"%s\".\n", 
					_wsa_get_err_msg(WSA_ERR_INVINTFMETHOD), intf_method);
				return WSA_ERR_INVINTFMETHOD;
			}
			
			is_tcpip = TRUE;
		}
		
		// If it's USB
		else if (strstr(intf_type, "USB") != NULL) {
			// TODO: add to this section if ever use USB.
			doutf(DHIGH, "Error WSA_ERR_USBNOTAVBL: %s.\n", 
				_wsa_get_err_msg(WSA_ERR_USBNOTAVBL));
			return WSA_ERR_USBNOTAVBL;	
		}

		// Can't determine connection method from the interface string
		else {
			doutf(DMED, "Error WSA_ERR_INVINTFMETHOD: %s.\n", 
				_wsa_get_err_msg(WSA_ERR_INVINTFMETHOD));
			return WSA_ERR_INVINTFMETHOD;
		}
	} // end if SCPI section

	// When the cmd_syntax is not supported/recognized
	else {
		printf(" ERROR: Command syntax is not recognized/valid!\n");
		return -1;
	}
	
	
	// *****
	// Do the connection
	// *****
	if (is_tcpip) {
		long temp;

		// extract the ports if they exist
		if (strlen(ports_str) > 0)	{
			// get control port
			temp_str = strtok(ports_str, ",");
			if (to_int(temp_str, &temp) < 0)
				return WSA_ERR_INVNUMBER;
			else 
				ctrl_port = (int32_t) temp;
			
			// get data port
			temp_str = strtok(NULL, ",");
			if (to_int(temp_str, &temp) < 0)
				return WSA_ERR_INVNUMBER;
			else 
				data_port = (int32_t) temp;
		}
		else {
			ctrl_port = CTRL_PORT;
			data_port = DATA_PORT;
		}
		doutf(DLOW, "%d %d\n", ctrl_port, data_port);

		// Do the connection
		result = wsa_start_client(wsa_addr, &(dev->sock).cmd, 
				&(dev->sock).data, ctrl_port, data_port);
		if (result < 0) {
			doutf(DMED, "Error %d: %s.\n", result, _wsa_get_err_msg(result));
			return result;
		}

		strcpy(dev->descr.intf_type, "TCPIP");
	}
	
	// TODO Add other connection methods here...

	// *****
	// Check for any errors exist in the WSA
	// *****
	result = _wsa_open(dev);
	
	return result;
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


////**
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
 * @remarks To send query command, use wsa_send_query() instead.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A char pointer to the control command string written 
 * in the format specified by the syntax standard in wsa_connect()
 *
 * @return Number of bytes sent on success, or a negative number on error.
 */
int16_t wsa_send_command(struct wsa_device *dev, char *command)
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
			if (bytes_txed < 0)
				return bytes_txed;
			else if (bytes_txed < len) {
				if (resend_cnt > 3)
					return WSA_ERR_CMDSENDFAILED;

				printf("Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			else 
				break;
		}

		// If it's not asking for data, query for any error to
		// make sure that the set is done w/out any error in the system
		if (strstr(command, "IQ?") == NULL)
			if ((strstr(wsa_query_error(dev), "no response") != 0) && 
				(bytes_txed > 0))
				return WSA_ERR_QUERYNORESP;

			if (strcmp(wsa_query_error(dev), "") != 0) {
				printf("%s", wsa_query_error(dev));
				return WSA_ERR_SETFAILED;
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
	struct wsa_resp resp;
	int64_t result = 0;
	int16_t lines = 0;
	char *cmd_strs[MAX_FILE_LINES]; // store user's input words
	FILE *cmd_fptr;

	// set defaults
	strcpy(resp.output, "");
	resp.status = 0;

	if((cmd_fptr = fopen(file_name, "r")) == NULL) {
		result = WSA_ERR_FILEREADFAILED;
		printf("ERROR %lld: %s '%s'.\n", result, 
			_wsa_get_err_msg((int16_t) result), file_name);
		return (int16_t) result;
	}

	// Allocate memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STR_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	fclose(cmd_fptr);

	if (result > 0) {
		// Send each command line to WSA
		lines = (int16_t) result;
		for (int i = 0; i < lines; i++) {
			// Send non-query cmds
			if (strstr(cmd_strs[i], "?") == NULL)
				result = wsa_send_command(dev, cmd_strs[i]);
			// Send query cmds
			else {
				resp = wsa_send_query(dev, cmd_strs[i]);
				result = resp.status;
			}
			
			// If a bad command is detected, continue? Prefer not.
			if (result < 0) {
				//result = WSA_ERR_CMDINVALID;
				//printf("Error WSA_ERR_CMDINVALID: \"%s\".\n", 
				//	_wsa_get_err_msg((int16_t) result));
				printf("Error %d: \"%s\" (possibly: %s)\n", resp.output, 
					_wsa_get_err_msg(WSA_ERR_CMDINVALID));
				printf("Line %d: '%s'.\n", i + 1, cmd_strs[i]);
				break;
			}
			// TODO how to handle response result
			else if (strstr(cmd_strs[i], "?") != NULL) {
				printf("\"%s\": %s\n", cmd_strs[i], resp.output);
				result = lines;
			}
		}
	}

	// Free memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		free(cmd_strs[i]);


	return (int16_t) result;
}


/**
 * Send query command to the WSA device specified by \b dev. The commands 
 * format must be written according to the specified command syntax 
 * in wsa_connect() (ex. SCPI).
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
	uint8_t resend_cnt = 0;
	uint16_t len = strlen(command);
	int loop_count = 0;

	// set defaults
	strcpy(resp.output, "");
	resp.status = 0;

	if (strcmp(dev->descr.intf_type, "USB") == 0) {	
		resp.status = WSA_ERR_USBNOTAVBL;
		strcpy(resp.output, _wsa_get_err_msg(WSA_ERR_USBNOTAVBL));
	}
	else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
		while (1) {
			// Send the query command out
			bytes_got = wsa_sock_send(dev->sock.cmd, command, len);
			if (bytes_got < 0) {
				resp.status = bytes_got;
				strcpy(resp.output, 
					_wsa_get_err_msg(bytes_got));
				return resp;
			}
			else if (bytes_got < len) {
				if (resend_cnt > 3) {
					resp.status = WSA_ERR_CMDSENDFAILED;
					strcpy(resp.output, 
						_wsa_get_err_msg(WSA_ERR_CMDSENDFAILED));
					return resp;
				}

				printf("Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			// Read back the output
			else {
				do {
					bytes_got = wsa_sock_recv(dev->sock.cmd, resp.output, 
						MAX_STR_LEN, TIMEOUT);
					if (bytes_got > 0)
						break;

					// Loop to make sure received bytes
					if (loop_count == 5)
						break;
					loop_count++;
				} while (bytes_got < 1);

				resp.output[bytes_got] = 0; // add EOL to the string
				break;
			}
		}

		// TODO define what result should be
		if (bytes_got == 0)
			resp.status = WSA_ERR_QUERYNORESP;
		else 
			resp.status = bytes_got;
	}

	return resp;
}


/**
 * Query the status of the WSA box for any event and store the output 
 * response(s) in the \b output parameter.  
 * @remarks This function is equivalent to the SCPI command "*STB?".
 *
 * @param dev
 * @param output - a char pointer to the output result message of the query
 * 
 * @return 0 if successfully queried, or a negative number upon errors.
 */
int16_t wsa_read_status(struct wsa_device *dev, char *output)
{
	int16_t result = 0;

	result = _wsa_query_stb(dev, output);

	return result;
}


/**
 * Returns a message string associated with the given \b err_code that is
 * returned from a \b wsa_lib function.
 * 
 * @param err_code - The negative WSA error code, returned from a WSA function.
 *
 * @return A char pointer to the error message string.
 */
const char *wsa_get_error_msg(int16_t err_code)
{
	return _wsa_get_err_msg(err_code);
}

/**
 * Reads a frame of data. \e Each frame consists of a header and a 
 * buffer of data of length determined by the \b sample_size parameter 
 * (i.e. sizeof(\b data_buf) = \b sample_size * 4 bytes per sample).
 * 
 * Each I and Q samples is 16-bit (2-byte) wide, signed 2-complement.  The raw 
 * data_buf contains alternatively 2-byte Q follows by 2-byte I, so on.  In 
 * another words, the I & Q samples are distributed in the raw data_buf 
 * as follow:
 *
 * @code 
 *		data_buf = IQIQIQIQ... = <2 bytes I><2 bytes Q><...>
 * @endcode
 *
 * The bytes can be decoded, as an example, as follow:
 * @code
 *	Let takes the first 4 bytes of the \b data_buf, for example, then:
 * 
 *		int16_t I = data_buf[3] << 8 + data_buf[2];
 *		int16_t Q = data_buf[1] << 8 + data_buf[0];
 *
 *	And so on for N number of samples:
 *
 *		int16_t I[i] = data_buf[i+3] << 8 + data_buf[i+2];
 *		int16_t Q[i] = data_buf[i+1] << 8 + data_buf[i];
 *
 *	where i = 0, 1, 2, ..., (N - 2), (N - 1).
 * @endcode
 * 
 * Alternatively, the \b data_buf can be passed to wsa_decode_frame() to have I
 * and Q splitted up and stored into separate int16_t buffers. The 
 * wsa_decode_frame() function is useful for later needs of decoding the 
 * data bytes when a large amount of raw data (multiple frames) has been 
 * captured for instance. 
 * 
 * @remarks This function does not set the \b sample_size to WSA at each 
 * capture in order to minimize the delay between captures.  The number of 
 * samples per frame must be sent to WSA at least once during the WSA 
 * powered on.  For example, with SCPI, do: @code
 * wsa_send_command(dev, "TRACE:IQ:POINTS 1024\n"); @endcode
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
 * @param time_out - The time, in milliseconds, to wait for a packet from
 * a WSA before time out.
 *
 * @return A 4-bit frame count number that starts at 0, or a 16-bit negative 
 * number on error.
 */
int16_t wsa_read_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 char *data_buf, uint32_t sample_size, uint32_t time_out)
{
	int32_t result = 0;
	uint16_t frame_count = 0, frame_size;
	uint32_t bytes = (sample_size + VRT_HEADER_SIZE + VRT_TRAILER_SIZE) * 4;
	char *dbuf;
	
	// allocate the data buffer
	dbuf = (char *) malloc(bytes * sizeof(char));

	// reset header ???
	header->sample_size = 0;
	header->time_stamp.sec = 0;
	header->time_stamp.psec = 0;

	// go get the required bytes
	result = wsa_sock_recv_data(dev->sock.data, dbuf, bytes, time_out);
	doutf(DLOW, "@lib: %ld\n", result);
	if (result < 0)
		return WSA_ERR_READFRAMEFAILED;

	if (result > 0) {
		// set sample size to the header
		header->sample_size = (result/4) - VRT_HEADER_SIZE - VRT_TRAILER_SIZE;

		// *****
		// Handle the 5 header words
		// *****
#ifndef DUMMY_CONN
		// 1. Check "Pkt Type" & get the Stream identifier word
		if ((dbuf[0] & 0xf0) == 0x10) {
			result = (((uint8_t) dbuf[4]) << 24) + (((uint8_t) dbuf[5]) << 16) 
					+ (((uint8_t) dbuf[6]) << 8) + (uint8_t) dbuf[7];
			if (result != 0x90000003)
				return WSA_ERR_NOTIQFRAME;
		}

		// Class identifier C?
		// If 1, returns the data pf class c type?

		// 2. Get the 4-bit "Pkt Count" number
		frame_count = dbuf[1] & 0x0f;
		doutf(DLOW, "Packet count: %d 0x%02x\n", frame_count, frame_count);

		// 3. Get the 16-bit "Pkt Size"
		frame_size = (((uint8_t) dbuf[2]) << 8) + (uint8_t) dbuf[3];
		doutf(DLOW, "Packet size: 0x%04x %d\n", frame_size, frame_size);
		// TODO: compare the sample size w/ this frame_size, less hdr & trailer

		// 4. Check TSI field for 01 & get sec time stamp at the 3rd word
		if (!((dbuf[1] & 0xC0) >> 6))
			printf("ERROR: Second timestamp is not of UTC type.\n");
		header->time_stamp.sec = (((uint8_t) dbuf[8]) << 24) +
								(((uint8_t) dbuf[9]) << 16) +
								(((uint8_t) dbuf[10]) << 8) + 
								(uint8_t) dbuf[11];
		doutf(DLOW, "second: 0x%08x %ld\n", header->time_stamp.sec, 
			header->time_stamp.sec);

		// 5. Check the TSF field, if presents (= 0x10), 
		// get the picoseconds time stamp at the 4th & 5th words
		if ((dbuf[1] & 0x30) >> 5)
			header->time_stamp.psec = (uint64_t)
					((((uint8_t) dbuf[12]) & 0x0100000000000000) +
					(((uint8_t) dbuf[13]) & 0x01000000000000) +
					(((uint8_t) dbuf[14]) & 0x010000000000) +
					(((uint8_t) dbuf[15]) & 0x0100000000) +
					(uint32_t) (((uint8_t) dbuf[16]) << 24) +	// u32 shouldn't be there?
					(((uint8_t) dbuf[17]) << 16) + 
					(((uint8_t) dbuf[18]) << 8) + 
					(uint8_t) dbuf[19]);
		else 
			header->time_stamp.psec = 0;
		doutf(DLOW, "psec: 0x%016llx %lld\n", header->time_stamp.psec, 
			header->time_stamp.psec);
#endif
		// *****
		// Get the data_buf
		// *****
		memcpy(data_buf, dbuf + 20, sample_size * 4);

		// *****
		// TODO: Handle the trailer word. 
		// *****
		//if (dbuf[0] & 0x04)
			// handle the trailer here
	}

	free(dbuf);

	return frame_count;
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
int32_t wsa_decode_frame(char *data_buf, int16_t *i_buf, int16_t *q_buf, 
						 uint32_t sample_size)
{
	int32_t result = 0;
	uint32_t i;
	int j = 0;

	// *****
	// Split up the IQ data bytes
	// *****
	for (i = 0; i < sample_size * 4; i += 4) {
		// Gets the payload, each word = I2I1Q2Q1 bytes
		// WARNING: note the IQ is swapped!!! :(
		q_buf[j] = (((uint8_t) data_buf[i]) << 8) + ((uint8_t) data_buf[i + 1]); 
		i_buf[j] = (((uint8_t) data_buf[i + 2]) << 8) + (uint8_t) data_buf[i + 3];

		// Do the signed 2-complement on the 14-bit sample
		if (i_buf[j] > 8191) i_buf[j] -= 16384;
		if (q_buf[j] > 8191) q_buf[j] -= 16384;
		
		/*if ((j % 4) == 0) printf("\n");
		printf("%04x,%04x ", i_buf[j], q_buf[j]);*/
		
		j++;
	}

	return sample_size;
}

