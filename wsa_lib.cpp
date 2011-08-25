
#include <string.h>
#include "wsa_commons.h"
#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_lib.h"

//TODO create a log file method
//TODO add proper error method


/**
 * Initialized the the wsa_device structure
 *
 * @param dev - a wsa device structure.
 *
 * @return None
 */
int16_t wsa_dev_init(struct wsa_device *dev)
{
	dev->descr.inst_bw = 0;
	dev->descr.max_pkt_size = 0;
	dev->descr.max_tune_freq = 0;
	dev->descr.min_tune_freq = 0;
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
		dev->descr.max_pkt_size = WSA4000_MAX_PKT_SIZE;
		dev->descr.inst_bw = WSA4000_INST_BW;
		
		if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			dev->descr.max_tune_freq = (uint64_t) WSA_RFE0560_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
		}
	}

	//dev->run_mode = (wsa_run_mode) 0;
	//dev->trig_list = (wsa_trig *) malloc(sizeof(wsa_trig));

	return 0;
}


/**
 * Connect to a WSA through the specified interface method \b intf_method,
 * and communicate control commands in the format of the given command 
 * syntax.
 *
 * @param dev - the WSA device structure to be connected/establised.
 * @param cmd_syntax - The standard for control commands communication to the
 * WSA. \n Currently supported standard command syntax type is: SCPI.
 * @param intf_method - The interface method to the WSA. \n
 * Possible methods: \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::HISLIP" \n
 * - With USB, use: "USB" (check if supported with the WSA version used)
 * 
 * @return 0 on success, or a negative number on error.
 * TODO: define ERROR values with associated messages....
 */
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method)
{
	struct wsa_device wsa_dev;	// the wsa device structure
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
				printf("ERROR: Invalid TCPIP string \"%s\".\n", intf_method);
				doutf(1, "Error WSA_ERR_INVIPHOSTADDRESS: %s.\n", 
					wsa_get_err_msg(WSA_ERR_INVIPHOSTADDRESS));
				return WSA_ERR_INVIPHOSTADDRESS;
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
			printf(" ERROR: Interface method is not recognized/valid!\n");
			return -1;
		}
	}

	// When the cmd_syntax is not supported/recognized
	else {
		doutf(1, "Error WSA_ERR_INVINTFMETHOD: %s.\n", 
			wsa_get_err_msg(WSA_ERR_INVINTFMETHOD));
		return WSA_ERR_INVINTFMETHOD;
	}
	

	//*****
	// Do the connection
	//*****
	if (is_tcpip) {
		result = wsa_start_client(wsa_addr, &wsa_dev.sock.cmd, 
				&wsa_dev.sock.data);

		strcpy(wsa_dev.descr.intf_type, "TCPIP");

		if (result < 0) {
			printf("ERROR: Failed to start client sockets, closing down the "
				"connection!\n");
			doutf(1, "Error WSA_ERR_ETHERNETCONNECTFAILED: %s.\n", 
					wsa_get_err_msg(WSA_ERR_ETHERNETCONNECTFAILED));
			return WSA_ERR_ETHERNETCONNECTFAILED;
		}
	}
	
	// TODO Add other methods here
	
	*dev = wsa_dev;

	// Initialize wsa_device structure with the proper values
	if (wsa_dev_init(dev) < 0) {
		doutf(1, "Error WSA_ERR_USBINITFAILED: "
			"%s.\n", wsa_get_err_msg(WSA_ERR_INITFAILED));
		return WSA_ERR_INITFAILED;
	}

	return result;
}


/**
 * Close the device connection if one is started, stop any existing data 
 * capture, and perform any necessary clean ups.
 *
 * @param dev - The WSA device structure to be closed.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_disconnect(struct wsa_device *dev)
{
	int16_t result = 0;			// result returned from a function

//TODO Remove this line
wsa_send_command(dev, "STOPALL");
	
	//TODO close based on connection type
	// right now do only client
	result = wsa_close_client(dev->sock.cmd, dev->sock.data);

	// TODO test if you can still send after this.

	return result;
}


/**
 * List (print out) the IPs of connected WSAs to the network? or the PC???
 * For now, will list the IPs for any of the connected devices to a PC?
 *
 * @param wsa_list - Store (WSA???) IP addresses connected to a network???.
 *
 * @return Number of connected WSAs (or IPs for now) on success, or a 
 * negative number on error.
 */
// TODO: This section is to be replaced w/ list connected WSAs
int16_t wsa_list_devs(char **wsa_list) 
{
	int16_t result = 0;			// result returned from a function

	result = wsa_list_ips(wsa_list);

	return result;
}

/**
 * Open a file or print the help commands information associated with the 
 * WSA used.
 *
 * @param dev - The WSA device structure from which the help information 
 * will be provided.
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_help(struct wsa_device dev)
{
	// Open the generic SCPI for now
	if(_popen("ThinkRF SCPI DS 101202.pdf", "r") == NULL) {
		printf("ERROR: Failed to opent the SCPI file.\n");
		return -1;
	}
	return 0;
}


/**
 * Send the control command string to the WSA device specified by \b dev. 
 * The commands format must be written according to the specified 
 * standard syntax in wsa_connect().
 *
 * @param dev - The WSA device structure from which the command is sent to
 * @param command - The control command string written in the format specified 
 * by the standard syntax in wsa_connect()
 *
 * @return Number of bytes sent on success, or a negative number on error.
 */
int16_t wsa_send_command(struct wsa_device *dev, char *command)
{
	int16_t bytes_txed = 0;
	uint8_t resend_cnt = 0;
	uint16_t len = strlen(command);

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

	return bytes_txed;
}


/**
 * Send query command to the WSA device specified by \b dev. The
 * commands format must be written according to the specified command syntax 
 * in wsa_connect().
 *
 * @param dev - The WSA device structure from which the query is sent to
 * @param command - The query command string written in the format specified 
 * by the command syntax in wsa_connect()
 *
 * @return The result stored in a wsa_resp struct format.
 */
struct wsa_resp wsa_send_query(struct wsa_device *dev, char *command)
{
	struct wsa_resp resp;
	int16_t bytes_rxed = 0;
	char *rx_buf;

	// Initialized the receive buffer
	rx_buf = (char *) malloc(MAX_STR_LEN * sizeof(char));

	// Send the query command out
	bytes_rxed = wsa_send_command(dev, command);

	// Receive query result from the WSA server
	bytes_rxed = wsa_sock_recv(dev->sock.cmd, rx_buf, TIMEOUT);

//TODO Remove these
	printf("\nRxed %d bytes: ", bytes_rxed);
	//for (int i = 0; i < bytes_rxed; i++)
		printf("%s ", rx_buf);
	printf("\n");
// til here

	// TODO define what result should be
	resp.result = rx_buf;
	resp.status = bytes_rxed;

	free(rx_buf);

	return resp;
}


//TODO: Determine if string should be returned instead.
/**
 * Querry the WSA for any error
 *
 * @param dev - The WSA device structure from which the error query is sent to
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_query_error(struct wsa_device *dev)
{
	printf("To be added later.\n");

	return 0;
}


// this one need better design base on SCPI?
/**
 *
 * @param dev - The WSA device structure from which the command is sent to.
 * @param header - A wsa_frame_header structure of information for the frame.
 * @param i_buf - The unscaled, signed integer I data buffer with size 
 * specified by the frame_size.
 * @param q_buf - The unscaled, signed integer Q data buffer with size 
 * specified by the frame_size.
 * @param frame_size - The number of samples (i.e. {I, Q} sample pairs) to be 
 * captured per frame.
 *
 * @return Number of samples read on success, or a negative number on error.
 */
int64_t wsa_get_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 int32_t *i_buf, int32_t *q_buf, uint64_t frame_size)
{
	printf("Slow down... To be added later.\n");

	return 0;
}
