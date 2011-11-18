#ifndef __WSA_COMMONS_H__
#define __WSA_COMMONS_H__


#include "stdint.h"

#define FALSE	0
#define TRUE	1

#define NUM_RF_GAINS 5	// including 0 but not use
#define MHZ 1000000

#define VRT_HEADER_SIZE 5
#define VRT_TRAILER_SIZE 1


// *****
// SCPI related registers/bits
// *****

// Status Byte Register SBR, use with SRE
#define SCPI_SBR_EVTAVL 0x04	// Error/event queue available
#define SCPI_SBR_QSR 0x08		// Questionable Status register
#define SCPI_SBR_MSGAVL 0x10	// Message available
#define SCPI_SBR_ESR 0x20		// Event Status register
#define SCPI_SBR_RQS 0x40		// Request Service register
#define SCPI_SBR_OSR 0x80		// Operational Status register


// Standard Event Status Register ESR, use with ESE
#define SCPI_ESR_OPC 0x00	// Operation complete
#define SCPI_ESR_QYE 0x04	// Query error
#define SCPI_ESR_DDE 0x08	// Device dependent error
#define SCPI_ESR_EXE 0x10	// Execution error
#define SCPI_ESR_CME 0x20	// Command error
#define SCPI_ESR_PON 0x80	// Power ON


// Operation Status Register OSR
#define SCPI_OSR_CALI 0x0001	// RFE Calibrating
#define SCPI_OSR_SETT 0x0002	// Settling
#define SCPI_OSR_SWE  0x0008	// Sweeping
//#define SCPI_OSR_MEAS 0x0010	// Measuring?
#define SCPI_OSR_TRIG 0x0020	// Triggering
#define SCPI_OSR_CORR 0x0080	//Correcting which? IQ or DC ?


// Questionable Status Register QSR
#define SCPI_QSR_POW 0x0002	// Battery power ???
#define SCPI_QSR_TEMP 0x0010	// Temperature ?
#define SCPI_QSR_FREQ 0x0020	// Frequency unlocked ?
#define SCPI_QSR_PHAS 0x0040	// IQ out of phase ?
#define SCPI_QSR_CALI 0x0100	//RFE Calibration ?


//*****
// WSA specific values
//*****
#define WSA4000 "WSA4000"
#define WSA4000_INST_BW 125000000
#define WSA4000_MAX_SAMPLE_SIZE 32768 // RANDOM NUMBER FOR NOW. CHECK VRT & DDR SIZE.


// *****
// RFE0440 SPECIFIC
// *****
#define WSA_RFE0440 "RFE0440"
#define WSA_RFE0440_MAX_FREQ 4000000000
#define WSA_RFE0440_MIN_FREQ 200000000
#define WSA_RFE0440_FREQRES	10000
#define WSA_RFE0440_ABS_AMP_HIGH -15
#define WSA_RFE0440_ABS_AMP_MED 0
#define WSA_RFE0440_ABS_AMP_LOW 13
#define WSA_RFE0440_ABS_AMP_VLOW 20


// *****
// RFE0560 SPECIFIC
// *****
#define WSA_RFE0560 "RFE0560"
#define WSA_RFE0560_MAX_FREQ 7500000000
#define WSA_RFE0560_MIN_FREQ 10000000
#define WSA_RFE0560_MAX_IF_GAIN 34
#define WSA_RFE0560_MIN_IF_GAIN -10
#define WSA_RFE0560_FREQRES	100000 // to read in the register

// TODO: TO BE DETERMINE W/ NIKHIL FOR THE FOLLOWING #S:
#define WSA_RFE0560_ABS_AMP_HIGH -15
#define WSA_RFE0560_ABS_AMP_MED 0
#define WSA_RFE0560_ABS_AMP_LOW 13
#define WSA_RFE0560_ABS_AMP_VLOW 20

#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"

int16_t wsa_tokenize_file(FILE *fptr, char *cmd_str[]);
int16_t to_int(char *num_str, long int *val);
int16_t to_double(char *num_str, double *val);

#endif
