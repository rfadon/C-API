#ifndef __WSA_ERROR_H__
#define __WSA_ERROR_H__

#include "thinkrf_stdint.h"
#include "wsa_debug.h"

//*****************************************************************************
// Defines error parameters & and associated messages
//*****************************************************************************

// A large 16-bit negative number
#define LNEG_NUM (-10000)
#define WARNING_NUM (LNEG_NUM  - 10000)

// ///////////////////////////////
// WSA RELATED ERRORS			//
// ///////////////////////////////
#define WSA_ERR_NOWSA			(LNEG_NUM - 1)
#define WSA_ERR_UNKNOWNPRODSER	(LNEG_NUM - 2)
#define WSA_ERR_UNKNOWNPRODVSN	(LNEG_NUM - 3)
#define WSA_ERR_UNKNOWNFWRVSN	(LNEG_NUM - 4)
#define WSA_ERR_UNKNOWNRFEVSN	(LNEG_NUM - 5)
#define WSA_ERR_PRODOBSOLETE	(LNEG_NUM - 6)
#define WSA_ERR_DATAACCESSDENIED (LNEG_NUM - 7)
#define WSA_ERR_INV4000COMMAND (LNEG_NUM - 8)
#define WSA_ERR_INV5000COMMAND (LNEG_NUM - 9)


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
#define WSA_ERR_INVLANCONFIG (LNEG_NUM - 214)

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
#define WSA_ERR_NOTIQFRAME	(LNEG_NUM - 405)
#define WSA_ERR_INVDECIMATIONRATE (LNEG_NUM - 406)
#define WSA_ERR_VRTPACKETSIZE (LNEG_NUM - 407)
#define WSA_ERR_INVTIMESTAMP (LNEG_NUM - 408)
#define WSA_ERR_INVCAPTURESIZE (LNEG_NUM - 409)
#define WSA_ERR_PACKETOUTOFORDER (LNEG_NUM - 410)
#define WSA_ERR_CAPTUREACCESSDENIED  (LNEG_NUM - 411)


// ///////////////////////////////
// FREQUENCY ERRORS				//
// ///////////////////////////////
#define WSA_ERR_FREQOUTOFBOUND	(LNEG_NUM - 601)
#define WSA_ERR_INVFREQRES		(LNEG_NUM - 602)
#define WSA_ERR_PLLLOCKFAILED	(LNEG_NUM - 603)
#define WSA_ERR_INVSTOPFREQ		(LNEG_NUM - 604)
#define WSA_ERR_STARTOOB		(LNEG_NUM - 605)
#define WSA_ERR_STOPOOB			(LNEG_NUM - 606)


// ///////////////////////////////
// GAIN ERRORS	            	//
// ///////////////////////////////
#define WSA_ERR_INVRFGAIN	(LNEG_NUM - 801)
#define WSA_ERR_INVIFGAIN	(LNEG_NUM - 802)

// ///////////////////////////////
// RUNMODE ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVRUNMODE	(LNEG_NUM - 1001)


// ///////////////////////////////
// TRIGGER ERRORS				//
// ///////////////////////////////
#define WSA_ERR_INVTRIGGERMODE	(LNEG_NUM - 1201)
#define WSA_ERR_INVTRIGGERDELAY (LNEG_NUM - 1202)
#define WSA_ERR_INVTRIGGERSYNC  (LNEG_NUM - 1203)

// ///////////////////////////////
// TIME RELATED ERRORS			//
// ///////////////////////////////
#define WSA_ERR_INVDWELL	(LNEG_NUM - 1301)

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
#define WSA_ERR_INVFILTERMODE (LNEG_NUM - 1602)
#define WSA_ERR_INVCALIBRATEMODE (LNEG_NUM - 1603)
#define WSA_ERR_INVRFESETTING (LNEG_NUM - 1604)
#define WSA_ERR_INVPLLREFSOURCE (LNEG_NUM - 1605)
#define WSA_ERR_INVATTEN	(LNEG_NUM - 1606)
#define WSA_ERR_INVRFEINPUTMODE (LNEG_NUM - 1607)

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
#define WSA_ERR_INVINPUT	(LNEG_NUM - 2004)

// ///////////////////////////////
// SWEEP ERRORS					//
// ///////////////////////////////
#define WSA_ERR_SWEEPSTARTFAIL	(LNEG_NUM - 3000)
#define WSA_ERR_SWEEPSTOPFAIL	(LNEG_NUM - 3001)
#define WSA_ERR_SWEEPRESUMEFAIL	(LNEG_NUM - 3002)
#define WSA_ERR_SWEEPSTATUSFAIL	(LNEG_NUM - 3003)
#define WSA_ERR_SWEEPSIZEFAIL	(LNEG_NUM - 3004)
#define WSA_ERR_SWEEPENTRYSAVEFAIL	(LNEG_NUM - 3005)
#define WSA_ERR_SWEEPENTRYCOPYFAIL	(LNEG_NUM - 3006)
#define WSA_ERR_SWEEPENTRYNEWFAIL	(LNEG_NUM - 3007)
#define WSA_ERR_SWEEPENTRYDELETEFAIL	(LNEG_NUM - 3008)
#define WSA_ERR_SWEEPALREADYRUNNING	(LNEG_NUM - 3009)
#define WSA_ERR_SWEEPLISTEMPTY	(LNEG_NUM - 3010)
#define WSA_ERR_SWEEPNOTRUNNING	(LNEG_NUM - 3011)
#define WSA_ERR_SWEEPIDOOB	(LNEG_NUM - 3012)
#define WSA_ERR_SWEEPMODEUNDEF (LNEG_NUM - 3013)
#define WSA_ERR_INVSWEEPSTARTID (LNEG_NUM - 3014)
#define WSA_ERR_SWEEPWHILESTREAMING (LNEG_NUM - 3015)

// ///////////////////////////////
// STREAM ERRORS  				//
// ///////////////////////////////
#define WSA_ERR_STREAMALREADYRUNNING	(LNEG_NUM - 4000)
#define WSA_ERR_STREAMNOTRUNNING     (LNEG_NUM - 4001)
#define WSA_ERR_STREAMWHILESWEEPING (LNEG_NUM - 4002)
#define WSA_ERR_INVSTREAMSTARTID	(LNEG_NUM - 4003)


// ///////////////////////////////
// WARNINGS						//
// ///////////////////////////////
#define WSA_WARNING_TRIGGER_CONFLICT	(WARNING_NUM - 0)



const char *_wsa_get_err_msg(int16_t err_id);

#endif
