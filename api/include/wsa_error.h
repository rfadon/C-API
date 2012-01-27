#ifndef __WSA_ERROR_H__
#define __WSA_ERROR_H__

#include "stdint.h"
#include "wsa_debug.h"

//*****************************************************************************
// Defines error parameters & and associated messages
//*****************************************************************************

// A large 16-bit negative number
#define LNEG_NUM (-10000)


// ///////////////////////////////
// WSA RELATED ERRORS			//
// ///////////////////////////////
#define WSA_ERR_NOWSA			(LNEG_NUM - 1)
#define WSA_ERR_UNKNOWNPRODSER	(LNEG_NUM - 2)
#define WSA_ERR_UNKNOWNPRODVSN	(LNEG_NUM - 3)
#define WSA_ERR_UNKNOWNFWRVSN	(LNEG_NUM - 4)
#define WSA_ERR_UNKNOWNRFEVSN	(LNEG_NUM - 5)
#define WSA_ERR_PRODOBSOLETE	(LNEG_NUM - 6)


// ///////////////////////////////
// WSA SETUP ERRORS				//
// ///////////////////////////////
#define WSA_ERR_WSANOTRDY		(LNEG_NUM - 101)
#define WSA_ERR_WSAINUSE		(LNEG_NUM - 102)
#define WSA_ERR_SETFAILED		(LNEG_NUM - 103)
#define WSA_ERR_OPENFAILED		(LNEG_NUM - 104)
#define WSA_ERR_INITFAILED		(LNEG_NUM - 105)
#define WSA_ERR_INVADCCORRVALUE	(LNEG_NUM - 106)



// ///////////////////////////////
// INTERFACE/CONNECTION ERRORS  //
// ///////////////////////////////
#define WSA_ERR_INVINTFMETHOD	(LNEG_NUM - 201)
#define WSA_ERR_USBNOTAVBL		(LNEG_NUM - 202)
#define WSA_ERR_USBOPENFAILED	(LNEG_NUM - 203)
#define WSA_ERR_USBINITFAILED	(LNEG_NUM - 204)

#define WSA_ERR_INVIPHOSTADDRESS	(LNEG_NUM - 205)
#define WSA_ERR_ETHERNETNOTAVBL	(LNEG_NUM - 206)
#define WSA_ERR_ETHERNETCONNECTFAILED	(LNEG_NUM - 207)
#define WSA_ERR_ETHERNETINITFAILED	(LNEG_NUM - 209)
#define WSA_ERR_WINSOCKSTARTUPFAILED (LNEG_NUM - 210)
#define WSA_ERR_SOCKETSETFUPFAILED	(LNEG_NUM - 211)
#define WSA_ERR_SOCKETERROR	(LNEG_NUM - 212)
#define WSA_ERR_SOCKETDROPPED (LNEG_NUM - 213)


// ///////////////////////////////
// AMPLITUDE ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVAMP	(LNEG_NUM - 301)


// ///////////////////////////////
// DATA ACQUISITION ERRORS		//
// ///////////////////////////////
#define WSA_ERR_NODATABUS		(LNEG_NUM - 401)
#define WSA_ERR_READFRAMEFAILED	(LNEG_NUM - 402)
#define WSA_ERR_INVSAMPLESIZE	(LNEG_NUM - 403)
#define WSA_ERR_SIZESETFAILED	(LNEG_NUM - 404)
#define WSA_ERR_NOTIQFRAME	(LNEG_NUM - 405)
#define WSA_ERR_INVDECIMATIONRATE (LNEG_NUM - 406)


// ///////////////////////////////
// FREQUENCY ERRORS				//
// ///////////////////////////////
#define WSA_ERR_FREQOUTOFBOUND	(LNEG_NUM - 601)
#define WSA_ERR_INVFREQRES		(LNEG_NUM - 602)
#define WSA_ERR_FREQSETFAILED	(LNEG_NUM - 603)
#define WSA_ERR_PLLLOCKFAILED	(LNEG_NUM - 604)


// ///////////////////////////////
// GAIN ERRORS					//
// ///////////////////////////////
#define WSA_ERR_INVRFGAIN	(LNEG_NUM - 801)
#define WSA_ERR_INVIFGAIN	(LNEG_NUM - 802)
#define WSA_ERR_IFGAINSETFAILED (LNEG_NUM - 803)
#define WSA_ERR_RFGAINSETFAILED (LNEG_NUM - 804)


// ///////////////////////////////
// RUNMODE ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVRUNMODE	(LNEG_NUM - 1001)


// ///////////////////////////////
// TRIGGER ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVTRIGID		(LNEG_NUM - 1201)
#define WSA_ERR_INVSTOPFREQ		(LNEG_NUM - 1202)
#define WSA_ERR_STARTOOB		(LNEG_NUM - 1203)
#define WSA_ERR_STOPOOB			(LNEG_NUM - 1204)
#define WSA_ERR_INVSTARTRES		(LNEG_NUM - 1205)
#define WSA_ERR_INVSTOPRES		(LNEG_NUM - 1206)
#define WSA_ERR_INVTRIGRANGE	(LNEG_NUM - 1207)
#define WSA_ERR_INVDWELL		(LNEG_NUM - 1208)
#define WSA_ERR_INVNUMFRAMES	(LNEG_NUM - 1209)


// ///////////////////////////////
// CTRL/CMD ERRORS				//
// ///////////////////////////////
#define WSA_ERR_NOCTRLPIPE		(LNEG_NUM - 1500)
#define WSA_ERR_CMDSENDFAILED	(LNEG_NUM - 1501)
#define WSA_ERR_CMDINVALID		(LNEG_NUM - 1502)
#define WSA_ERR_RESPUNKNOWN	(LNEG_NUM - 1503)
#define WSA_ERR_QUERYNORESP	(LNEG_NUM - 1504)


// ///////////////////////////////
// RFE ERRORS				    //
// ///////////////////////////////
#define WSA_ERR_INVANTENNAPORT (LNEG_NUM - 1601)
#define WSA_ERR_ANTENNASETFAILED (LNEG_NUM - 1602)
#define WSA_ERR_INVFILTERMODE (LNEG_NUM - 1603)
#define WSA_ERR_FILTERSETFAILED (LNEG_NUM - 1604)
#define WSA_ERR_INVCALIBRATEMODE (LNEG_NUM - 1605)
#define WSA_ERR_CALIBRATESETFAILED (LNEG_NUM - 1606)
#define WSA_ERR_INVRFESETTING (LNEG_NUM - 1607)


// ///////////////////////////////
// FILE RELATED ERRORS			//
// ///////////////////////////////
#define WSA_ERR_FILECREATEFAILED (LNEG_NUM - 1900)
#define WSA_ERR_FILEOPENFAILED (LNEG_NUM - 1901)
#define WSA_ERR_FILEREADFAILED (LNEG_NUM - 1902)
#define WSA_ERR_FILEWRITEFAILED (LNEG_NUM - 1903)


// ///////////////////////////////
// OTHERS ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVNUMBER		(LNEG_NUM - 2000)
#define WSA_ERR_INVREGADDR		(LNEG_NUM - 2001)
#define WSA_ERR_MALLOCFAILED	(LNEG_NUM - 2002)
#define WSA_ERR_UNKNOWN_ERROR	(LNEG_NUM - 2003)


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

const char *_wsa_get_err_msg(int16_t err_id);

#endif
