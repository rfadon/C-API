#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "wsa_commons.h"
#include "wsa_error.h"

/**
 * Returns the error message based on the error ID given
 *
 * @param err_id The error ID
 */
const char *_wsa_get_err_msg(int16_t err_id)
{
	static struct wsa_err_item {
		int16_t err_id;
		const char *err_msg;
	} wsa_err_list[] = {
		{0, "No error"},
	
		//*****
		// WSA Related
		//*****
		{WSA_ERR_NOWSA, "No WSA detected. Check the power or the connection"},
		{WSA_ERR_UNKNOWNPRODSER, "Unknown WSA product serial number detected"},
		{WSA_ERR_UNKNOWNPRODVSN, "Unknown WSA product version detected"},
		{WSA_ERR_UNKNOWNRFEVSN, "Unknown WSA RFE version detected"},
		{WSA_ERR_UNKNOWNFWRVSN, "Unknown WSA firmware version detected"},
		{WSA_ERR_PRODOBSOLETE, "Product is obsolete and not supported"},

		//*****
		// WSA Setup Related
		//*****
		{WSA_ERR_WSANOTRDY, "WSA does not seem to be ready"},
		{WSA_ERR_WSAINUSE, "WSA is in use at the moment"},
		{WSA_ERR_SETFAILED, "Failed to set the WSA"},
		{WSA_ERR_OPENFAILED, "Unable to open the WSA"},
		{WSA_ERR_INITFAILED, "Unable to initialize the WSA"},
		{WSA_ERR_INVADCCORRVALUE, 
			"Invalid ADC correction value (use -1, 0 to 13)"},


		//*****
		// WSA Interface/Connection Related
		//*****
		{WSA_ERR_INVINTFMETHOD, "Invalid connection type"},
		{WSA_ERR_USBNOTAVBL, "USB connection is not available with this WSA"},
		{WSA_ERR_ETHERNETNOTAVBL, 
			"Ethernet connection is not available with this WSA"},
		{WSA_ERR_USBOPENFAILED, "Unable to open the WSA's USB connection"},
		{WSA_ERR_USBINITFAILED, "Unable to initialize the WSA's USB component"},
		{WSA_ERR_INVIPHOSTADDRESS, "Invalid IP or Host Name given"},
		{WSA_ERR_ETHERNETCONNECTFAILED, 
			"Unable to establish the WSA's Ethernet connection"},
		{WSA_ERR_ETHERNETINITFAILED,
			"Unable to initialize the WSA's Ethernet component"},
		{WSA_ERR_WINSOCKSTARTUPFAILED,
			"Unable to start up Windows Socket with this system"},
		{WSA_ERR_SOCKETSETFUPFAILED, "Socket setup failed"},
		{WSA_ERR_SOCKETERROR,
			"Socket error/closed. The Ethernet connection might have been closed"},
		{WSA_ERR_SOCKETDROPPED,
			"Socket connection unexpectedly dropped. Close this application and "
			"check the Ethernet connection (repower WSA if necessary)"},

		//*****
		// Amplitude related
		//*****
		{WSA_ERR_INVAMP, "Invalid amplitude value"},

		//*****
		// Data acquisition related
		//*****
		{WSA_ERR_NODATABUS, "No data bus detected. Possible firmware error"},
		{WSA_ERR_READFRAMEFAILED, "Failed reading a WSA frame"},
		{WSA_ERR_INVSAMPLESIZE, "Invalid sample size"},	
		{WSA_ERR_SIZESETFAILED, "Failed to set the sample size"},
		{WSA_ERR_NOTIQFRAME, "Not an IQ packet frame"},
		{WSA_ERR_INVDECIMATIONRATE, "Invalid decimation rate"},

		//*****
		// Frequency related
		//*****
		{WSA_ERR_FREQOUTOFBOUND, "The frequency input is out of bound"},
		{WSA_ERR_INVFREQRES, "Invalid frequency resolution"},
		{WSA_ERR_FREQSETFAILED, "Failed tuning the frequency in the WSA"},
		{WSA_ERR_PLLLOCKFAILED, "The WSA's PLL failed to lock"},
	
		//*****
		// Gain related
		//*****
		{WSA_ERR_INVRFGAIN, "Invalid RF gain setting"},
		{WSA_ERR_INVIFGAIN, "IF gain value out of range or invalid"},
		{WSA_ERR_IFGAINSETFAILED, "Failed setting the IF gain setting to WSA"},
		{WSA_ERR_RFGAINSETFAILED, "Failed setting the RF gain value to WSA"},

		//*****
		// Run mode related
		//*****
		{WSA_ERR_INVRUNMODE, "Invalide run mode"},

		//*****
		// Trigger related
		//*****
		{WSA_ERR_INVTRIGID, "Invalid trigger ID"},
		{WSA_ERR_INVSTOPFREQ, 
			"Invalid stop frequency (must be larger than the start frequency"},
		{WSA_ERR_STARTOOB, "Start frequency is out of bound"},
		{WSA_ERR_STOPOOB, "Stop frequency is out of bound"},
		{WSA_ERR_INVSTARTRES, "Invalid start frequency resolution"},
		{WSA_ERR_INVSTOPRES, "Invalid stop frequency resolution"},
		{WSA_ERR_INVTRIGRANGE, 
			"Invalid trigger range between the start & stop frequencies"},
		{WSA_ERR_INVDWELL, "Invalid trigger dwell time"},
		{WSA_ERR_INVNUMFRAMES, "Invalid number of frames to capture"},

		//*****
		// CONTROL/COMMAND ERRORS
		//*****
		{WSA_ERR_NOCTRLPIPE, 
			"No control channel detected. Possible firmware error"},
		{WSA_ERR_CMDSENDFAILED, "Sending of the command failed"},
		{WSA_ERR_CMDINVALID, "Command is not valid or incorrectly written"},
		{WSA_ERR_RESPUNKNOWN, 
			"The response received is invalid for the query sent"},
		{WSA_ERR_QUERYNORESP, "Query returns no response"},


		//*****
		// RFE SECTION
		//*****
		{WSA_ERR_INVANTENNAPORT, "Invalid antenna switch port"},
		{WSA_ERR_ANTENNASETFAILED, "Failed to set the antenna port"},
		{WSA_ERR_INVFILTERMODE, "Invalid filter mode"},
		{WSA_ERR_FILTERSETFAILED, "Failed to set the filter mode"},
		{WSA_ERR_INVCALIBRATEMODE, "Invalid calibration mode"},
		{WSA_ERR_CALIBRATESETFAILED, "Failed to set the calibration mode"},
		{WSA_ERR_INVRFESETTING, 
			"This setting is not valid with the current RFE product"},


		//*****
		// File related
		//*****
		{WSA_ERR_FILECREATEFAILED, "Unable to create the file"},
		{WSA_ERR_FILEOPENFAILED, "Unable to open the file"},
		{WSA_ERR_FILEREADFAILED, "Unable to read the file"},
		{WSA_ERR_FILEWRITEFAILED, "Unable to write to the file"},

		//*****
		// Others
		//*****
		{WSA_ERR_INVNUMBER, 
			"Invalid number format. Check the required number type"},
		{WSA_ERR_INVREGADDR, "Invalid register address"},
		{WSA_ERR_MALLOCFAILED, "Memory allocation failed"},
		{WSA_ERR_UNKNOWN_ERROR, "Unknow error"}	
	};

	int id = 0;

	// This loop is not efficient.  Should probably do a binary 
	// search instead but the error list is small right now.
	do
	{
		if (wsa_err_list[id].err_id == err_id) 
		{
			return wsa_err_list[id].err_msg;
			break;
		}
		else id++;
	} while (wsa_err_list[id].err_msg != NULL);
	
	//return wsa_err_list[WSA_ERR_UNKNOWN_ERROR].err_msg;
	return _wsa_get_err_msg(WSA_ERR_UNKNOWN_ERROR);
}


/**
 * Tokenized all the words/strings in a file.
 * Pointer to the tokens is stored in cmd_strs.
 * Return the number of tokens read.
 */
int16_t wsa_tokenize_file(FILE *fptr, char *cmd_strs[])
{
	long fSize;
	char *buffer;
	char *fToken;
	int16_t next = 0;
	int i;

	fseek(fptr, 0, SEEK_END);	// Reposition fptr posn indicator to the end
	fSize = ftell(fptr);		// obtain file size
	rewind(fptr);
	
	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char) * fSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr); 	
		return WSA_ERR_MALLOCFAILED;
	}
	fread(buffer, 1, fSize, fptr);	// copy the file into the buffer
	doutf(DLOW, "\nFile content: \n%s\n", buffer);
	
	for (i = 0; i < MAX_FILE_LINES; i++) 
		strcpy(cmd_strs[i], "");

	fToken = strtok(buffer, SEP_CHARS);
	while (fToken != NULL)	{
		doutf(DLOW, "%d fToken (%d) = %s\n", next, strlen(fToken), fToken);
		// Avoid taking any empty line
		if (strpbrk(fToken, ":*?") != NULL) {
			strcpy(cmd_strs[next], fToken);
			next++;
		}
		fToken = strtok(NULL, SEP_CHARS);
	}

	free(buffer);

	return next;
}

/**
 * Convert a string to a long number type
 */
int16_t to_int(char *num_str, long int *val)
{
	char *temp;
	long int temp_val;

	errno = 0; // to distinguish success/failure after calling strtol
	temp_val = strtol(num_str, &temp, 0);
	if ((errno == ERANGE && (temp_val == LONG_MAX || temp_val == LONG_MIN))
		|| (errno != 0 && temp_val == 0) || temp == num_str) {
		perror("strtol");
		return WSA_ERR_INVNUMBER;
	}

	*val = temp_val;

	return 0;
}

/**
 * Convert a string to a long number type
 */
int16_t to_double(char *num_str, double *val)
{
	char *temp;
	double temp_val;

	errno = 0; // to distinguish success/failure after calling strtol
	temp_val = strtod(num_str, &temp);
	if (errno == ERANGE || (errno != 0 && temp_val == 0) || temp == num_str) {
		perror("strtod");
		return WSA_ERR_INVNUMBER;
	}

	*val = temp_val;

	return 0;
}


int16_t string_to_integer(const char* from_string, int32_t* to_integer)
{
	char* end_of_string;
	errno = 0;

	*to_integer = strtol(from_string, &end_of_string, 10);

	if (errno != 0) {
		errno = 0;
		return (int16_t) errno;
	}
	else if (from_string == end_of_string) {
		return WSA_ERR_INVIFGAIN;
	}
	else if (*end_of_string != 0) {
		return WSA_ERR_INVIFGAIN;
	}

	return 0;
}
