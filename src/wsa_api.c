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
 *  - DC correction.  
 *  - IQ correction.  
 *  - Automatic finding of a WSA box(s) on a network.
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
#include "wsa_dsp.h"
#include "wsa_sweep_device.h"

#ifdef _WIN32
# define strtok_r strtok_s
#endif


#define MAX_RETRIES_READ_FRAME 5

// ////////////////////////////////////////////////////////////////////////////
// Local functions                                                           //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_verify_freq(struct wsa_device *dev, int64_t freq);

// Verify if the frequency is valid (within allowed range)
int16_t wsa_verify_freq(struct wsa_device *dev, int64_t freq)
{
	
	// verify the frequency value
	if ((freq < dev->descr.min_tune_freq) || (freq > dev->descr.max_tune_freq)) {
		return WSA_ERR_FREQOUTOFBOUND;
    }

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
	result = wsa_connect(dev, SCPI, intf_method, WSA_CONNECT_TIMEOUT);
	return result;
}

/**
 * Ping a WSA by attempting to establish a socket connection with a user specified timeout
 (
 * @param dev - A pointer to the WSA device structure to be opened.
 * @param intf_method - A char pointer to store the interface method to the 
 * WSA. \n Possible methods: \n
 * - With LAN, use: "TCPIP::<Ip address of the WSA>::37001" \n
 * - With USB, use: "USB" (check if supported with the WSA version used). \n
 *
 * @return 0 on success, or a negative number on error.
 * -
 */
int16_t wsa_ping(struct wsa_device *dev, char *intf_method)
{
	int16_t result = 0;
	
	// Start the WSA connection
	result = wsa_connect(dev, SCPI, intf_method, WSA_PING_TIMEOUT);
	wsa_disconnect(dev);
	
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
int16_t wsa_check_addr(char const *ip_addr) 
{
	int16_t result = 0;

	// Check with command port
	result = wsa_verify_addr(ip_addr, "37001");  //TODO make this dynamic
	if (result < 0) {
		return result;
    }

	// check with data port
	result = wsa_verify_addr(ip_addr, "37000");
	if (result < 0) {
		return result;
    }

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
int16_t wsa_check_addrandport(char const *ip_addr, char const *port) 
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
int16_t wsa_do_scpi_command_file(struct wsa_device *dev, char const *file_name)
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
int16_t wsa_query_scpi(struct wsa_device *dev, char const *command, char *response)
{
	struct wsa_resp query;		
    size_t len = strlen(command);
    char * tmpbuffer = malloc(len + 2);
	
    if(tmpbuffer) {
	    sprintf(tmpbuffer, "%s\n", command);
	
	    wsa_send_query(dev, tmpbuffer, &query);
	    strcpy(response, query.output);

        free(tmpbuffer);

	    return (int16_t) query.status;
    }

    return WSA_ERR_MALLOCFAILED;
}

/**
 * Send a scpi command to the wsa
 *
 * @param dev - A pointer to the WSA device structure.
 * @param command - A pointer to the scpi command
 *
 */
int16_t wsa_send_scpi(struct wsa_device *dev, char const *command)
{
	int16_t result;
    size_t len = strlen(command);
    char * tmpbuffer = malloc(len + 2);
	
    if(tmpbuffer) {
	    sprintf(tmpbuffer, "%s\n", command);
	
	    result = wsa_send_command(dev, tmpbuffer);

        free(tmpbuffer);

        return result;
    }

    return WSA_ERR_MALLOCFAILED;
}


// ////////////////////////////////////////////////////////////////////////////
// LAN CONFIGURATION SECTION                                                 //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Gets the lan configuration (either current or option set)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param config - A char pointer that indicates which lan configuration to return
 * For the WSA's current lan configuraiton, set to WSA_CURRENT_LAN_CONFIG.
 * For the current set option, set to ""
 * @param config - Char pointer containing the requested lan configuration
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_lan_config(struct wsa_device *dev, char const *config, char *lan_config)
{
	struct wsa_resp query;
	char command[MAX_STR_LEN];

	if ((strcmp(config, WSA_CURRENT_LAN_CONFIG)  != 0) &&
		(strcmp(config, WSA_OPTION_LAN_CONFIG) != 0))
		return WSA_ERR_INVRFEINPUTMODE;
	
	sprintf(command, "SYST:COMM:LAN:CONF? %s \n", config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev, command, &query);
	
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(lan_config, query.output);
	
	return 0;
}



/**
 * Sets the option lan configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * @param lan_config - A char pointer containing the lan configuration\n
 * valid lan configrations:  DHCP | STATIC
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lan_config(struct wsa_device *dev, char const *lan_config)
{
	int16_t result = 0;
	char command[MAX_STR_LEN];
	sprintf(command, "SYST:COMM:LAN:CONF %s \n", lan_config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	result = wsa_send_command(dev, command);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_lan_config: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}



/**
 * Gets the lan ip (either current or option set)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param config - A char pointer that indicates which lan configuration to return
 * For the WSA's current lan configuraiton, set to WSA_CURRENT_LAN_CONFIG.
 * For the current set option set to ""
 * @param ip - Char pointer containing the requested ip configuration
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_lan_ip(struct wsa_device *dev, char const *config, char *ip)
{
	struct wsa_resp query;		// store query results
	char command[MAX_STR_LEN];

	if ((strcmp(config, WSA_CURRENT_LAN_CONFIG)  != 0) &&
		(strcmp(config, WSA_OPTION_LAN_CONFIG) != 0))
		return WSA_ERR_INVRFEINPUTMODE;

	sprintf(command, "SYST:COMM:LAN:IP? %s \n", config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev, command, &query);
	
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(ip, query.output);
	
	return 0;
}


/**
 * Sets the user's ip configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * @param ip - A char pointer containing the ip
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lan_ip(struct wsa_device *dev, char const *ip)
{
	int16_t result = 0;
	char command[MAX_STR_LEN];

	sprintf(command, "SYST:COMM:LAN:IP %s \n", ip);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	result = wsa_send_command(dev, command);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_lan_ip: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Gets the lan netmask (either current or option set)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param config - A char pointer that indicates which lan configuration to return
 * For the WSA's current lan configuraiton, set to WSA_CURRENT_LAN_CONFIG.
 * For the current set option set to ""
 * @param netmask - Char pointer containing the requested netmask configuration
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_lan_netmask(struct wsa_device *dev, char const *config, char *netmask)
{
	struct wsa_resp query;
	
	char command[MAX_STR_LEN];

	if ((strcmp(config, WSA_CURRENT_LAN_CONFIG)  != 0) &&
		(strcmp(config, WSA_OPTION_LAN_CONFIG) != 0))
		return WSA_ERR_INVRFEINPUTMODE;

	sprintf(command, "SYST:COMM:LAN:NETMASK? %s \n", config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev, command, &query);
	
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(netmask, query.output);
	
	return 0;
}


/**
 * Sets the user's netmask configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * @param netmask - A char pointer containing the netmask
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lan_netmask(struct wsa_device *dev, char const *netmask)
{
	int16_t result = 0;
	char command[MAX_STR_LEN];

	sprintf(command, "SYST:COMM:LAN:NETMASK %s \n", netmask);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	result = wsa_send_command(dev, command);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_lan_netmask: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Gets the lan gateway (either current or option set)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param config - A char pointer that indicates which lan configuration to return
 * For the WSA's current lan configuraiton, set to WSA_CURRENT_LAN_CONFIG.
 * For the current set option set to ""
 * @param gateway - Char pointer containing the requested gateway configuration
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_lan_gateway(struct wsa_device *dev, char const *config, char *gateway)
{
	struct wsa_resp query;
	char command[MAX_STR_LEN];

	if ((strcmp(config, WSA_CURRENT_LAN_CONFIG)  != 0) &&
		(strcmp(config, WSA_OPTION_LAN_CONFIG) != 0))
		return WSA_ERR_INVRFEINPUTMODE;

	sprintf(command, "SYST:COMM:LAN:GATEWAY? %s \n", config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev, command, &query);
	
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(gateway, query.output);
	
	return 0;
}


/**
 * Sets the user's gateway configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * @param gateway - A char pointer containing the gateway\n
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lan_gateway(struct wsa_device *dev, char const *gateway)
{
	int16_t result = 0;
	char command[MAX_STR_LEN];

	sprintf(command, "SYST:COMM:LAN:GATEWAY %s \n", gateway);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	result = wsa_send_command(dev, command);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_lan_gateway: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Gets the lan dbs (either current or option set)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param config - A char pointer that indicates which lan configuration to return
 * For the WSA's current lan configuraiton, set to WSA_CURRENT_LAN_CONFIG.
 * For the current set option set to ""
 * @param dns - Char pointer containing the requested dns configuration
 * Note: dns may contain a comma seperated alternate dns value
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_lan_dns(struct wsa_device *dev, char const *config, char *dns)
{
	struct wsa_resp query;		// store query results
	char command[MAX_STR_LEN];
	
	if ((strcmp(config, WSA_CURRENT_LAN_CONFIG)  != 0) &&
		(strcmp(config, WSA_OPTION_LAN_CONFIG) != 0))
		return WSA_ERR_INVRFEINPUTMODE;
	
	sprintf(command, "SYST:COMM:LAN:DNS? %s \n", config);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev, command, &query);
	
	if (query.status <= 0)
		return (int16_t) query.status;

	return 0;
}


// TODO: ADD BETTER ERROR HANDLING
/**
 * Sets the user's dns configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * @param dns - A char pointer containing the dns ip \n
 * @param alternate_dns - A char pointer containing the alternate dns ip
 * Note: The alternate DNS is optional, if you don't want to set an alternate DNS \n
 * set alternate_dns to ""
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_lan_dns(struct wsa_device *dev, char const *dns, char const *alternate_dns)
{
	int16_t result = 0;
	char command[MAX_STR_LEN];
	sprintf(command, "SYST:COMM:LAN:DNS %s \n", dns);

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	result = wsa_send_command(dev, command);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_lan_dns: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


// TODO: ADD BETTER ERROR HANDLING
/**
 * Apply the user's current lan configuration
 *
 * @param dev - A pointer to the WSA device structure.
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_apply_lan_config(struct wsa_device *dev)
{
	int16_t result;

	result = wsa_send_command(dev, ":SYST:COMM:LAN:APPLY\n");
	if (result < 0) {
        doutf(DHIGH, "In wsa_apply_lan_config: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char status[40];

	// check if the wsa is already sweeping
	result = wsa_get_sweep_status(dev, status);
	if (result < 0) {
		return result;
    }

	if (strcmp(status, WSA_SWEEP_STATE_RUNNING) == 0) {
		return WSA_ERR_SWEEPALREADYRUNNING;
    }

	result = wsa_send_command(dev, "SYSTEM:FLUSH\n");
	if (result < 0) {
        doutf(DHIGH, "In wsa_flush_data: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
	return result;
}


/**
 * Read out the data remaining in the data socket(reads for 1 second)
 *
 * @param dev - A pointer to the WSA device structure.
 *
 */
int16_t wsa_clean_data_socket(struct wsa_device *dev)
{
	int32_t bytes_received = 0;
	uint8_t *packet;
    int32_t packet_size = WSA_MAX_CAPTURE_BLOCK;
	uint32_t timeout = 360;
    clock_t start_time;
    clock_t end_time;
	
	start_time = clock();
	end_time = 1000 + start_time;

	packet = (uint8_t *) malloc(packet_size * sizeof(uint8_t));
	if (packet == NULL)	{
		doutf(DHIGH, "In wsa_clean_data_socket: failed to allocate memory\n");
		return WSA_ERR_MALLOCFAILED;
	}
	// read the left over packets from the socket
	while(clock() <= end_time) {
		wsa_sock_recv_data(dev->sock.data, 
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

	result = wsa_send_command(dev, "SYSTEM:ABORT\n");
	if (result < 0) {
        doutf(DHIGH, "In wsa_system_abort_capture: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	
	if (strcmp(query.output, WSA_BLOCK_CAPTURE_MODE) == 0 || 
		strcmp(query.output, WSA_STREAM_CAPTURE_MODE) == 0 ||
		strcmp(query.output, WSA_SWEEP_CAPTURE_MODE) == 0)
		strcpy(mode, query.output);
	
	else
		return WSA_ERR_RESPUNKNOWN;

	return 0;
}


/**
 * Aborts the current data capturing process (sweep mode/stream mode) and 
 * places the WSA system into capture block mode
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
	if (result < 0) {
        doutf(DHIGH, "Error in wsa_abort_capture: %d - %s\n", result, wsa_get_error_msg(result));
    }

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
	if (result < 0) {
        doutf(DHIGH, "Error in wsa_capture_block: %d - %s\n", result, wsa_get_error_msg(result));
    }

	return result;
}

/**
 * Reads one VRT packet containing raw IF data. 
 * Each packet consists of a header, a data payload, and a trailer.
 * The number of samples expected in the packet is indicated by
 * the \b samples_per_packet parameter.
 *
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
 * @param i16_buffer - A 16-bit signed integer pointer for the unscaled, 
 *		I data buffer with size specified by samples_per_packet.
 * @param q16_buffer - A 16-bit signed integer pointer for the unscaled 
 *		Q data buffer with size specified by samples_per_packet.
  * @param i32_buffer - A 32-bit signed integer pointer for the unscaled 
 *		I data buffer with size specified by samples_per_packet.
 * @param samples_per_packet - A 16-bit unsigned integer sample size (i.e. number of
 *		{I, Q} sample pairs) per VRT packet to be captured.
 * @param timeout - An unsigned 32-bit value containing the timeout (in miliseconds)
 *
 * @return 0 on success or a negative value on error
 */
int16_t wsa_read_vrt_packet (struct wsa_device * const dev, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet * const digitizer,
		struct wsa_extension_packet * const sweep_info,
		int16_t * const i16_buffer, 
		int16_t * const q16_buffer,
		int32_t * const i32_buffer,
		int32_t samples_per_packet,
		uint32_t timeout)		
{
	uint8_t *data_buffer;
	int16_t result = 0;
	int16_t result2 = 0;
	int i = 0;
	// allocate the data buffer
	data_buffer = (uint8_t *) malloc(samples_per_packet * BYTES_PER_VRT_WORD * sizeof(uint8_t));
	if (data_buffer == NULL) {
		doutf(DHIGH, "In wsa_read_vrt_packet: failed to allocate memory\n");
		return WSA_ERR_MALLOCFAILED;
	}
			
	result = wsa_read_vrt_packet_raw(dev, header, trailer, receiver, digitizer, sweep_info, data_buffer, timeout);
	doutf(DLOW, "wsa_read_vrt_packet_raw returned %hd\n", result);
	if (result < 0)	{
		doutf(DHIGH, "Error in wsa_read_vrt_packet: %s\n", wsa_get_error_msg(result));
		if (result == WSA_ERR_NOTIQFRAME || result == WSA_ERR_QUERYNORESP) {
			wsa_system_abort_capture(dev);
			result2 = wsa_flush_data(dev); 
        }

		free(data_buffer);
		return result;
	} 

	// decode ZIF data packets
	if (header->stream_id == I16Q16_DATA_STREAM_ID) 
		result = (int16_t) wsa_decode_zif_frame(data_buffer, i16_buffer, q16_buffer, header->samples_per_packet);
	
	// decode HDR/SH data packets
	else if (header->stream_id == I32_DATA_STREAM_ID || header->stream_id == I16_DATA_STREAM_ID)
		result = (int16_t) wsa_decode_i_only_frame(header->stream_id, data_buffer, i16_buffer, i32_buffer,  header->samples_per_packet);
	
	free(data_buffer);

	return 0;
}


/**
 * Find the peak value within the specified range
 * Note you must set the sample size, and acquire read status before 
 * calling this function
 * @param dev - A pointer to the WSA device structure.
 * @fstart- An unsigned 64-bit integer containing the start frequency
 * @fstop - An unsigned 64-bit integer containing the stop frequency
 * @rbw - A 64-bit integer containing the RBW value of the captured data (in Hz)
 * @mode - A string containing the mode for the measurement
 * @attenuator - An integer to hold the 20dB attenuator's state (0 = on, 1 = off)
 * @peak_freq - An unsigned 64-bit integer to store the frequency of the peak(in Hz)
 * @peak_power - A flloating point pointer to stopre the power level of the peak (in dBm)

 *
 * @return 0 on success or a negative value on error
 */
int16_t peak_find(struct wsa_device *dev, 
					uint64_t fstart, 
					uint64_t fstop, 
					uint32_t rbw, 
					char *mode,
					int32_t attenuator,
					uint64_t *peak_freq,
					float *peak_power)
{
	

	struct wsa_power_spectrum_config *pscfg;
	struct wsa_sweep_device wsa_Sweep_Device;
	struct wsa_sweep_device *wsa_sweep_dev = &wsa_Sweep_Device;
	int result = 5;
	int16_t acq_status = 0;
	float *psbuf;
	uint32_t i = 0;
	uint64_t current_freq = fstart;

	// create the sweep device
	wsa_sweep_dev = wsa_sweep_device_new(dev);
	
	// set the attenuator
	wsa_sweep_device_set_attenuator(wsa_sweep_dev, attenuator);

	// allocate memory for our ffts to go in
	result = wsa_power_spectrum_alloc(wsa_sweep_dev, fstart, fstop, rbw, mode, &pscfg);
	if (result < 0)
	{
		wsa_power_spectrum_free(pscfg);
		wsa_sweep_device_free(wsa_sweep_dev);
		return (int16_t) result; 
	}

	// capture power spectrum
	result = wsa_capture_power_spectrum(wsa_sweep_dev, pscfg, &psbuf);
	*peak_power = psbuf[i];
	*peak_freq = fstart;
	for (i = 0; i < pscfg->buflen; i++){
		if (psbuf[i] > *peak_power){
			*peak_power = psbuf[i];
			*peak_freq = current_freq;
		}
		current_freq = current_freq + (uint64_t) rbw;
	}

	wsa_power_spectrum_free(pscfg);
	wsa_sweep_device_free(wsa_sweep_dev);
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

	if ((samples_per_packet < WSA_MIN_SPP) || 
		(samples_per_packet > WSA_MAX_SPP) || ((samples_per_packet % WSA_SPP_MULTIPLE) != 0))
		return WSA_ERR_INVSAMPLESIZE;
	
	sprintf(temp_str, "TRACE:SPPACKET %u\n", samples_per_packet);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_samples_per_packet: %d - %s.\n", result, wsa_get_error_msg(result));
    }
		
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
	int temp;
	wsa_send_query(dev, "TRACE:SPPACKET?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA_MIN_SPP) || (temp > WSA_MAX_SPP)) {
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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
	
	if (packets_per_block < WSA_MIN_PPB) {
		return WSA_ERR_INVNUMBER;
	} else if (packets_per_block > WSA_MAX_PPB) {
		return WSA_ERR_INVCAPTURESIZE;
    }

	sprintf(temp_str, "TRACE:BLOCK:PACKETS %u\n", packets_per_block);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_packets_per_block: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int temp;

	wsa_send_query(dev, "TRACE:BLOCK:PACKETS?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*packets_per_block = (int32_t) temp;

	return 0;
}


/**
 * Gets the decimation rate currently set in the WSA. If rate is 1, it means
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
	int temp;

	wsa_send_query(dev, ":SENSE:DEC?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// convert & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// make sure the returned value is valid
	if (((temp != 1) && (temp < dev->descr.min_decimation)) || 
		(temp > dev->descr.max_decimation)) 
	{
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
		return WSA_ERR_RESPUNKNOWN;
	}

	*rate = (int32_t) temp;

	return 0;
}


/**
 * Set the decimation rate. 
 * Rate supported: 1, 4 - 1023. Rate of 1 is equivalent to no decimation.
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
    if (result < 0) {
	    doutf(DHIGH, "In wsa_set_decimation: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}

// ////////////////////////////////////////////////////////////////////////////
// DSP Section                                                               //
// ////////////////////////////////////////////////////////////////////////////

/**
 * Retrieve the the size of the buffer required to store the spectral data
 *
 * @param samples_per_packet - The number of time domain samples
 * @param stream_id - The ID indentifying the type of data
 * @param buffer_size - The size of the buffer required to store the spectral data
 *
 * @return 0 on successful or a negative number on error.
 */
int16_t wsa_get_fft_size(int32_t const samples_per_packet, 
							uint32_t const stream_id,
							int32_t *buffer_size)
{
	if (stream_id == I16Q16_DATA_STREAM_ID)
		*buffer_size = samples_per_packet;
	else
		*buffer_size = samples_per_packet / 2;
	return 0;
}
/**
 * Retrieve the the size of the buffer required to store the spectral data
 *
 * @param samples_per_packet - The number of time domain samples
 * @param stream_id - The ID indentifying the type of data
 * @param reference_level - dBm value used to calibrate the signal
 * @param spectral_inversion - byte containing whether spectral inversion is active
 * @param i16_buffer - buffer containing 16-bit iData
 * @param q16_buffer - buffer containing 16-bit qData
 * @param i32_buffer - buffer containing 32-bit iData
 * @param fft_buffer - Buffer to store the FFT data
 *
* @return 0 on successful or a negative number   on error.
 */
int16_t wsa_compute_fft(int32_t const samples_per_packet,
				int32_t const fft_size,
				uint32_t const stream_id,
				int16_t const reference_level,
				uint8_t const spectral_inversion,
				int16_t * const i16_buffer,
				int16_t * const q16_buffer,
				int32_t * const i32_buffer,
				float * fft_buffer
				)
{
	kiss_fft_scalar idata[32768];
	kiss_fft_scalar qdata[32768];
	kiss_fft_cpx fftout[32768];
	kiss_fft_scalar tmpscalar;
	int16_t result = 0;
	int32_t i = 0;

	// window and normalize the data
	normalize_iq_data(samples_per_packet,
					stream_id,
					i16_buffer,
					q16_buffer,
					i32_buffer,
					idata,
					qdata);
	doutf(DHIGH, "In wsa_compute_fft: normalized data\n");

	// correct the DC offset
	//correct_dc_offset(samples_per_packet, idata, qdata);

	window_hanning_scalar_array(idata, samples_per_packet);

	doutf(DHIGH, "In wsa_compute_fft: applied hanning window\n");
	// fft this data
	rfft(idata, fftout, samples_per_packet);

	doutf(DHIGH, "In wsa_compute_fft: finished computing FFT\n");	
	/*
	* we used to be in superhet mode, but after a complex FFT, we have twice 
	* the spectrum at twice the RBW.
	* our fcenter is now moved from center to $passband_center so our start and stop 
	* indexes are calculated given that fact
	*/

	// check for inversion and calculate indexes of our good data
	if (spectral_inversion)
		reverse_cpx(fftout, fft_size);

	doutf(DHIGH, "In wsa_compute_fft: finished compensating for spectral inversion\n");
	// for the usable section, convert to power, apply reflevel and copy into buffer
	for (i = 0; i < fft_size; i++) {
		tmpscalar = cpx_to_power(fftout[i]) / samples_per_packet;
		tmpscalar = 2 * power_to_logpower(tmpscalar);
		fft_buffer[i] = tmpscalar + reference_level;
		}

	doutf(DHIGH, "In wsa_compute_fft: finished moving buffer\n");
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
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0)	{
		printf("Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < dev->descr.min_tune_freq) || (temp > dev->descr.max_tune_freq)) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
	if (result < 0) {
		return result;
    }

	sprintf(temp_str, "FREQ:CENT %lld Hz\n", cfreq);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_freq: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}


	if ((temp < -range) || (temp > range)) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
	if ((fshift < (-range)) || (fshift > range)) {
		return WSA_ERR_FREQOUTOFBOUND;
    }

	sprintf(temp_str, "FREQ:SHIFt %f Hz\n", fshift);
	result = wsa_send_command(dev, temp_str);
    if(result < 0) {
	    doutf(DHIGH, "In wsa_set_freq_shift: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * get spectral inversion status at  a specific frequency (assumes current rfe mode set)
 * Note: Check the spec_inv param in the trailer to determine  if
 * spectral inversion has been compensated
 * in the digitizer
 *
 * @param dev - A pointer to the WSA device structure.	
 * @param freq - The center frequency band to check (Hz)
 * @param inv - A pointer to store whether spectral inversion 
 * has occured
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_spec_inv(struct wsa_device *dev, int64_t freq, int16_t *inv)
{
	struct wsa_resp query;		// store query results
	double temp;
	char scpi_cmd[MAX_STR_LEN];

	sprintf(scpi_cmd, "SENSE:FREQ:INV? %lld Hz\n", freq);

	wsa_send_query(dev,scpi_cmd, &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*inv = (int16_t) temp;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// GAIN/ATTNEUATION SECTION                                                 //
//////////////////////////////////////////////////////////////////////////////


/**
 * Gets the attenuator's mode of operation
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer pointer to store the attenuator's mode
 * of operation
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_attenuation(struct wsa_device *dev, int32_t *mode)
{
	struct wsa_resp query;
	int temp;
	
	if (strcmp(dev->descr.prod_model,WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	wsa_send_query(dev, "INPUT:ATTENUATOR?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }
	
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if ((int32_t) temp != WSA_ATTEN_ENABLED  && (int32_t) temp != WSA_ATTEN_DISABLED)
		return WSA_ERR_INVATTEN;
	*mode = (int32_t) temp;
	
	return 0;
}


/**
 * Sets the attenuator's mode of operation (0 = Off/1 = On)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer pointer containing the attenuation's mode of operation
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_attenuation(struct wsa_device *dev, int32_t mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;
	
	if (mode != WSA_ATTEN_ENABLED  && mode != WSA_ATTEN_DISABLED)
		return WSA_ERR_INVATTEN;

	sprintf(temp_str, "INPUT:ATTENUATOR %d\n", mode);

	result = wsa_send_command(dev, temp_str);
    if(result < 0) {
	  doutf(DHIGH, "In wsa_set_attenuation: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
	return result;
}


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
	int temp;

	if (strcmp(dev->descr.prod_model,WSA5000) == 0) {
		return WSA_ERR_INV5000COMMAND;
    }

	wsa_send_query(dev, "INPUT:GAIN:IF?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (((int32_t) temp < dev->descr.min_if_gain) || 
		((int32_t) temp > dev->descr.max_if_gain))
	{
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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
	
	if (strcmp(dev->descr.prod_model,WSA5000) == 0) {
		return WSA_ERR_INV5000COMMAND;
    }

	if ((gain < dev->descr.min_if_gain) || (gain > dev->descr.max_if_gain)) {
		return WSA_ERR_INVIFGAIN;
    }

	sprintf(temp_str, "INPUT:GAIN:IF %d dB\n", gain);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_gain_if: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

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
int16_t wsa_set_gain_rf(struct wsa_device *dev, char const * gain)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

	if (strcmp(gain, WSA_GAIN_VLOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_LOW_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_MED_STRING) != 0 &&
		strcmp(gain, WSA_GAIN_HIGH_STRING) != 0)
		return WSA_ERR_INVRFGAIN;

	sprintf(temp_str, "INPUT:GAIN:RF %s\n", gain);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_gain_rf: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////


/**
 * Query the WSA5000's RFE mode of operation
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer to store the current RFE mode of operation.
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_rfe_input_mode(struct wsa_device *dev, char *mode)
{
	struct wsa_resp query;		// store query results

	if (strcmp(dev->descr.prod_model , WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	wsa_send_query(dev, "INPUT:MODE?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	strcpy(mode,query.output);

	if ((strcmp(mode, WSA_RFE_ZIF_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_DD_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_HDR_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_SH_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_SHN_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_IQIN_STRING) != 0)) {
		return WSA_ERR_INVRFEINPUTMODE;
    }

	return 0;
}


/**
 * Sets the RFE's input mode of the WSA5000
 * Valid RFE modes are: ZIF, HDR, SH
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer containing the RFE input mode\n
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_rfe_input_mode(struct wsa_device *dev, char const *mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model , WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	if ((strcmp(mode, WSA_RFE_ZIF_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_DD_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_HDR_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_SH_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_SHN_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_IQIN_STRING) != 0)) {
		return WSA_ERR_INVRFEINPUTMODE;
    }

	sprintf(temp_str, "INPUT:MODE %s\n", mode);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_rfe_input_mode: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Query the WSA5000's IQ output mode
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer to store the current IQ  output mode.
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_iq_output_mode(struct wsa_device *dev, char *mode)
{
	struct wsa_resp query;		// store query results

	if (strcmp(dev->descr.prod_model , WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	wsa_send_query(dev,":OUT:IQ:MODE?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	strcpy(mode,query.output);

	if (strcmp(mode, WSA_IQ_DIGITIZER_STRING) != 0 &&
		strcmp(mode, WSA_IQ_CONNECTOR_STRING) != 0)
		return WSA_ERR_INVRFEINPUTMODE;

	return 0;
}


/**
 * Sets the IQ output mode of the WSA5000
 * Valid output modes are: DIGITIZER, CONNECTOR
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer containing the IQ output mode\n
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_iq_output_mode(struct wsa_device *dev, char const *mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model , WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;

	if (strcmp(mode, WSA_IQ_DIGITIZER_STRING) != 0 &&
		strcmp(mode, WSA_IQ_CONNECTOR_STRING) != 0)
		return WSA_ERR_INVRFEINPUTMODE;

	sprintf(temp_str, ":OUT:IQ:MODE %s\n", mode);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_iq_output_mode: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


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
	int temp;

	if (strcmp(dev->descr.prod_model,WSA5000) == 0) {
		return WSA_ERR_INV5000COMMAND;
    }

	wsa_send_query(dev, "INPUT:ANTENNA?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < 1) || (temp > WSA_4000_MAX_ANT_PORT)) {
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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
	
	if (strcmp(dev->descr.prod_model,WSA5000) == 0) {
		return WSA_ERR_INV5000COMMAND;
    }

	if ((port_num < 1) || (port_num > WSA_4000_MAX_ANT_PORT)) { // TODO replace the max
		return WSA_ERR_INVANTENNAPORT;
    }

	sprintf(temp_str, "INPUT:ANTENNA %d\n", port_num);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_antenna: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int temp;

	wsa_send_query(dev, "INP:FILT:PRES?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if ((temp < 0) || (temp > 1)) {
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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

	if ((mode < 0) || (mode > 1)) {
		return WSA_ERR_INVFILTERMODE;
    }

	sprintf(temp_str, "INPUT:FILT:PRES %d\n", mode);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_bpf_mode: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
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
	if (result == WSA_ERR_FREQOUTOFBOUND) {
		return WSA_ERR_STARTOOB;
    } else if (result < 0) {
		return result;
    }

	result = wsa_verify_freq(dev, stop_freq);
	if (result == WSA_ERR_FREQOUTOFBOUND) {
		return WSA_ERR_STOPOOB;
	} else if (result < 0) {
		return result;
    }

	sprintf(temp_str, ":TRIG:LEVEL %lld,%lld,%d\n", start_freq, stop_freq, amplitude);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_trigger_level: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char * strtok_result;
    char * strtok_context = 0;

	wsa_send_query(dev, ":TRIG:LEVEL?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }
	
	// Convert the 1st number & make sure no error
	strtok_result = strtok_r(query.output, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < dev->descr.min_tune_freq) || (temp > dev->descr.max_tune_freq)) {
		doutf(DHIGH, "Error1: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*start_freq = (int64_t) temp;
	
	// Convert the 2nd number & make sure no error
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < dev->descr.min_tune_freq) || (temp > dev->descr.max_tune_freq)) {
		doutf(DHIGH, "Error2: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*stop_freq = (int64_t) temp;
	
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	// Convert the number & make sure no error
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*amplitude = (int32_t) temp;

	return 0;
}


/**
 * Set the current trigger mode of the WSA
 * 
 * @param dev - A pointer to the WSA device structure.
 * @param trigger_type - Trigger mode of selection (NONE,LEVEL, OR PULSE).
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_type(struct wsa_device *dev, char const *trigger_type)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if((strcmp(trigger_type, WSA_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA_PULSE_TRIGGER_TYPE) == 0))
		
		sprintf(temp_str, "TRIGGER:TYPE %s \n", trigger_type);
	else
		return WSA_ERR_INVTRIGGERMODE;
    
	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
      	doutf(DHIGH, "In wsa_set_trigger_type: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Gets the WSA's capture mode to triggered 
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
	
	if((strcmp(query.output, WSA_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA_PULSE_TRIGGER_TYPE) == 0))
		strcpy(type, query.output);
	else
		return WSA_ERR_INVTRIGGERMODE;

	return 0;
}


/**
 * Set the WSA's the delay time between each satisfying 
 * trigger (only valid in PULSE trigger mode)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger sync delay (in nanoseconds, must be multiple of 8) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_sync_delay(struct wsa_device *dev, int32_t delay)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (delay > WSA_trigger_SYNC_DELAY_MIN && 
		delay < WSA_trigger_SYNC_DELAY_MAX && 
		delay % WSA_trigger_SYNC_DELAY_MULTIPLE == 0)
		
		sprintf(temp_str, "TRIGGER:DELAY %d \n", delay);
	
	else
		return WSA_ERR_INVTRIGGERDELAY;

    result = wsa_send_command(dev, temp_str);
    if (result < 0) {
        doutf(DHIGH, "In wsa_set_trigger_sync_delay: %d - %s.\n", result, wsa_get_error_msg(result));
    }

    return result;
}


/**
 * Retrieve the WSA's the delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger sync delay (in nanoseconds)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_sync_delay(struct wsa_device *dev, int32_t *delay)
{
	struct wsa_resp query;
	int temp;
	wsa_send_query(dev, "TRIGGER:DELAY?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if (temp < WSA_trigger_SYNC_DELAY_MIN || 
		temp > WSA_trigger_SYNC_DELAY_MAX || 
		temp % WSA_trigger_SYNC_DELAY_MULTIPLE != 0)
		return WSA_ERR_INVTRIGGERDELAY;
	else
		*delay = (int32_t) temp;
		return 0;

}


/**
 * Set the WSA's current synchronization state
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_state - The synchronization state (MASTER or SLAVE) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_trigger_sync_state(struct wsa_device *dev, char const *sync_state)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (strcmp(sync_state, WSA_MASTER_TRIGGER) == 0 || 
		strcmp(sync_state,  WSA_SLAVE_TRIGGER) == 0) 
		
		sprintf(temp_str, "TRIGGER:SYNC %s \n", sync_state);
	
	else
		return WSA_ERR_INVTRIGGERSYNC;

	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
        doutf(DHIGH, "In wsa_set_trigger_sync_state: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Retrieve the WSA's current synchronization state
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_state - The  trigger synchronization state (MASTER or SLAVE) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_trigger_sync_state(struct wsa_device *dev, char *sync_state)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "TRIGGER:SYNC?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	strcpy(sync_state, query.output);
	
	if (strcmp(sync_state, WSA_MASTER_TRIGGER) != 0 && 
		strcmp(sync_state,  WSA_SLAVE_TRIGGER) != 0) 

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
int16_t wsa_set_reference_pll(struct wsa_device *dev, char const *pll_ref)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(pll_ref, "INT") == 0 || strcmp(pll_ref, "EXT") == 0)
		sprintf(temp_str, "SOURCE:REFERENCE:PLL %s\n", pll_ref); 
	else
		return WSA_ERR_INVPLLREFSOURCE;
	
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_reference_pll: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	if (result < 0) {
        doutf(DHIGH, "In wsa_reset_reference_pll: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	double temp;

	wsa_send_query(dev, "LOCK:REFerence?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	if (wsa_to_double(query.output, &temp) < 0)	{
		doutf(DHIGH, "Error: WSA returned '%f'.\n", temp);
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
    double temp;

    wsa_send_query(dev, "LOCK:RF?\n", &query);
    if (query.status <= 0) {
        return (int16_t) query.status;
    }

    if (wsa_to_double(query.output, &temp) < 0) {
        doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
        return WSA_ERR_RESPUNKNOWN;
    }

    *lock_rf = (int32_t) temp;

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// TEMPERATURE CONTROL SECTION                                               //
///////////////////////////////////////////////////////////////////////////////


/**
 * Get the WSA's current temperature
 *
 * @param dev - A pointer to the WSA device structure.
 * @param rfe_temp - A float pointer to store the RFE temperature
 * @param mixer_temp - A float pointer to store the mixer's temperature
 * @param digital_temp - A float pointer to store the digital section'stemperature
 *
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_temperature(struct wsa_device *dev, float* rfe_temp, float* mixer_temp, float* digital_temp)
{
	struct wsa_resp query;		// store query results
	double temp;
	char * strtok_result;
	char * strtok_context = 0;

	wsa_send_query(dev, "STAT:TEMP?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the 1st temperature value 
	strtok_result = strtok_r(query.output, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*rfe_temp = (float) temp;

	// Convert the 2nd temperature value
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*mixer_temp = (float) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);

	// Convert the temperature value
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*digital_temp = (float) temp;

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
	
	if (strcmp(capture_mode, WSA_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMALREADYRUNNING;

	else if (strcmp(capture_mode, WSA_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMWHILESWEEPING;

	result = wsa_send_command(dev, "TRACE:STREAM:START\n");
	if (result < 0) {
        doutf(DHIGH, "Error in wsa_stream_start: %d - %s\n", result, wsa_get_error_msg(result));
    }

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
	
	if (strcmp(capture_mode, WSA_STREAM_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMALREADYRUNNING;

	else if (strcmp(capture_mode, WSA_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_STREAMWHILESWEEPING;
	
	if (stream_start_id < 0 || stream_start_id > UINT_MAX)
		return WSA_ERR_INVSTREAMSTARTID;

	sprintf(temp_str, "TRACE:STREAM:START %lld \n", stream_start_id);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "Error in wsa_stream_start_id: %d - %s\n", result, wsa_get_error_msg(result));
    }

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
	if (strcmp(status, WSA_STREAM_CAPTURE_MODE) != 0)
		return WSA_ERR_STREAMNOTRUNNING;

	result = wsa_send_command(dev, "TRACE:STREAM:STOP\n");
	if (result < 0)	{
		doutf(DHIGH, "Error in wsa_stream_stop: %d - %s\n", result, wsa_get_error_msg(result));
		return result;
	}

	doutf(DHIGH, "Clearing socket buffer... ");
	
	//flush remaining data in the wsa
	result = wsa_flush_data(dev); 
	if (result < 0) {
		return result;
    }
	
	// clean remaining data in the data socket socket
	result = wsa_clean_data_socket(dev);
	if (result < 0) {
		return result;
    }

	doutf(DHIGH, "done.\n");
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
	int temp;

	if (strcmp(dev->descr.prod_model,WSA5000) == 0) {
		return WSA_ERR_INV5000COMMAND;
    }

	wsa_send_query(dev, "SWEEP:ENTRY:ANTENNA?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < 1) || (temp > WSA_4000_MAX_ANT_PORT)) {
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp); 
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

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;
	
	if (port_num < 1 || port_num > WSA_4000_MAX_ANT_PORT)
		return WSA_ERR_INVANTENNAPORT;

	sprintf(temp_str, "SWEEP:ENTRY:ANTENNA %d\n", port_num);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_antenna: %d - %s.\n", result, wsa_get_error_msg(result));
    }
		
	return result;
}


/**
 * Gets the sweep entry's attenuator's mode of operation
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer pointer to store the attenuator's mode
 * of operation
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_sweep_attenuation(struct wsa_device *dev, int32_t *mode)
{
	struct wsa_resp query;
	int temp;
	
	if (strcmp(dev->descr.prod_model,WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	wsa_send_query(dev, "SWEEP:ENTRY:ATTENUATOR?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }
	
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if ((int32_t) temp != WSA_ATTEN_ENABLED  && (int32_t) temp != WSA_ATTEN_DISABLED) {
		return WSA_ERR_INVATTEN;
    }

	*mode = (int32_t) temp;
	
	return 0;
}


/**
 * Sets the sweep entry's attenuator's mode of operation (0 = Off/1 = On)
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - An integer pointer containing the attenuation's mode of operation
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_attenuation(struct wsa_device *dev, int32_t mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model,WSA4000) == 0)
		return WSA_ERR_INV4000COMMAND;
	
	if (mode != WSA_ATTEN_ENABLED  && mode != WSA_ATTEN_DISABLED)
		return WSA_ERR_INVATTEN;

	sprintf(temp_str, "SWEEP:ENTRY:ATTENUATOR %d\n", mode);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_attenuation: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
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
	int temp;

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

	wsa_send_query(dev, "SWEEP:ENTRY:GAIN:IF?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	// Verify the validity of the return value
	if (temp < dev->descr.min_if_gain || temp > dev->descr.max_if_gain) 
	{
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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
	
	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

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

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

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
int16_t wsa_set_sweep_gain_rf(struct wsa_device *dev, char const *gain)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model,WSA5000) == 0)
		return WSA_ERR_INV5000COMMAND;

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
 * Query the WSA5000's RFE mode of operation in the sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer to store the RFE input mode
 *
 * @return 0 on successful, or a negative number on error.
 */
int16_t wsa_get_sweep_rfe_input_mode(struct wsa_device *dev, char *mode)
{
	struct wsa_resp query;		// store query results

	if (strcmp(dev->descr.prod_model , WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	wsa_send_query(dev, "SWEEP:ENTRY:MODE?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	strcpy(mode,query.output);

	if ((strcmp(mode, WSA_RFE_ZIF_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_DD_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_HDR_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_SH_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_SHN_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_IQIN_STRING) != 0)) {
		return WSA_ERR_INVRFEINPUTMODE;
    }

	return 0;
}


/**
 * Sets the RFE's input mode of the WSA5000 in the sweep entry
 * Valid RFE modes are: ZIF, HDR, SH
 *
 * @param dev - A pointer to the WSA device structure.
 * @param mode - A char pointer containing the RFE input mode\n
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_rfe_input_mode(struct wsa_device *dev, char const *mode)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if (strcmp(dev->descr.prod_model , WSA4000) == 0) {
		return WSA_ERR_INV4000COMMAND;
    }

	if ((strcmp(mode, WSA_RFE_ZIF_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_DD_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_HDR_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_SH_STRING)   != 0) &&
		(strcmp(mode, WSA_RFE_SHN_STRING)  != 0) &&
		(strcmp(mode, WSA_RFE_IQIN_STRING) != 0)) {
		return WSA_ERR_INVRFEINPUTMODE;
    }

	sprintf(temp_str, "SWEEP:ENTRY:MODE %s\n", mode);

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_rfe_input_mode: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int temp;

	wsa_send_query(dev, "SWEEP:ENTRY:SPPACKET?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	// Verify the validity of the return value
	if ((temp < WSA_MIN_SPP) || (temp > WSA_MAX_SPP)) {
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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

	if ((samples_per_packet < WSA_MIN_SPP) || 
		(samples_per_packet > WSA_MAX_SPP) || 
		((samples_per_packet % WSA_SPP_MULTIPLE) != 0))
		return WSA_ERR_INVSAMPLESIZE;

	sprintf(temp_str, "SWEEP:ENTRY:SPPACKET %u\n", samples_per_packet);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_samples_per_packet: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int temp;

	wsa_send_query(dev, "SWEEP:ENTRY:PPBLOCK?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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

	if (packets_per_block < WSA_MIN_PPB)
		return WSA_ERR_INVNUMBER;
	else if (packets_per_block > WSA_MAX_PPB) 
		return WSA_ERR_INVCAPTURESIZE;

	sprintf(temp_str, "SWEEP:ENTRY:PPBLOCK %d\n", packets_per_block);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_packets_per_block: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int temp;

	wsa_send_query(dev, ":SWEEP:ENTRY:DECIMATION?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// convert & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }

	// make sure the returned value is valid
	if (((temp != 1) && (temp < dev->descr.min_decimation)) || 
		(temp > dev->descr.max_decimation))
	{
		doutf(DHIGH, "Error: WSA returned '%d'.\n", temp);
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
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_decimation: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char * strtok_result;
	char * strtok_context = 0;

	wsa_send_query(dev, "SWEEP:ENTRY:FREQ:CENTER?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	strtok_result = strtok_r(query.output, ",", &strtok_context);
	// Convert the number & make sure no error
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	*start_freq = (int64_t) temp;
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	// Convert the number & make sure no error
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
		
	// make sure stop_freq is larger than (or equal to) start
	if (stop_freq < start_freq)
		return  WSA_ERR_INVSTOPFREQ;
		
	sprintf(temp_str, "SWEEP:ENTRY:FREQ:CENT %lld Hz, %lld Hz\n", start_freq, stop_freq);
	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_freq: %d - %s.\n", result, wsa_get_error_msg(result));
    }
		
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
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_freq_shift: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_freq_step: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0)	{
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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

	if ((seconds < 0) || (microseconds < 0)) {
  	  return WSA_ERR_INVDWELL;
    }

	sprintf(temp_str, "SWEEP:ENTRY:DWELL %u,%u\n", seconds, microseconds);
	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_dwell: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char * strtok_result;
	char * strtok_context = 0;

	wsa_send_query(dev, "SWEEP:ENTRY:DWELL?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the 1st number & make sure no error
	strtok_result = strtok_r(query.output, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*seconds = (int32_t) temp;
	
	// Convert the 2nd number & make sure no error
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
	if (result == WSA_ERR_FREQOUTOFBOUND) {
		return WSA_ERR_STARTOOB;
    }

	result = wsa_verify_freq(dev, stop_freq);

	if (result == WSA_ERR_FREQOUTOFBOUND) {
		return WSA_ERR_STOPOOB;
    }

	if (stop_freq <= start_freq) {
		return WSA_ERR_INVSTOPFREQ;
    }

	sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:LEVEL %lld,%lld,%d\n", start_freq, stop_freq, amplitude);
	result = wsa_send_command(dev, temp_str);

	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_trigger_level: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char * strtok_result;
	char * strtok_context = 0;
	
	wsa_send_query(dev, "SWEEP:ENTRY:TRIGGER:LEVEL?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the 1st number & make sure no error
	strtok_result = strtok_r(query.output, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*start_freq = (int64_t) temp;

	// Convert the 2nd number & make sure no error
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	*stop_freq = (int64_t) temp;

	// Convert the 3rd number & make sure no error
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
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
int16_t wsa_set_sweep_trigger_type(struct wsa_device *dev, char const *trigger_type)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];

	if((strcmp(trigger_type, WSA_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(trigger_type, WSA_PULSE_TRIGGER_TYPE) == 0))
		sprintf(temp_str, "SWEEP:ENTRY:TRIGGER:TYPE %s \n", trigger_type);
	else
		return WSA_ERR_INVTRIGGERMODE;

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_trigger_type: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	
	if((strcmp(query.output, WSA_NONE_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA_LEVEL_TRIGGER_TYPE) == 0) || 
		(strcmp(query.output, WSA_PULSE_TRIGGER_TYPE) == 0))
		strcpy(trigger_type,query.output);
	
	else
		return WSA_ERR_INVTRIGGERMODE;
	return 0;
}


/**
 * Set the sweep entry's delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger sync delay (in nanoseconds, must be multiple of 8) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t delay)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (delay > WSA_trigger_SYNC_DELAY_MIN && 
		delay < WSA_trigger_SYNC_DELAY_MAX && 
		delay % WSA_trigger_SYNC_DELAY_MULTIPLE == 0)
		
		sprintf(temp_str, "SWEEP:LIST:TRIGGER:DELAY %d \n", delay);
	
	else
		return WSA_ERR_INVTRIGGERDELAY;

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_trigger_sync_delay: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Retrieve sweep entry's delay time between each satisfying trigger
 *
 * @param dev - A pointer to the WSA device structure.
 * @param delay - The trigger sync delay (in nanoseconds)
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t *delay)
{
	struct wsa_resp query;
	int temp;

	wsa_send_query(dev, "SWEEP:LIST:TRIGGER:DELAY?\n", &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_int(query.output, &temp) < 0) {
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}

	if (temp < WSA_trigger_SYNC_DELAY_MIN || 
		temp > WSA_trigger_SYNC_DELAY_MAX || 
		temp % WSA_trigger_SYNC_DELAY_MULTIPLE != 0)
		return WSA_ERR_INVTRIGGERDELAY;
	else
		*delay = (int32_t) temp;
		return 0;
}


/**
 * Set the WSA's current synchronization state in the sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_state - The synchronization state (MASTER/SLAVE) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_set_sweep_trigger_sync_state(struct wsa_device *dev, char const *sync_state)
{
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	if (strcmp(sync_state, WSA_MASTER_TRIGGER) == 0 || 
		strcmp(sync_state,  WSA_SLAVE_TRIGGER) == 0) 
		
		sprintf(temp_str, "SWEEP:LIST:TRIGGER:SYNC %s \n", sync_state);
	
	else
		return WSA_ERR_INVTRIGGERSYNC;

	result = wsa_send_command(dev, temp_str);
	if (result < 0) {
        doutf(DHIGH, "In wsa_set_sweep_trigger_sync_state: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * Retrieve the WSA's current synchronization state in the sweep entry
 *
 * @param dev - A pointer to the WSA device structure.
 * @param sync_state - The  trigger synchronization mode (MASTER/SLAVE) 
 * 
 * @return 0 on success, or a negative number on error.
 */
int16_t wsa_get_sweep_trigger_sync_state(struct wsa_device *dev, char *sync_state)
{
	struct wsa_resp query;
	
	wsa_send_query(dev, "SWEEP:LIST:TRIGGER:SYNC?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;
	
	strcpy(sync_state, query.output);
	
	if (strcmp(sync_state, WSA_MASTER_TRIGGER) != 0 || 
		strcmp(sync_state,  WSA_SLAVE_TRIGGER) != 0) 

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
	if ((strcmp(query.output, WSA_SWEEP_STATE_STOPPED) != 0) &&
		(strcmp(query.output, WSA_SWEEP_STATE_RUNNING) != 0))
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
	if (query.status <= 0) {
		return (int16_t) query.status;
    }

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0)	{
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
    if (result < 0) {
    	doutf(DHIGH, "In wsa_sweep_entry_delete: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
    if (result < 0) {
    	doutf(DHIGH, "In wsa_sweep_entry_delete: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	if (id < 0 || id > size) {
		return WSA_ERR_SWEEPIDOOB;
    }

	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0) {
		return result;
    }

	if (size == 0) {
		return  WSA_ERR_SWEEPLISTEMPTY;
    }

	sprintf(temp_str, "SWEEP:ENTRY:COPY %u\n", id);
	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
        doutf(DHIGH, "In wsa_sweep_entry_copy: %d - %s.\n", result, wsa_get_error_msg(result));
    }

	return result;
}


/**
 * retrieve the number iterations of the sweep list
 *
 * @param dev - A pointer to the WSA device structure
 * @param iteration - An int containing pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_get_sweep_iteration(struct wsa_device *dev, int32_t *iterations)
{	
	struct wsa_resp query;		// store query results
	double temp;

	wsa_send_query(dev, "SWEEP:LIST:ITER?\n", &query);
	if (query.status <= 0)
		return (int16_t) query.status;

	// Convert the number & make sure no error
	if (wsa_to_double(query.output, &temp) < 0)	{
		doutf(DHIGH, "Error: WSA returned '%s'.\n", query.output);
		return WSA_ERR_RESPUNKNOWN;
	}
	
	*iterations = (int32_t) temp;
		
	return 0;
}

/**
 * set the number iterations of the sweep list
 *
 * @param dev - A pointer to the WSA device structure
 * @param iteration - An int containing pointer to the WSA device structure
 *
 * @return 0 on success, or a negative number on error
 */
int16_t wsa_set_sweep_iteration(struct wsa_device *dev, int32_t iteration)
{	
	int16_t result = 0;
	char temp_str[MAX_STR_LEN];
	
	sprintf(temp_str, "SWEEP:LIST:ITER %d \n", iteration);
	
	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
    	doutf(DHIGH, "In  wsa_set_sweep_iteration: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	int32_t size = 0;

	result = wsa_send_command(dev, "SWEEP:LIST:START\n");
    if (result < 0) {
    	doutf(DHIGH, "In wsa_sweep_start: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
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
	if (strcmp(status, WSA_SWEEP_CAPTURE_MODE) == 0)
		return WSA_ERR_SWEEPALREADYRUNNING;
	
	// check if the wsa is streaming
	if (strcmp(status, WSA_STREAM_CAPTURE_MODE) == 0)
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
    if (result < 0) {
	    doutf(DHIGH, "In wsa_sweep_start_id: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
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
	if (strcmp(status, WSA_SWEEP_CAPTURE_MODE) != 0)
		return WSA_ERR_SWEEPNOTRUNNING;
	
	result = wsa_send_command(dev, "SWEEP:LIST:STOP\n");
	if (result < 0) {
      	doutf(DHIGH, "In wsa_sweep_stop: %d - %s.\n", result, wsa_get_error_msg(result));
		return result;
    }
	
	doutf(DHIGH, "Clearing socket buffer... ");
	
	// flush remaining data in the wsa
	result = wsa_flush_data(dev); 
	if (result < 0) {
		return result;
    }

	// clean remaining data in the data socket socket
	result = wsa_clean_data_socket(dev);
	if (result < 0) {
		return result;
    }

	doutf(DHIGH, "done.\n");
	
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
	if (strcmp(status, WSA_SWEEP_STATE_RUNNING) == 0) 
		return WSA_ERR_SWEEPALREADYRUNNING;

	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0)
		return result;
	if (size <= 0)
		return WSA_ERR_SWEEPLISTEMPTY;

	result = wsa_send_command(dev, "SWEEP:LIST:RESUME\n");
    if (result < 0) {
        doutf(DHIGH, "In wsa_sweep_resume: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
    if (result < 0) {
      	doutf(DHIGH, "In wsa_sweep_entry_new: %d - %s.\n", result, wsa_get_error_msg(result));
    }
	
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

    if(id) {
	  if((id < 0) || (id > size+1)) {
        return WSA_ERR_SWEEPIDOOB;
      }    
  	  sprintf(temp_str, "SWEEP:ENTRY:SAVE %u\n", id);
    } else {
	  sprintf(temp_str, "SWEEP:ENTRY:SAVE\n");
    }
  
	result = wsa_send_command(dev, temp_str);
    if (result < 0) {
    	doutf(DHIGH, "In wsa_sweep_entry_save: %d - %s.\n", result, wsa_get_error_msg(result));
    }

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
	char * strtok_result;
	char * strtok_context = 0;
	
	// check if id is out of bounds
	result = wsa_get_sweep_entry_size(dev, &size);
	if (result < 0) {
		return result;
    }

	if ((id < 0) || (id > size)) {
		return WSA_ERR_SWEEPIDOOB;
    }

	sprintf(temp_str, "SWEEP:ENTRY:READ? %d\n", id);
	wsa_send_query(dev, temp_str, &query);
	if (query.status <= 0) {
		return (int16_t) query.status;
    }
	
	// *****
	// Convert the numbers & make sure no error
	// ****

	strtok_result = strtok_r(query.output, ",", &strtok_context);
	strcpy(sweep_list->rfe_mode,strtok_result);

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->start_freq = (int64_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->stop_freq = (int64_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;	
    }
	sweep_list->fstep = (int64_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->fshift = (float) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;	
    }
	sweep_list->decimation_rate = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->attenuator = (int32_t) temp;
	
	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->gain_if = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->gain_hdr = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->samples_per_packet = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->packets_per_block = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;
    }
	sweep_list->dwell_seconds = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);
	if (wsa_to_double(strtok_result, &temp) < 0) {
		return WSA_ERR_RESPUNKNOWN;	
    }
	sweep_list->dwell_microseconds = (int32_t) temp;

	strtok_result = strtok_r(NULL, ",", &strtok_context);	
	if (strstr(strtok_result, WSA_LEVEL_TRIGGER_TYPE) != NULL) {
		strcpy(sweep_list->trigger_type,strtok_result);

		strtok_result = strtok_r(NULL, ",", &strtok_context);
		if (wsa_to_double(strtok_result, &temp) < 0) {
			return WSA_ERR_RESPUNKNOWN;
        }
		sweep_list->trigger_start_freq = (int64_t) temp;
		
		strtok_result = strtok_r(NULL, ",", &strtok_context);
		if (wsa_to_double(strtok_result, &temp) < 0) {
			return WSA_ERR_RESPUNKNOWN;
        }
		sweep_list->trigger_stop_freq = (int64_t) temp;

		strtok_result = strtok_r(NULL, ",", &strtok_context);
		if (wsa_to_double(strtok_result, &temp) < 0) {
			return WSA_ERR_RESPUNKNOWN;	
        }
		sweep_list->trigger_amplitude = (int32_t) temp;
	} else {
		strcpy(sweep_list->trigger_type,strtok_result);
    }

	return 0;
}

