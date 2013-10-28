#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_lib.h"


//*****
// LOCAL DEFINES
//*****

// *****
// Local functions:
// *****
//char *wsa_query_error(struct wsa_device *dev);
int16_t wsa_query_error(struct wsa_device *dev, char *output);
int16_t _wsa_dev_init(struct wsa_device *dev);
int16_t _wsa_open(struct wsa_device *dev);
int16_t _wsa_query_stb(struct wsa_device *dev, char *output);
int16_t _wsa_query_esr(struct wsa_device *dev, char *output);
void extract_receiver_packet_data(uint8_t *temp_buffer, struct wsa_receiver_packet * const receiver);
void extract_digitizer_packet_data(uint8_t *temp_buffer, struct wsa_digitizer_packet * const digitizer);
void extract_extension_packet_data(uint8_t *temp_buffer, struct wsa_extension_packet * const extension);

// Initialized the \b wsa_device descriptor structure
// Return 0 on success or a 16-bit negative number on error.
int16_t _wsa_dev_init(struct wsa_device *dev)
{
	struct wsa_resp query;
	char *strtok_result;
	int16_t i = 0;
	// Initialized with "null" constants
	dev->descr.inst_bw = 0;
	dev->descr.max_sample_size = 0;
	dev->descr.max_tune_freq = 0;
	dev->descr.min_tune_freq = 0;
	dev->descr.freq_resolution = 0;
	dev->descr.max_if_gain = -1000;	// some impossible #
	dev->descr.min_if_gain = -1000;	// some impossible #
	dev->descr.max_decimation = 0;
	dev->descr.min_decimation = 0;
	
	for (i = 0; i < NUM_RF_GAINS; i++)
		dev->descr.abs_max_amp[i] = -1000;	// some impossible #
	
	wsa_send_query(dev, "*IDN?\n", &query);
	
	strtok_result = strtok(query.output, ",");
	strtok_result = strtok(NULL, ",");
	
	// grab product model
	if(strstr(strtok_result, WSA4000) != NULL) 
		sprintf(dev->descr.prod_model, "%s", WSA4000);
	
	else if(strstr(strtok_result, WSA5000) != NULL) 
		sprintf(dev->descr.prod_model, "%s", WSA5000);
	
	// grab product mac address
	strtok_result = strtok(NULL, ",");
	strcpy(dev->descr.mac_addr, strtok_result); // temp for now
	
	// grab product firmware version
	strtok_result = strtok(NULL, ",");
	strcpy(dev->descr.fw_version, strtok_result);
	
	dev->descr.max_sample_size = WSA_MAX_CAPTURE_BLOCK;
	dev->descr.inst_bw = (uint64_t) WSA_IBW;
	dev->descr.max_decimation = WSA_MAX_DECIMATION;
	dev->descr.min_decimation = WSA_MIN_DECIMATION;
	// 3rd, set some values base on the model
	// TODO: read from regs/eeprom instead once available
	
	if (strcmp(dev->descr.prod_model, WSA4000) == 0) 
	{

		dev->descr.max_tune_freq = (uint64_t) (WSA_4000_MAX_FREQ * MHZ);
		dev->descr.min_tune_freq = WSA_4000_MIN_FREQ;
		dev->descr.freq_resolution = WSA_4000_FREQRES;
		dev->descr.max_if_gain = WSA_4000_MAX_IF_GAIN;
		dev->descr.min_if_gain = WSA_4000_MIN_IF_GAIN;
		dev->descr.abs_max_amp[WSA_GAIN_HIGH] = WSA_4000_ABS_AMP_HIGH;
		dev->descr.abs_max_amp[WSA_GAIN_MED] = WSA_4000_ABS_AMP_MED;
		dev->descr.abs_max_amp[WSA_GAIN_LOW] = WSA_4000_ABS_AMP_LOW;
		dev->descr.abs_max_amp[WSA_GAIN_VLOW] = WSA_4000_ABS_AMP_VLOW;
	}
	else if (strcmp(dev->descr.prod_model, WSA5000) == 0) 
	{
		dev->descr.min_tune_freq = WSA_5000_MIN_FREQ;
		dev->descr.max_tune_freq = (uint64_t) (WSA_5000_MAX_FREQ * MHZ);
		dev->descr.freq_resolution = WSA_5000_FREQRES;
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
		printf("WSA returned: %s\n", resp.output);
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
	if (strcmp(dev->descr.intf_type, "USB") == 0) 
	{	
		return WSA_ERR_USBNOTAVBL;
	}
	else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) 
	{
		while (1) 
		{
			// Making the assumption that we will not send more bytes
			// than can fit into int16_t
			// TODO: revisit this and move bytes_txed into the parameter list
			bytes_txed = (int16_t) wsa_sock_send(dev->sock.cmd, command, len);
			if (bytes_txed < 0)
			{
				return bytes_txed;
			}
			else if (bytes_txed < len) 
			{
				if (resend_cnt > 3)
					return WSA_ERR_CMDSENDFAILED;

				printf("Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			else
			{
				break;
			}
		}  
		// If it's not asking for data, query for any error to
		// make sure that the set is done w/out any error in the system
		if (strstr(command, "DATA?") == NULL) 
		{
			wsa_query_error(dev, query_msg);
			if ((strstr(query_msg, "no response") != 0) && (bytes_txed > 0))
				return WSA_ERR_QUERYNORESP;
			
			if (strcmp(query_msg, "") != 0) {
				if (strstr(query_msg, "-221") != NULL)
					return WSA_WARNING_TRIGGER_CONFLICT;
				
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
    char new_str[MAX_STR_LEN];
    int i;

    // set defaults
    strcpy(resp.output, "");
    resp.status = 0;

    if((cmd_fptr = fopen(file_name, "r")) == NULL)
    {
        result = WSA_ERR_FILEREADFAILED;
        printf("ERROR %d: %s '%s'.\n", result, wsa_get_error_msg(result),
            file_name);
        return result;
    }

    // Allocate memory
    for (i = 0; i < MAX_FILE_LINES; i++)
	{
        cmd_strs[i] = (char *) malloc(sizeof(char) * MAX_STR_LEN);
		if (cmd_strs[i] == NULL)
		{
			doutf(DHIGH, "In wsa_send_command_file: failed to allocate memory\n");
			return WSA_ERR_MALLOCFAILED;
		}
	}

    result = wsa_tokenize_file(cmd_fptr, cmd_strs);
   
    // don't need command file any more
    fclose(cmd_fptr);

    // process the command strings acquired
    if (result > 0)
    {
        // Send each command line to WSA
        lines = result;
        for (i = 0; i < lines; i++)
        {
            strcpy(new_str, cmd_strs[i]);
            strcat(new_str, "\n");

            // Send non-query cmds
            if (strstr(new_str, "?") == NULL)
            {
                result = wsa_send_command(dev, new_str);
                // If a bad command is detected, continue? Prefer not.
                if (result < 0)
                {
                    printf("Error at line %d: '%s'.\n", i + 1, cmd_strs[i]);
                    break;
                }
            }
           
            // Send query cmds
            else
            {
                result = wsa_send_query(dev, new_str, &resp);           
           
                // If a bad command is detected, continue? Prefer not.
                if (resp.status < 0)
                {
                    printf("WSA returned error %lld: \"%s\" (possibly: %s)\n",
                        resp.status, resp.output,
                        _wsa_get_err_msg(WSA_ERR_CMDINVALID));
                    printf("Line %d: '%s'.\n", i + 1, cmd_strs[i]);
                    break;
                }

                printf("\"%s\" \n   WSA response: %s\n\n", cmd_strs[i], resp.output);
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
			else if (bytes_got < len) 
			{
				if (resend_cnt > 3) 
				{
					temp_resp.status = WSA_ERR_CMDSENDFAILED;
					strcpy(temp_resp.output, 
						_wsa_get_err_msg(WSA_ERR_CMDSENDFAILED));
					return WSA_ERR_CMDSENDFAILED;
				}

				doutf(DMED, "Not all bytes sent. Resending the packet...\n");
				resend_cnt++;
			}
			// Read back the output
			else 
			{

				recv_result = -1;
				while (recv_result != 0 && loop_count < 5) 
				{
					recv_result = wsa_sock_recv(dev->sock.cmd, 
							(uint8_t *) temp_resp.output, 
							MAX_STR_LEN, TIMEOUT, 
							&bytes_received);

					loop_count++;
				}

				if (recv_result == 0 && bytes_received < MAX_STR_LEN)
					temp_resp.output[bytes_received] = '\0'; // add EOL to the string
				else
					temp_resp.output[MAX_STR_LEN - 1] = '\0'; // add EOL to the string

				break;
			}
		}

		// TODO define what result should be
		if (recv_result != 0) 
		{
			temp_resp.status = WSA_ERR_QUERYNORESP;
			return WSA_ERR_QUERYNORESP;
		}
		else 
		{
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
 *inside the receiver structure and the digitizer structure.
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
 *@param receiver - a pointer to \b wsa_receiver_packet strucuture to store
 *		the receiver Context data
 *@param digitizer - a pointer to \b wsa_digitizer_packet strucuture to store
 *		the digitizer Context data
 *@param extension - a pointer to \b wsa_extension_packet strucuture to store
 *		the custom Context data
 * @param data_buffer - A uint8_t pointer buffer to store the raw I and Q data
 *		in bytes. Its size is determined by the number of 
 *		16-bit \b samples_per_packet words multiplied by 4 
 *		(i.e. sizeof(\b data_buffer) = \b samples_per_packet * 4 bytes per sample).
 * @param samples_per_packet - A 16-bit unsigned integer sample size (i.e. number of
 *		{I, Q} sample pairs) per VRT packet to be captured.
 *
 * @return  0 on success or a negative value on error
 */
int16_t wsa_read_vrt_packet_raw(struct wsa_device * const device, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet * const digitizer,
		struct wsa_extension_packet * const extension,
		uint8_t * const data_buffer)
{	
	uint8_t *vrt_header_buffer;
	int32_t vrt_header_bytes;

	uint8_t *vrt_packet_buffer;
	int32_t vrt_packet_bytes;
	
	uint32_t stream_identifier_word = 0;
	
	int32_t bytes_received = 0;
	int16_t socket_receive_result = 0;
	
	uint16_t packet_size = 0;
	uint16_t iq_packet_size;
	
	uint8_t has_trailer = 0;
	uint32_t trailer_word = 0;

	// reset header
	header->pkt_count = 0;
	header->samples_per_packet = 0;
	header->time_stamp.sec = 0;
	header->time_stamp.psec = 0;

	// *****
	// Setup, allocate buffer memory and fetch the first 2 header words
	// *****
	
	// Set to get the first 2 words of the header to extract 
	// packet size and packet type
	vrt_header_bytes = 2 * BYTES_PER_VRT_WORD;
	
	// allocate space for the header buffer
	vrt_header_buffer = (uint8_t *) malloc(vrt_header_bytes * sizeof(uint8_t));
	if (vrt_header_buffer == NULL)
		return WSA_ERR_MALLOCFAILED;

	// retrieve the first two words of the packet to determine if the packet contains IQ data or context data
	socket_receive_result = wsa_sock_recv_data(device->sock.data, 
												vrt_header_buffer, 
												vrt_header_bytes, 
												TIMEOUT, 
												&bytes_received);	
	doutf(DMED, "In wsa_read_vrt_packet_raw: wsa_sock_recv_data returned %hd\n", socket_receive_result);
	if (socket_receive_result < 0)
	{
		doutf(DHIGH, "Error in wsa_read_vrt_packet_raw:  %s\n", wsa_get_error_msg(socket_receive_result));
		free(vrt_header_buffer);

		return socket_receive_result;
	}
	
	// *****
	// Decode the first 2 words from the header
	// *****

	has_trailer = (vrt_header_buffer[0] & 0x04) >> 2;
	
	// Get the packet type
	header->packet_type = vrt_header_buffer[0] >> 4;
	
	// Get the 4-bit VRT "Pkt Count"
	// This counter increments from 0 to 15 and repeats again from 0 in a never-ending loop.
	// It provides a simple verification that packets are arriving in the right order
	header->pkt_count = (uint8_t) vrt_header_buffer[1] & 0x0f;	
	doutf(DLOW, "Packet order indicator: 0x%02X\n", header->pkt_count);
	
	// Check TSI field for 0x01 & get sec time stamp at the 3rd word
	if (!((vrt_header_buffer[1] & 0xC0) >> 6)) 
	{
		doutf(DHIGH, "ERROR: Second timestamp is not of UTC type.\n");
		free(vrt_header_buffer);
		return WSA_ERR_INVTIMESTAMP;
	}
		
	// retrieve the VRT packet size
	packet_size = (((uint16_t) vrt_header_buffer[2]) << 8) + (uint16_t) vrt_header_buffer[3];
	header->samples_per_packet = packet_size - VRT_HEADER_SIZE - VRT_TRAILER_SIZE;
	
	// Store the Stream Identifier to determine if the packet is an IQ packet or a context packet
	stream_identifier_word = (((uint32_t) vrt_header_buffer[4]) << 24) 
			+ (((uint32_t) vrt_header_buffer[5]) << 16) 
			+ (((uint32_t) vrt_header_buffer[6]) << 8) 
			+ (uint32_t) vrt_header_buffer[7];
	if ((stream_identifier_word != RECEIVER_STREAM_ID) && 
		(stream_identifier_word != DIGITIZER_STREAM_ID) && 
		(stream_identifier_word != IF_DATA_STREAM_ID) &&
		(stream_identifier_word != EXTENSION_STREAM_ID) &&
		(stream_identifier_word != HDR_DATA_STREAM_ID))
	{
		free(vrt_header_buffer);
		return WSA_ERR_NOTIQFRAME;
	}
	header->stream_id = stream_identifier_word;

	// *****
	// set up and get the remaining words of each different type of packet accordingly
	// *****
	
	// allocate memory for the vrt packet without the first two words
	vrt_packet_bytes = BYTES_PER_VRT_WORD * (packet_size - 2);
	vrt_packet_buffer = (uint8_t *) malloc(vrt_packet_bytes * sizeof(uint8_t));
	if (vrt_packet_buffer == NULL)
	{
		free(vrt_header_buffer);
		return WSA_ERR_MALLOCFAILED;
	}

	socket_receive_result = wsa_sock_recv_data(device->sock.data, 
		vrt_packet_buffer, vrt_packet_bytes, TIMEOUT, &bytes_received);
	doutf(DMED, "In wsa_read_vrt_packet_raw: wsa_sock_recv_data returned %hd\n",
		socket_receive_result);
	if (socket_receive_result < 0)
	{
		doutf(DHIGH, "Error in wsa_read_vrt_packet_raw:  %s\n", 
			wsa_get_error_msg(socket_receive_result));
		free(vrt_packet_buffer);
		free(vrt_header_buffer);

		return socket_receive_result;
	}

	// Get the second timestamp
	header->time_stamp.sec = (((uint32_t) vrt_packet_buffer[0]) << 24) +
						(((uint32_t) vrt_packet_buffer[1]) << 16) +
						(((uint32_t) vrt_packet_buffer[2]) << 8) + 
						(uint32_t) vrt_packet_buffer[3];
	
	doutf(DLOW, "second: 0x%08X %lu\n",
		header->time_stamp.sec, 
		header->time_stamp.sec);

	// Check the TSF field, if present (= 0x10), 
	// then get the picoseconds time stamp at the 4th & 5th words
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
	
	if (stream_identifier_word == EXTENSION_STREAM_ID)
	{
		// extract and store the extension context data
		extract_extension_packet_data(vrt_packet_buffer, extension);
		
		extension->pkt_count = header->pkt_count;
	}
	else if (stream_identifier_word == RECEIVER_STREAM_ID) 
	{
		// extract and store the receiver context data
		extract_receiver_packet_data(vrt_packet_buffer, receiver);
		
		receiver->pkt_count = header->pkt_count;
	} 
	else if (stream_identifier_word == DIGITIZER_STREAM_ID) 
	{
		// extract and store the digitizer context data
		extract_digitizer_packet_data(vrt_packet_buffer, digitizer);
		
		digitizer->pkt_count = header->pkt_count;
	}
	// if the packet is an IQ packet proceed with the method from previous release
	else if (stream_identifier_word == IF_DATA_STREAM_ID || stream_identifier_word == HDR_DATA_STREAM_ID)
	{
		iq_packet_size = header->samples_per_packet;
		
		// Copy only the IQ data payload to the provided buffer
		memcpy(data_buffer, 
			vrt_packet_buffer + ((VRT_HEADER_SIZE - 2) * BYTES_PER_VRT_WORD),
			iq_packet_size * BYTES_PER_VRT_WORD);

		// Handle the trailer word
		if (has_trailer)
		{
			memcpy(&trailer_word,
				vrt_packet_buffer + (((VRT_HEADER_SIZE - 2) + iq_packet_size) * BYTES_PER_VRT_WORD),
				1 * BYTES_PER_VRT_WORD);
			doutf(DLOW, "trailer_word: %08x\n", trailer_word);
			
			trailer->valid_data_indicator = 
				((trailer_word >> 30) & 0x1) ? ((trailer_word >> 18) & 0x1) : 0;
			trailer->ref_lock_indicator =
				((trailer_word >> 29) & 0x1) ? ((trailer_word >> 17) & 0x1) : 0;
			trailer->over_range_indicator =
				((trailer_word >> 25) & 0x1) ? ((trailer_word >> 13) & 0x1) : 0;
			trailer->sample_loss_indicator =
				((trailer_word >> 24) & 0x1) ? ((trailer_word >> 12) & 0x1) : 0;
			
			doutf(DLOW, "Valid_data: %d\n", trailer->valid_data_indicator);
			doutf(DLOW, "Ref-lock: %d\n", trailer->ref_lock_indicator);
			doutf(DLOW, "Over-range: %d\n", trailer->over_range_indicator);
			doutf(DLOW, "Sample loss: %d\n", trailer->sample_loss_indicator);
		}
	}
		
	free(vrt_packet_buffer);
	free(vrt_header_buffer);
	
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
 * @param i_buf - A 32-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the \b sample_size.
 * @param q_buf - A 32-bit signed integer pointer for the unscaled, 
 * Q data buffer with size specified by the \b sample_size.
 * @param sample_size - A 32-bit unsigned integer number of {I, Q} 
 * sample pairs to be decoded from \b data_buf. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of samples decoded, or a 16-bit negative 
 * number on error.
 */
int32_t wsa_decode_zif_frame(uint8_t *data_buf, int32_t *i_buf, int32_t *q_buf, 
						 int32_t sample_size)
{
	int32_t i;
	int32_t j = 0;

	// Split up the IQ data bytes
	for (i = 0; i < sample_size * 4; i += 4) 
	{
		i_buf[j] = (((int32_t) data_buf[i]) << 8) + ((int32_t) data_buf[i + 1]);
		q_buf[j] = (((int32_t) data_buf[i + 2]) << 8) + ((int32_t) data_buf[i + 3]);
		j++;
	}

	return (i / 4);
}

/**
 * Decodes the raw \b data_buf buffer I data bytes 
 * and returned the I buffer of data with the size determined by the 
 * \b sample_size parameter.  
 * 
 * Note: the \b data_buf size is assumed as \b sample_size * 4 bytes per sample
 *
 * @param data_buf - A char pointer buffer containing the raw I and Q data in
 * in bytes to be decoded into separate I and Q buffers. Its size is assumed to
 * be the number of 32-bit sample_size words multiply by 4 (i.e. 
 * sizeof(data_buf) = sample_size * 4 bytes per sample).
 * @param i_buf - A 32-bit signed integer pointer for the unscaled, 
 * I data buffer with size specified by the \b sample_size.
 * @param sample_size - A 32-bit nsigned integer containing the size 
 *  of \b data_buf. \n
 * The frame size is limited to a maximum number, \b max_sample_size, listed 
 * in the \b wsa_descriptor structure.
 *
 * @return The number of samples decoded, or a 16-bit negative 
 * number on error.
 */
int32_t wsa_decode_hdr_frame(uint8_t *data_buf, int32_t *i_buf, int32_t sample_size)
{
	int32_t i;
	int32_t j = 0;

	// Split up the I data bytes
	for (i = 0; i < sample_size * 4; i += 4) 
	{
		i_buf[j] = (((int32_t) data_buf[i])  << 24) + ((int32_t) data_buf[i + 1] << 16) + ((int32_t) data_buf[i + 2] << 8) + ((int32_t) data_buf[i + 3]);
		j++;
	}

	return i/4;
}

/**
 * Decodes the raw receiver context packet and store it in the receiver 
 * structure
 * @note: The first two words of the VRT context packet are not passed to here. 
 * The first word that \b temp_buffer points to is the timestamp. See the 
 * Programmer's Guide for further information on VRT packets.
 *
 * @param temp_buffer - A char pointer pointing to the timestamp, 'second' time
 *				field of the receiver packet
 * @param extension - A pointer to the receiver packet structure
 *
 * @return None
 */
void extract_receiver_packet_data(uint8_t *temp_buffer, struct wsa_receiver_packet * const receiver)
{	
	int32_t reference_point = 0;
	double gain_if = 0;
	double gain_rf = 0;
	int64_t freq_word1 = 0; 
	int64_t freq_word2 = 0;
	long double freq_int_part = 0;
	long double freq_dec_part = 0;
	
	int8_t data_pos = 16; // to increment data index
	
	// store the indicator field, which contains the content of the packet
	receiver->indicator_field = ((((int32_t) temp_buffer[12]) << 24) +
								(((int32_t) temp_buffer[13]) << 16) +
								(((int32_t) temp_buffer[14]) << 8) + 
								(int32_t) temp_buffer[15]);
	
	// determine if reference point data is present
	if ((receiver->indicator_field & REF_POINT_INDICATOR_MASK)== REF_POINT_INDICATOR_MASK) 
	{
		reference_point = ((((int32_t) temp_buffer[data_pos]) << 24) +
							(((int32_t) temp_buffer[data_pos + 1]) << 16) +
							(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
							(int32_t) temp_buffer[data_pos + 3]);
		receiver->reference_point = reference_point;
		data_pos = data_pos + 4;
	}
	
	// determine if frequency data is present
	if ((receiver->indicator_field  & FREQ_INDICATOR_MASK) == FREQ_INDICATOR_MASK)
	{
		freq_word1 = ((((int32_t) temp_buffer[data_pos]) << 24) +
					(((int32_t) temp_buffer[data_pos + 1]) << 16) +
					(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
					(int32_t) temp_buffer[data_pos + 3]);

		freq_word2 = ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
					(((int64_t) temp_buffer[data_pos + 5]) << 16) +
					(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
					(int64_t) temp_buffer[data_pos + 7]);

		freq_int_part = (long double) ((freq_word1 << 12) + (freq_word2 >> 20));
		freq_dec_part = (long double) (freq_word2 & 0x000fffff);
		receiver->freq = freq_int_part + (freq_dec_part / MHZ);
		data_pos = data_pos + 8;
	}
	
	// determine if gain data is present
	if ((receiver->indicator_field & GAIN_INDICATOR_MASK) == GAIN_INDICATOR_MASK) 
	{
		receiver->gain_if = ((int16_t) (temp_buffer[data_pos] << 8) + 
							temp_buffer[data_pos + 1]) / 128.0;

		receiver->gain_rf = ((int16_t) (temp_buffer[data_pos + 2] << 8) + 
							temp_buffer[data_pos + 3]) / 128.0;
		
		data_pos = data_pos + 4;		
	}

	// TODO: handle temperature
	/*if ((receiver->indicator_field & 0x0f) == 0x04) {
		receiver->temperature = ((int16_t) (temp_buffer[data_pos + 2] << 8) + 
					temp_buffer[data_pos + 3]) / 64.0;
	}*/
}
		
		
/**
 * Decodes the raw digitizer context packet and store it in the digitizer 
 * structure.
 * @note: The first two words of the VRT context packet are not passed to here. 
 * The first word that \b temp_buffer points to is the timestamp. See the 
 * Programmer's Guide for further information on VRT packets.
 *
 * @param temp_buffer - A char pointer pointing to the timestamp, 'second' time
 *			field of the digitizer packet
 * @param digitizer - A pointer the digitizer packet structure
 *
 * @return None
 */
void extract_digitizer_packet_data(uint8_t *temp_buffer, struct wsa_digitizer_packet * const digitizer) 
{

	int64_t bw_word1 = 0;
	int64_t bw_word2 = 0;
	long double bw_int_part = 0;
	long double bw_dec_part = 0;

	int64_t rf_freq_word1 = 0;
	int64_t rf_freq_word2 = 0;
	long double rf_freq_int_part = 0;
	long double rf_freq_dec_part = 0;

	int16_t ref_level_word = 0;
	double ref_level_int_part = 0;
	double ref_level_dec_part = 0;

	int32_t data_pos = 16;

	////store the indicator field, which contains the content of the packet
	digitizer->indicator_field = ((((int32_t) temp_buffer[12]) << 24) +
								(((int32_t) temp_buffer[13]) << 16) +
								(((int32_t) temp_buffer[14]) << 8) + 
								(int32_t) temp_buffer[15]);
	
	//determine if bandwidth data is present	
	if ((digitizer->indicator_field & BW_INDICATOR_MASK) ==  
		BW_INDICATOR_MASK) 
	{		
		bw_word1 = ((((int32_t) temp_buffer[data_pos]) << 24) +
						(((int32_t) temp_buffer[data_pos + 1]) << 16) +
						(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
						(int32_t) temp_buffer[data_pos + 3]);

		bw_word2 =  ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
						(((int64_t) temp_buffer[data_pos + 5]) << 16) +
						(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
						(int64_t) temp_buffer[data_pos + 7]);
		
		bw_int_part = (long double) ((bw_word1 << 12) + (bw_word2 >> 20));
		bw_dec_part = (long double) (bw_word2 & 0x000fffff);
		digitizer->bandwidth = bw_int_part + (bw_dec_part / MHZ);
		data_pos = data_pos + 8;
	}

	//determine if rf frequency offset data is present
	if ((digitizer->indicator_field & RF_FREQ_OFFSET_INDICATOR_MASK) == 
		RF_FREQ_OFFSET_INDICATOR_MASK) 
	{
		rf_freq_word1 = ((((int32_t) temp_buffer[data_pos]) << 24) +
						(((int32_t) temp_buffer[data_pos + 1]) << 16) +
						(((int32_t) temp_buffer[data_pos + 2]) << 8) + 
						(int32_t) temp_buffer[data_pos + 3]);

		rf_freq_word2 =  ((((int64_t) temp_buffer[data_pos + 4]) << 24) +
						(((int64_t) temp_buffer[data_pos + 5]) << 16) +
						(((int64_t) temp_buffer[data_pos + 6]) << 8) + 
						(int64_t) temp_buffer[data_pos + 7]);

		rf_freq_int_part = (long double) ((rf_freq_word1 << 12) + (rf_freq_word2 >> 20));
		rf_freq_dec_part = (long double) (rf_freq_word2 & 0x000fffff);
		digitizer->rf_freq_offset = rf_freq_int_part + (rf_freq_dec_part / MHZ);
		
		data_pos = data_pos + 8;
	}

	//determine if the reference level is present
	if ((digitizer->indicator_field & REF_LEVEL_INDICATOR_MASK) == 
		REF_LEVEL_INDICATOR_MASK) 
	{
					
		ref_level_word = ((((int16_t) (temp_buffer[data_pos + 2] << 8))) +
						(((int16_t) (temp_buffer[data_pos + 3]))));	

		digitizer->reference_level = (int16_t) (ref_level_word >> 7);
	}
}


/**
 * Decodes the raw extension packet and store it in the sweep
 * structure 
 * @note: The first two words of the VRT context packet are not passed to here. 
 * The first word that \b temp_buffer points to is the timestamp. See the 
 * Programmer's Guide for further information on VRT packets.
 *
 * @param temp_buffer - A char pointer pointing to the timestamp, 'second' time
 *				field of the extension packet
 * @param extension - A pointer to the extension packet structure
 * 
 * @return None
 */
void extract_extension_packet_data(uint8_t *temp_buffer,
		struct wsa_extension_packet * const extension)
{
	int32_t data_pos = 16;

	// store the indicator field, which contains the content of the packet
	extension->indicator_field = ((((int32_t) temp_buffer[12]) << 24) +
								(((int32_t) temp_buffer[13]) << 16) +
								(((int32_t) temp_buffer[14]) << 8) + 
								(int32_t) temp_buffer[15]);

	// determine if a sweep start id is in the packet
	if ((extension->indicator_field & SWEEP_START_ID_INDICATOR_MASK) == SWEEP_START_ID_INDICATOR_MASK) 
	{
		extension->sweep_start_id = 
				((((uint32_t) temp_buffer[data_pos]) << 24) +
				(((uint32_t) temp_buffer[data_pos + 1]) << 16) +
				(((uint32_t) temp_buffer[data_pos + 2]) << 8) + 
			    (uint32_t) temp_buffer[data_pos + 3]);
		data_pos = data_pos + 4;
	}
	
	if ((extension->indicator_field & STREAM_START_ID_INDICATOR_MASK) == STREAM_START_ID_INDICATOR_MASK) 
	{
		extension->stream_start_id = 
				((((uint32_t) temp_buffer[data_pos]) << 24) +
				(((uint32_t) temp_buffer[data_pos + 1]) << 16) +
				(((uint32_t) temp_buffer[data_pos + 2]) << 8) + 
			    (uint32_t) temp_buffer[data_pos + 3]);
	}
}
