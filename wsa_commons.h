#ifndef __WSA_COMMONS_H__
#define __WSA_COMMONS_H__


#include "stdint.h"

#define FALSE	0
#define TRUE	1

#define NUM_RF_GAINS 5	// including 0 but not use
#define MHZ 1000000

#define VRT_HEADER_SIZE 5
#define VRT_TRAILER_SIZE 1

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
#define WSA_RFE0440_ABS_AMP_MEDIUM 0
#define WSA_RFE0440_ABS_AMP_LOW 13
#define WSA_RFE0440_ABS_AMP_VLOW 20


// *****
// RFE0560 SPECIFIC
// *****
#define WSA_RFE0560 "RFE0560"
#define WSA_RFE0560_MAX_FREQ 7500000000
#define WSA_RFE0560_MIN_FREQ 10000000
#define WSA_RFE0560_MAX_IF_GAIN 0	// TODO Temp values for now
#define WSA_RFE0560_MIN_IF_GAIN -39
#define WSA_RFE0560_FREQRES	100000 // to read in the register

// TODO: TO BE DETERMINE W/ NIKHIL FOR THE FOLLOWING #S:
#define WSA_RFE0560_ABS_AMP_HIGH -15
#define WSA_RFE0560_ABS_AMP_MEDIUM 0
#define WSA_RFE0560_ABS_AMP_LOW 13
#define WSA_RFE0560_ABS_AMP_VLOW 20

#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"

int16_t wsa_tokenize_file(FILE *fptr, char *cmd_str[]);

#endif
