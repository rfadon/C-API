
#include <stdio.h>
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
		dev->descr.max_sample_size = WSA4000_MAX_PKT_SIZE;
		dev->descr.inst_bw = (uint64_t) WSA4000_INST_BW;
		
		if (strcmp(dev->descr.rfe_name, WSA_RFE0560) == 0) {
			dev->descr.max_tune_freq = WSA_RFE0560_MAX_FREQ;
			dev->descr.min_tune_freq = WSA_RFE0560_MIN_FREQ;
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
			doutf(1, "Error WSA_ERR_ETHERNETCONNECTFAILED: %s.\n", 
					wsa_get_err_msg(WSA_ERR_ETHERNETCONNECTFAILED));
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


/**
 * List (print out) the IPs of connected WSAs to the network? or the PC???
 * For now, will list the IPs for any of the connected devices to a PC?
 *
 * @param wsa_list - A double char pointer to store (WSA???) IP addresses 
 * connected to a network???.
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


///**
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
//	if(_popen("ThinkRF SCPI DS 101202.pdf", "r") == NULL) {
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
int16_t wsa_send_command(struct wsa_device *dev, char *command)
{
	int16_t bytes_txed = 0;
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
	int64_t bytes_got = 0;

	// Send the query command out
	bytes_got = wsa_send_command(dev, command);

	// Receive query result from the WSA server
	if (bytes_got > 0) {
		// TODO: check WSA version/model # ?
		if (strcmp(dev->descr.intf_type, "USB") == 0) {	
			resp.status = WSA_ERR_USBNOTAVBL;
		}
		else if (strcmp(dev->descr.intf_type, "TCPIP") == 0) {
				bytes_got = wsa_sock_recv(dev->sock.cmd, resp.result, TIMEOUT);
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
int16_t wsa_query_error(struct wsa_device *dev)
{
	printf("To be added later.\n");

	return 0;
}


// this one need better design base on SCPI?
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
 * @return Number of samples read on success, or a negative number on error.
 */
int64_t wsa_get_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 int32_t *i_buf, int32_t *q_buf, uint64_t sample_size)
{
	printf("Slow down... To be added later.\n");

	return 0;
}


