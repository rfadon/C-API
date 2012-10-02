#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_lib.h"


//*****
// LOCAL DEFINES
//*****
//#define DUMMY_CONN 0


// *****
// Local functions:
// *****
//char *wsa_query_error(struct wsa_device *dev);
int16_t wsa_query_error(struct wsa_device *dev, char *output);
int16_t _wsa_dev_init(struct wsa_device *dev);
int16_t _wsa_open(struct wsa_device *dev);
int16_t _wsa_query_stb(struct wsa_device *dev, char *output);
int16_t _wsa_query_esr(struct wsa_device *dev, char *output);
int16_t copy_sweep_data(uint8_t* data_buf, uint8_t* sweep_data_buf, int16_t size); 
int16_t extract_reciever_packet_data(uint8_t* temp_buffer, struct wsa_reciever_packet* const reciever);
int16_t extract_digitizer_packet_data(uint8_t* temp_buffer,struct wsa_digitizer_packet* const digitizer);



// Initialized the \b wsa_device descriptor structure
// Return 0 on success or a 16-bit negative number on error.
int16_t _wsa_dev_init(struct wsa_device *dev)
{
	int i;

	// Initialized with "null" constants
	dev->descr.inst_bw = 0;
	dev->descr.max_sample_size = 0;
	dev->descr.max_tune_freq = 0;
	dev->descr.min_tune_freq = 0;
	dev->descr.freq_resolution = 0;
	dev->descr.max_if_gain = -1000;	// some impossible #
	dev->descr.min_if_gain = -1000;	// some impossible #
	dev->descr.max_decimation = -1;
	dev->descr.min_decimation = -1;
	for (i = 0; i < NUM_RF_GAINS; i++)
		dev->descr.abs_max_amp[i] = -1000;	// some impossible #

	strcpy(dev->descr.prod_name, "");
	strcpy(dev->descr.prod_serial, ""); 
	strcpy(dev->descr.prod_version, "");
	strcpy(dev->descr.rfe_name, "");
	strcpy(dev->descr.rfe_version, "");
	strcpy(dev->descr.fw_version, "");
	
	// TODO get & update the versions & wsa model
	// TODO will need to replace with reading from reg or eeprom?
	sprintf(dev->descr.prod_name, "%s", WSA4000);
	strcpy(dev->descr.prod_serial, "TO BE DETERMINED"); // temp for now
	sprintf(dev->descr.prod_version, "v2.3.0"); // temp value
	sprintf(dev->descr.rfe_name, "%s", WSA_RFE0560); // TODO read from wsa
	sprintf(dev->descr.rfe_version, "v2.3.0"); // temp
	strcpy(dev->descr.fw_version, "v2.3.0");

	
	// 3rd, set some values base on the model
	// TODO: read from regs/eeprom instead once available
	if (strcmp(dev->descr.prod_name, WSA4000) == 0) {
		dev->descr.max_sample_size = WSA4000_MAX_SAMPLE_SIZE;
		dev->descr.inst_bw = (uint64_t) WSA4000_INST_BW;
		
		// if it's RFE0440
		if (strcmp(dev->descr.rfe_name, WSA_RFE0440) == 0) {
			dev->descr.max_tune_freq = (uint64_t) WSA_RFE0440_MAX_FREQ;
			dev->descr.min_tune_freq = (uint64_t) WSA_RFE0440_MIN_FREQ;
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
			dev->descr.max_tune_freq = (uint64_t) (WSA_RFE0560_MAX_FREQ * MHZ);
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
			dev->descr.freq_resolution = WSA_RFE0560_FREQRES;
			dev->descr.max_if_gain = WSA_RFE0560_MAX_IF_GAIN;
			dev->descr.min_if_gain = WSA_RFE0560_MIN_IF_GAIN;
			dev->descr.max_decimation = WSA_RFE0560_MAX_DECIMATION;
			dev->descr.min_decimation = WSA_RFE0560_MIN_DECIMATION;
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
	struct wsa_resp query;		// store query results
	char query_msg[256];

	// initialized the output buf
	strcpy(output, "");
	
	// read "*STB?" for any status bits
	wsa_send_query(dev, "*STB?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	if (to_int(query.output, &temp_val) < 0)
		return WSA_ERR_RESPUNKNOWN;
	stb_reg = (uint8_t) temp_val;

	if (stb_reg & SCPI_SBR_EVTAVL) {
		// loop until output is ""
		do {
			wsa_query_error(dev, query_msg);
			sprintf(output, "%s\n", query_msg);
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
	struct wsa_resp query;		// store query results

	// initialized the output buf
	strcpy(output, "");
	
	// read "*STB?" for any status bits
	wsa_send_query(dev, "*ESR?\n", &query);
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
int16_t wsa_query_error(struct wsa_device *dev, char *output)
{
	struct wsa_resp resp;

	wsa_send_query(dev, "SYST:ERR?\n", &resp);
	if (resp.status < 0)
	{
		strcpy(output, _wsa_get_err_msg((int16_t) resp.status));
		return (int16_t) resp.status;
	}

	if (strstr(resp.output, "No error") != NULL || strcmp(resp.output, "") == 0)
	{
		strcpy(output, "");
	}
	else 
	{
		printf("WSA returns: %s\n", resp.output);
		strcpy(output, resp.output); // TODO verify this output
	}

	return 0;
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
	char data_port[10];
	char ctrl_port[10];

	uint8_t is_tcpip = FALSE;	// flag to indicate a TCPIP connection method
	int32_t colons = 0;

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
		wsa_initialize_client();

		// extract the ports if they exist
		if (strlen(ports_str) > 0)	{
			// get control port
			temp_str = strtok(ports_str, ",");
			strcpy(ctrl_port, temp_str);
			
			// get data port
			temp_str = strtok(NULL, ",");
			strcpy(data_port, temp_str);
		}
		else {
			strcpy(ctrl_port, CTRL_PORT);
			strcpy(data_port, DATA_PORT);
		}
		doutf(DLOW, "%s %s\n", ctrl_port, data_port);

		// setup command socket & connect
		result = wsa_setup_sock("WSA 'command'", wsa_addr, &(dev->sock).cmd, 
			ctrl_port);
		if (result < 0)
			return result;

		// setup data socket & connect
		result = wsa_setup_sock("WSA 'data'", wsa_addr, &(dev->sock).data, 
			data_port);
		if (result < 0)
			return result;

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
	// right now do only TCPIP client
	if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
		result = wsa_close_sock(dev->sock.cmd);
		result = wsa_close_sock(dev->sock.data);

		wsa_destroy_client();
	}

	return result;
}


/** TODO redefine this
 * Given an address string, determine if it's a dotted-quad IP address
 * or a domain address.  If the latter, ask DNS to resolve it.  In
 * either case, return resolved IP address.  If we fail, we return
 * INADDR_NONE.
 *
 * @param sock_addr - 
 *
 * @return Resolved IP address or INADDR_NONE when failed.
 */
int16_t wsa_verify_addr(const char *sock_addr, const char *sock_port) 
{
	int16_t result;

	wsa_initialize_client();
	result = wsa_addr_check(sock_addr, sock_port);
	wsa_destroy_client();

	return result;
}


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
	int16_t bytes_txed = 0;
	uint8_t resend_cnt = 0;
	int32_t len = strlen(command);
	char query_msg[MAX_STR_LEN];

	// TODO: check WSA version/model # 
	if (strcmp(dev->descr.intf_type, "USB") == 0) {	
		return WSA_ERR_USBNOTAVBL;
	}
	else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
		while (1) {
			// Making the assumption that we will not send more bytes
			// than can fit into int16_t
			// TODO: revisit this and move bytes_txed into the parameter list
			bytes_txed = (int16_t) wsa_sock_send(dev->sock.cmd, command, len);
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
		if (strstr(command, "DATA?") == NULL) {
			wsa_query_error(dev, query_msg);
			if ((strstr(query_msg, "no response") != 0) && (bytes_txed > 0)) {
				return WSA_ERR_QUERYNORESP;
			}

			
			if (strcmp(query_msg, "") != 0) {
				if (strstr(query_msg, "-221") != NULL) {
					return WSA_WARNING_TRIGGER_CONFLICT;
				}
				return WSA_ERR_SETFAILED;
			}
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
	int16_t result = 0;
	int16_t lines = 0;
	char *cmd_strs[MAX_FILE_LINES]; // store user's input words
	FILE *cmd_fptr;
	int i;

	// set defaults
	strcpy(resp.output, "");
	resp.status = 0;

	if((cmd_fptr = fopen(file_name, "r")) == NULL) {
		result = WSA_ERR_FILEREADFAILED;
		printf("ERROR %d: %s '%s'.\n", result, wsa_get_error_msg(result), 
			file_name);
		return result;
	}

	// Allocate memory
	for (i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STR_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	// don't need command file any more
	fclose(cmd_fptr);

	// process the command strings acquired
	if (result > 0) {
		// Send each command line to WSA
		lines = result;
		for (i = 0; i < lines; i++) {
			// Send non-query cmds
			if (strstr(cmd_strs[i], "?") == NULL) {
				result = wsa_send_command(dev, cmd_strs[i]);
				// If a bad command is detected, continue? Prefer not.
				if (result < 0) {
					printf("Error at line %d: '%s'.\n", i + 1, cmd_strs[i]);
					break;
				}
			}
			
			// Send query cmds
			else {
				 result = wsa_send_query(dev, cmd_strs[i], &resp);			
			
				// If a bad command is detected, continue? Prefer not.
				if (resp.status < 0) {
					printf("WSA returned error %lld: \"%s\" (possibly: %s)\n", 
						resp.status, resp.output, 
						_wsa_get_err_msg(WSA_ERR_CMDINVALID));
					printf("Line %d: '%s'.\n", i + 1, cmd_strs[i]);
					break;
				}

				printf("\"%s\": %s\n", cmd_strs[i], resp.output);
				result = lines;
			}
		}
	}

	// Free memory
	for (i = 0; i < MAX_FILE_LINES; i++)
		free(cmd_strs[i]);

	return result;
}


/**
 * Send query command to the WSA device specified by \b dev. The commands 
 * format must be written according to the specified command syntax 
 * in wsa_connect() (ex. SCPI).
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A char pointer to the query command string written in 
 * the format specified by the command syntax in wsa_connect().
 * @param resp - A pointer to \b wsa_resp struct to store the responses.
 *
 * @return 0 upon successful or a negative value
 */
int16_t wsa_send_query(struct wsa_device *dev, char *command, 
						struct wsa_resp *resp)
{
	struct wsa_resp temp_resp;
	int16_t bytes_got = 0;
	int16_t recv_result = 0;
	int32_t bytes_received = 0;
	uint8_t resend_cnt = 0;
	int32_t len = strlen(command);
	int32_t loop_count = 0;

	// set defaults
	strcpy(temp_resp.output, "");
	temp_resp.status = 0;

	if (strcmp(dev->descr.intf_type, "USB") == 0) {	
		temp_resp.status = WSA_ERR_USBNOTAVBL;
		strcpy(temp_resp.output, _wsa_get_err_msg(WSA_ERR_USBNOTAVBL));
	}
	else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
		while (1) {
			// Send the query command out
			// Making the assumption that we will not send more bytes
			// than can fit into int16_t
			// TODO: revisit this and move bytes_txed into the parameter list
			bytes_got = (int16_t) wsa_sock_send(dev->sock.cmd, command, len);
			if (bytes_got < 0) {
				temp_resp.status = bytes_got;
				strcpy(temp_resp.output, 
					_wsa_get_err_msg(bytes_got));
				return bytes_got;
			}
			else if (bytes_got < len) {
				if (resend_cnt > 3) {
					temp_resp.status = WSA_ERR_CMDSENDFAILED;
					strcpy(temp_resp.output, 
						_wsa_get_err_msg(WSA_ERR_CMDSENDFAILED));
					return WSA_ERR_CMDSENDFAILED;
				}

				doutf(DMED, "Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			// Read back the output
			else {
				recv_result = -1;
				while (recv_result != 0 && loop_count < 5) {
					recv_result = wsa_sock_recv(dev->sock.cmd, 
							(uint8_t*) temp_resp.output, 
							MAX_STR_LEN, TIMEOUT, 
							&bytes_received);

					loop_count++;
				}

				if (recv_result == 0 && bytes_received < MAX_STR_LEN) {
					temp_resp.output[bytes_received] = '\0'; // add EOL to the string
				}
				else {
					temp_resp.output[MAX_STR_LEN - 1] = '\0'; // add EOL to the string
				}

				break;
			}
		}

		// TODO define what result should be
		if (recv_result != 0) {
			temp_resp.status = WSA_ERR_QUERYNORESP;
			return WSA_ERR_QUERYNORESP;
		}
		else {
			temp_resp.status = bytes_received;
		}
	}

	*resp = temp_resp;

	return 0;
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
 * Reads one VRT packet containing raw IQ data or a Context Packet.
 *if a Context Packet is detected, the information inside the packet will be returned
 *inside the reciever structure and the digitizer structure.
 *
 *if an IQ Packet is detected, the IQ data will be returned
 * Each packet consists of a header, a data payload, and a trailer.
 * The number of samples expected in the packet is indicated by
 * the \b samples_per_packet parameter.
 * 
 * Each I and Q sample is a 16-bit (2-byte) signed 2-complement integer.
 * The \b data_buffer will be populated with alternatively 2-byte I
 * followed by 2-byte Q, and so on.  In another words, \b data_buffer
 * will contain:
 *
 * @code 
 *		data_buffer = IQIQIQIQ... = <2 bytes I><2 bytes Q><...>
 * @endcode
 *
 * The bytes can be decoded like this:
 * @code
 *  Let's takes the first 4 bytes of \b data_buffer
 * 
 *		int16_t I = data_buffer[0] << 8 + data_buffer[1];
 *		int16_t Q = data_buffer[2] << 8 + data_buffer[3];
 *
 *	And so on for N number of samples:
 *
 *		int16_t I[i] = data_buffer[i] << 8 + data_buffer[i + 1];
 *		int16_t Q[i] = data_buffer[i + 2] << 8 + data_buffer[i + 3];
 *
 *	where i = 0, 1, 2, ..., (N - 2), (N - 1).
 * @endcode
 * 
 * @remarks This function does not set the \b samples_per_packet on the WSA.
 * It is the caller's responsibility to configure the WSA with the correct 
 * \b samples_per_packet before initiating the capture.  For example, with SCPI, do:
 * @code
 * wsa_send_command(dev, "TRACE:SPPACKET 1024\n");
 * @endcode
 *
 * @param device - A pointer to the WSA device structure.
 * @param header - A pointer to \b wsa_vrt_packet_header structure to store 
 *		the VRT header information
 * @param trailer - A pointer to \b wsa_vrt_packet_trailer structure to store 
 *		the VRT trailer information
 *@param reciever - a pointer to \b wsa_reciever_packet strucuture to store
 *		the Context data
 *@param digitizer - a pointer to \b wsa_digitizer_packet strucuture to store
 *		the Context data
 * @param data_buffer - A uint8_t pointer buffer to store the raw I and Q data
 *		in bytes. Its size is determined by the number of 
 *		16-bit \b samples_per_packet words multiplied by 4 
 *		(i.e. sizeof(\b data_buffer) = \b samples_per_packet * 4 bytes per sample).
 * @param samples_per_packet - A 16-bit unsigned integer sample size (i.e. number of
 *		{I, Q} sample pairs) per VRT packet to be captured.
 *
 * @return  0 on success or a negative value on error
 */
int16_t wsa_read_iq_packet_raw(struct wsa_device* const device, 
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		struct wsa_reciever_packet* const reciever,
		struct wsa_digitizer_packet* const digitizer,
		uint8_t* const data_buffer, 
		uint16_t* samples_per_packet,
		uint8_t* context_present)
{
	
	uint8_t* vrt_header_buffer = 0;
	uint8_t* temp_buffer=0;
	uint8_t packet_order_indicator = 0;
	uint16_t temp_size=0;
	int32_t temp_size_bytes=0;
	uint16_t expected_header_size = 2;
	int32_t vrt_header_bytes = expected_header_size * BYTES_PER_VRT_WORD;
	uint8_t* vrt_packet_buffer = 0;
	uint16_t expected_packet_size = (samples_per_packet + VRT_HEADER_SIZE + VRT_TRAILER_SIZE-2);
	int32_t vrt_packet_bytes = expected_packet_size * BYTES_PER_VRT_WORD;
	int32_t context_indicator_field = 0;
	int32_t bytes_received = 0;
	int16_t socket_receive_result = 0;
	int16_t result = 0;
	int64_t frequency1 = 0;
	uint32_t stream_identifier_word = 0;
	uint16_t packet_size = 0;
	uint16_t sweep_samples_per_packet = 1;
	uint8_t* sweep_data_buffer = 0;
	FILE * pFile;
	//FILE* example;
	//char buffer[] = { 'x' , 'y' , 'z' };
	//pFile = fopen ( "myfile.bin" , "ab+" );

	//allocate space for the header buffer
	vrt_header_buffer = (uint8_t*) malloc(vrt_header_bytes * sizeof(uint8_t));
		if (vrt_header_buffer == NULL)
	{
		return WSA_ERR_MALLOCFAILED;
	}

	// reset header
	header->packet_order_indicator = 0;
	header->samples_per_packet = 0;
	header->time_stamp.sec = 0;
	header->time_stamp.psec = 0;

	//1) retrieve the first two words of the packet to determine if the packet contains IQ data or context data
	socket_receive_result = wsa_sock_recv_data(device->sock.data, vrt_header_buffer, vrt_header_bytes, TIMEOUT, &bytes_received);
	//fwrite (vrt_header_buffer , 1 , vrt_header_bytes, pFile );
	
	doutf(DMED, "In wsa_read_iq_packet_raw: wsa_sock_recv_data returned %hd\n", socket_receive_result);

	if (socket_receive_result < 0)
	{
		doutf(DHIGH, "Error in wsa_read_iq_packet_raw:  %s\n", wsa_get_error_msg(socket_receive_result));
		free(vrt_header_buffer);
		return socket_receive_result;
	}

		//Store the Stream Identifier to determine if the packet is an IQ packet or a context packet
	stream_identifier_word = (((uint32_t) vrt_header_buffer[4]) << 24) 
			+ (((uint32_t) vrt_header_buffer[5]) << 16) 
			+ (((uint32_t) vrt_header_buffer[6]) << 8) 
			+ (uint32_t) vrt_header_buffer[7];		
	

	//TODO: Fix this not responding properly
	////determine if the packet is neither context or iq	 using the pkt type
	//if((vrt_header_buffer[0] & 0xf0) != 0x40 &&(vrt_header_buffer[0] & 0xf0) !=0x10){
	//		free(vrt_packet_buffer);
	//		printf("pkt ype is: %x \n", vrt_header_buffer[0] & 0xf0); 
	//		
	//		return WSA_ERR_NOTIQFRAME;
	//}


	if (stream_identifier_word != 0x90000001 && stream_identifier_word != 0x90000002 && stream_identifier_word != 0x90000003)
	{
		
		//fclose (pFile);
		free(vrt_packet_buffer);
		return WSA_ERR_NOTIQFRAME;
	}

		//Determine if this is a Context packet
	else if ((stream_identifier_word == 0x90000001 || stream_identifier_word == 0x90000002)) {
		
			//retrieve the packet size
		packet_size = (((uint16_t) vrt_header_buffer[2]) << 8) + (uint16_t) vrt_header_buffer[3];	

		//allocate memory for the context packet
		temp_size_bytes = (packet_size-2)*4;
		temp_buffer = (uint8_t*) malloc(temp_size_bytes * sizeof(uint8_t));
	
		if (temp_buffer == NULL)
		{
			return WSA_ERR_MALLOCFAILED;
		}
		//store the rest of the packet inside the buffer
		socket_receive_result = wsa_sock_recv_data(device->sock.data, temp_buffer, temp_size_bytes, TIMEOUT, &bytes_received);
		//	fwrite (temp_buffer ,1 , temp_size_bytes , pFile );
			//fclose (pFile);


		//store reciever data in the reciever structure
		if (stream_identifier_word == 0x90000001) {
			*context_present =1;
			result = extract_reciever_packet_data(temp_buffer, reciever);

			//store digitizer data in the digitizer structure
		} else if (stream_identifier_word == 0x90000002) {
			*context_present =2;
			result = extract_digitizer_packet_data(temp_buffer,digitizer);
		}
		free(vrt_packet_buffer);
		free(temp_buffer);
		free(vrt_header_buffer);
		return 0;
	
	//if the packet is an IQ packet proceed with the method from previous release
	} else if (stream_identifier_word == 0x90000003){ 

			*context_present =0;

		//  Get the 4-bit "Pkt Count" number, referred to here as "packet_order_indicator"
		// This counter increments from 0 to 15 and repeats again from 0 in a never-ending loop.
		// It provides a simple verification that packets are arriving in the right order
		packet_order_indicator = (uint8_t) vrt_header_buffer[1] & 0x0f;
		
		doutf(DLOW, "Packet order indicator: %hu 0x%02X\n", packet_order_indicator, packet_order_indicator);
		header->packet_order_indicator = packet_order_indicator;

		//Get the 16-bit "Pkt Size"
		packet_size = (((uint16_t) vrt_header_buffer[2]) << 8) + (uint16_t) vrt_header_buffer[3];
		doutf(DLOW, "Packet size: %hu 0x%04X\n", packet_size, packet_size);
		sweep_samples_per_packet = packet_size-6;

		header->samples_per_packet = packet_size - VRT_HEADER_SIZE - VRT_TRAILER_SIZE;

	
		//Check TSI field for 01 & get sec time stamp at the 3rd word
		if (!((vrt_header_buffer[1] & 0xC0) >> 6)) 
		{
			doutf(DHIGH, "ERROR: Second timestamp is not of UTC type.\n");
			free(vrt_header_buffer);
			return WSA_ERR_INVTIMESTAMP;
		}

		packet_size = (((uint16_t) vrt_header_buffer[2]) << 8) + (uint16_t) vrt_header_buffer[3];
		vrt_packet_bytes = 4 * (packet_size-2);
		vrt_packet_buffer = (uint8_t*) malloc(vrt_packet_bytes * sizeof(uint8_t));
	
		if (vrt_packet_buffer == NULL)
		{
			return WSA_ERR_MALLOCFAILED;
		}
			
		socket_receive_result = wsa_sock_recv_data(device->sock.data, vrt_packet_buffer, vrt_packet_bytes, TIMEOUT, &bytes_received);
		//fwrite (vrt_packet_buffer , 1 , vrt_packet_bytes, pFile );
		//fclose (pFile);
		doutf(DMED, "In wsa_read_iq_packet_raw: wsa_sock_recv_data returned %hd\n", socket_receive_result);

		if (socket_receive_result < 0)
		{
			doutf(DHIGH, "Error in wsa_read_iq_packet_raw:  %s\n", wsa_get_error_msg(socket_receive_result));
			free(vrt_packet_buffer);
			free(vrt_header_buffer);
			return socket_receive_result;
		}

		header->time_stamp.sec = (((uint32_t) vrt_packet_buffer[0]) << 24) +
							(((uint32_t) vrt_packet_buffer[1]) << 16) +
							(((uint32_t) vrt_packet_buffer[2]) << 8) + 
							(uint32_t) vrt_packet_buffer[3];
		doutf(DLOW, "second: 0x%08X %u\n",
			header->time_stamp.sec, 
			header->time_stamp.sec);

		//Check the TSF field, if present (= 0x10), 
		// get the picoseconds time stamp at the 4th & 5th words
		if ((vrt_header_buffer[1] & 0x30) >> 5) 
		{
			header->time_stamp.psec = (((uint64_t) vrt_packet_buffer[4]) << 56) +
					(((uint64_t) vrt_packet_buffer[5]) << 48) +
					(((uint64_t) vrt_packet_buffer[6]) << 40) +
					(((uint64_t) vrt_packet_buffer[7]) << 32) +
					(((uint64_t) vrt_packet_buffer[8]) << 24) +
					(((uint64_t) vrt_packet_buffer[9]) << 16) + 
					(((uint64_t) vrt_packet_buffer[10]) << 8) + 
					(uint64_t) vrt_packet_buffer[11];
		}
		else 
		{
			header->time_stamp.psec = 0ULL;
		}

		doutf(DLOW, "psec: 0x%016llX %llu\n", 
			header->time_stamp.psec, 
			header->time_stamp.psec);

		// *****
		// TODO: Handle the trailer word here once it is available
		// *****s
		//if (vrt_packet_buffer[0] & 0x04)
		// Placeholder values for now:
		trailer->valid_data_indicator = 0;
		trailer->ref_lock_indicator = 0;
		trailer->over_range_indicator = 0;
		trailer->sample_loss_indicator = 0;

		// *****
		// Copy the IQ data payload to the provided buffer
		// *****
				
		if(*samples_per_packet == 65530) {
		
			sweep_data_buffer = (uint8_t*) malloc((sweep_samples_per_packet) * BYTES_PER_VRT_WORD * sizeof(uint8_t));
		if (sweep_data_buffer == NULL)
		{
			return WSA_ERR_MALLOCFAILED;
		}
			memcpy(sweep_data_buffer, vrt_packet_buffer + ((VRT_HEADER_SIZE-2) * BYTES_PER_VRT_WORD), sweep_samples_per_packet * BYTES_PER_VRT_WORD);
			result = copy_sweep_data(data_buffer, sweep_data_buffer, sweep_samples_per_packet);
			*samples_per_packet = sweep_samples_per_packet;
			free(sweep_data_buffer);
			free(vrt_packet_buffer);
			free(vrt_header_buffer);
			return 0;
		}

		memcpy(data_buffer, vrt_packet_buffer + ((VRT_HEADER_SIZE-2) * BYTES_PER_VRT_WORD), *samples_per_packet * BYTES_PER_VRT_WORD);

	}
		free(sweep_data_buffer);
		free(vrt_packet_buffer);
		free(vrt_header_buffer);
	return 0;	
}






int16_t copy_sweep_data(uint8_t* data_buf, uint8_t* sweep_data_buf, int16_t size) {

	int i;
	for(i = 0; i < size; i++) {
		data_buf[i] = sweep_data_buf[i];
	}
	return 0;
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
int32_t wsa_decode_frame(uint8_t* data_buf, int16_t *i_buf, int16_t *q_buf, 
						 uint32_t sample_size)
{
	//int32_t result = 0;
	uint32_t i;
	int32_t j = 0;

	// *****
	// Split up the IQ data bytes
	// *****
	for (i = 0; i < sample_size * 4; i += 4) {
		// Gets the payload, each word = I2I1Q2Q1 bytes

		// TEMPORARY WORKAROUND
		// WSA returns a signed 14-bit integer padded with 2 most significant 0-bits
		// to encode the number in 16 bits.
		// Need to sign-extend by copying the value of bit 13 to bits 14 and 15
		// so that the number can be treated as a signed 16-bit integer
		// This workaround can be removed after the WSA firmware is updated
		// to perform the sign-extension in the embedded firmware

		if (data_buf[i] & 0x20)
		{
			i_buf[j] = (((int16_t) (data_buf[i] |= 0xC0)) << 8) + ((int16_t) data_buf[i + 1]);
		}
		else
		{
			// Need to keep this line after temporary workaround is removed
			i_buf[j] = (((int16_t) data_buf[i]) << 8) + ((int16_t) data_buf[i + 1]);
		}

		if (data_buf[i + 2] & 0x20)
		{
			q_buf[j] = (((int16_t) (data_buf[i + 2] |= 0xC0)) << 8) + ((int16_t) data_buf[i + 3]);
		}
		else
		{
			// Need to keep this line after temporary workaround is removed
			q_buf[j] = (((int16_t) data_buf[i + 2]) << 8) + ((int16_t) data_buf[i + 3]);
		}

		// END TEMPORARY WORKAROUND
		
		j++;
	}

	return (i / 4); //sample_size
}


/**
 * Decodes the raw reciever context packet and store it in the reciever 
 * structure 
 *
 * @param temp_buffer - pointer that points to the header of the reciever packet
 *note: the first two words are not included, the first word that temp points to is the 
 * timestamp. please review the program's guide for further information on how context packets are stored
 * @param reciever - a pointer structure to store the reciever data 
 * @return 0 if the data was stored succesfully, return a negative 16 bit integer otherwise 
 */
int16_t extract_reciever_packet_data(uint8_t* temp_buffer, 	struct wsa_reciever_packet* const reciever)
{


	int8_t gain_if_byte1 = 0;
	int8_t gain_if_byte2 = 0;
	int8_t gain_rf_byte1 = 0;
	int8_t gain_rf_byte2 = 0;
	int16_t gain_holder = 0;
	double gain_if = 0;
	double gain_rf = 0;
	int16_t temperature_holder = 0;
	int32_t temperature = 0;
	int32_t reference_point = 0;
	int64_t freq_word1 = 0;
	int64_t freq_word2 = 0;
	int64_t freq_holder1 = 0;
	long double freq_holder = 0;
	int64_t freq_dec = 0;
	int8_t data_pos = 16;
	double dec_holder = 0;
	double integer_holder = 0;

	int32_t context_fields = 0;
	
	//store the indicator field, which contains the content of the packet
	reciever->indicator_field = ((((int32_t) temp_buffer[12]) << 24) +
								(((int32_t) temp_buffer[13]) << 16) +
								(((int32_t) temp_buffer[14]) << 8) + 
								(int32_t) temp_buffer[15]);
	
	
	//determine if reference point data is present
	if (( temp_buffer[12] & 0xf0) == 0xc0) {
		
		reference_point = ((((int32_t) temp_buffer[data_pos]) << 24) +
								(((int32_t) temp_buffer[data_pos + 1]) << 16) +
								(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
								(int32_t) temp_buffer[data_pos + 3]);
		data_pos = data_pos + 4;
		
				if (reference_point == 16777217) {

		reciever->reference_point = 1;

		} else if (reference_point == 16777218) {
		
			reciever->reference_point = 2;
		}
	}
	
	
	//determine if frequency data is present
	if ((temp_buffer[12] & 0x0f) == 0x08) {
		//printf("%x %x %x %x \n",temp_buffer[data_pos], temp_buffer[data_pos + 1], temp_buffer[data_pos + 2] ,temp_buffer[data_pos + 3]);
		//printf("%x %x %x %x \n",temp_buffer[data_pos + 4], temp_buffer[data_pos + 5], temp_buffer[data_pos + 6] ,temp_buffer[data_pos + 7]);
		freq_word1 = ((((int64_t) temp_buffer[data_pos]) << 24) +
								(((int64_t) temp_buffer[data_pos + 1]) << 16) +
								(((int64_t) temp_buffer[data_pos + 2]) << 8) + 
								(int64_t) temp_buffer[data_pos + 3]);

		freq_word2 =  ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
								(((int64_t) temp_buffer[data_pos + 5]) << 16) +
								(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
								(int64_t) temp_buffer[data_pos + 7]);
		

		freq_holder1 = 4096 * freq_word1 + (freq_word2 & 0xfff00000)/1048576;
		freq_holder = (double) freq_holder1;
		integer_holder = freq_holder;
		freq_dec = (freq_word2 & 0x000fffff);
		dec_holder = (double) freq_dec;
		freq_holder = integer_holder + dec_holder/1000000;
		data_pos = data_pos + 8;
		printf("frequency is: %f \n", freq_holder/MHZ);
		reciever->frequency = freq_holder/MHZ;
		
		
		

		
	}
	
	//determine if gain data is present
	if ((temp_buffer[13] & 0xf0) == 0x80) {
	
		
		gain_if_byte1 = (int8_t) temp_buffer[data_pos]; 
		gain_if_byte2 = (int8_t) temp_buffer[data_pos + 1]; 
		gain_rf_byte1 = (int8_t) temp_buffer[data_pos + 2];
		gain_rf_byte2 = (int8_t) temp_buffer[data_pos + 3];

		
		gain_holder = (2 * gain_rf_byte1) + (gain_rf_byte2 & 0x8)/128;
		integer_holder = gain_holder;
		gain_holder = (gain_rf_byte2 & 0x7);
		dec_holder = gain_holder;
		gain_rf = integer_holder + dec_holder/1000;
		
		gain_holder = (2 * gain_if_byte1) + (gain_if_byte2 & 0x8)/128;
		integer_holder = gain_holder;
		gain_holder = (gain_if_byte2 & 0x7);
		dec_holder = gain_holder;
		gain_if = integer_holder + dec_holder/1000;

		reciever->gain_if = gain_if;
		reciever->gain_rf = gain_rf; 
		data_pos = data_pos + 4;
		
	}

	//determine of temperature data is present
	if ((temp_buffer[13] & 0x0f) == 0x04) {
				temperature = ((((int32_t) temp_buffer[data_pos]) << 24) +
								(((int32_t) temp_buffer[data_pos + 1]) << 16) +
								(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
								(int32_t) temp_buffer[data_pos + 3]);
				
				temperature_holder = (temperature & 0x0000ffc0);
				integer_holder = temperature_holder;
				temperature_holder = (temperature & 0x0000003f);
				dec_holder = temperature_holder;
				reciever->temperature = temperature;

	}


	return 0;

}
		
		
/**
 * Decodes the raw digitizer context packet and store it in the digitizer 
 * structure 
 *
 * @param temp_buffer - pointer that points to the header of the digitizer packet
 *note: the first two words are not included, the first word that temp points to is the 
 * timestamp. please review the program's guide for further information on how context packets are stored
 * @param digitizer - a pointer structure to store the digitizer data 
 * @return 0 if the data was stored succesfully, return a negative 16 bit integer otherwise 
 */
int16_t extract_digitizer_packet_data(uint8_t* temp_buffer, struct wsa_digitizer_packet* const digitizer){

	int64_t bandwidth = 0;
	int8_t data_pos = 16;
	int32_t context_fields = 0;
	int64_t rf_freq_offset = 0;
	int32_t reference_level = 0;
	int32_t reference_level_holder = 0;
	int64_t band_word1 = 0;
	int64_t band_word2 = 0;
	int64_t rf_freq_word1 = 0;
	int64_t rf_freq_word2 = 0;
	int64_t rf_freq_dec = 0;
	int64_t rf_freq_int = 0;
	int64_t band_holder = 0;
	int64_t band_dec = 0;
	double dec_holder = 0;
	double integer_holder = 0;
	double reference_level1 = 0;
	long double holder = 0;
	long double rf_freq_holder = 0;
	////store the indicator field, which contains the content of the packet
	digitizer->indicator_field = ((((int32_t) temp_buffer[12]) << 24) +
								(((int32_t) temp_buffer[13]) << 16) +
								(((int32_t) temp_buffer[14]) << 8) + 
								(int32_t) temp_buffer[15]);
	
	
	//determine if bandwidth data is present	
	if ((temp_buffer[12] & 0xf0) == 0xA0) {
		
		band_word1 = ((((int64_t) temp_buffer[data_pos]) << 24) +
								(((int64_t) temp_buffer[data_pos + 1]) << 16) +
								(((int64_t) temp_buffer[data_pos + 2]) << 8) + 
								(int64_t) temp_buffer[data_pos + 3]);

		band_word2 =  ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
								(((int64_t) temp_buffer[data_pos + 5]) << 16) +
								(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
								(int64_t) temp_buffer[data_pos + 7]);
		


		band_dec = (band_word2 & 0x000fffff);
		dec_holder = (double) band_dec;
		
		band_holder = 4096 * band_word1 + (band_word2 & 0xfff00000)/1048576;
		integer_holder = (double) band_holder;
		holder = (integer_holder*1000000 + band_dec)/1000000000000;
		digitizer->bandwidth = holder;
		

		data_pos = data_pos + 8;
	}

	//determine if rf frequency offset data is present
	if ( (temp_buffer[12] & 0xff) == 0x84) {
			
		rf_freq_word1 = ((((int64_t) temp_buffer[data_pos]) << 24) +
								(((int64_t) temp_buffer[data_pos + 1]) << 16) +
								(((int64_t) temp_buffer[data_pos + 2]) << 8) + 
								(int64_t) temp_buffer[data_pos + 3]);

		rf_freq_word2 =  ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
								(((int64_t) temp_buffer[data_pos + 5]) << 16) +
								(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
								(int64_t) temp_buffer[data_pos + 7]);

		rf_freq_holder = 4096 * rf_freq_word1 + (rf_freq_word2 & 0xfff00000)/1048576;
		integer_holder = (double) rf_freq_holder;
		rf_freq_holder = (rf_freq_word2 & 0x000fffff);
		dec_holder = (double) rf_freq_holder;
		rf_freq_holder = (integer_holder/1000000) + (dec_holder/100000000000);

		digitizer->rf_frequency_offset = rf_freq_holder;
		data_pos = data_pos + 8;

	}
	//determine if the reference level is present
	if ((temp_buffer[12] & 0x0f) == 0x01) {
					
		reference_level = ((((int32_t) temp_buffer[data_pos]) << 24) +
						(((int32_t) temp_buffer[data_pos + 1]) << 16) +
						(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
						(int32_t) temp_buffer[data_pos + 3]);	

						reference_level_holder = (reference_level & 0x0000ffc0);
				integer_holder = reference_level_holder;
				reference_level_holder = (reference_level & 0x0000003f);
				dec_holder = reference_level_holder;
				reference_level1 = dec_holder/1000000 + integer_holder/1000; 
				digitizer->reference_level = reference_level1;
	}

	return 0;




}


