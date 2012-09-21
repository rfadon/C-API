#ifndef __WSA_LIB_H__
#define __WSA_LIB_H__

#include "wsa_commons.h"

#include <limits.h>


#define MAX_STR_LEN 512

#define NUM_RF_GAINS 5	// including 0 but not use
#define MHZ 1000000ULL

#define VRT_HEADER_SIZE 5

#define VRT_TRAILER_SIZE 1
#define BYTES_PER_VRT_WORD 4

#define MAX_PACKET_ORDER_INDICATOR 15


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
#define SCPI_QSR_POW 0x0002		// Battery power ?
#define SCPI_QSR_TEMP 0x0010	// Temperature ?
#define SCPI_QSR_FREQ 0x0020	// Frequency unlocked ?
#define SCPI_QSR_PHAS 0x0040	// IQ out of phase ?
#define SCPI_QSR_CALI 0x0100	//RFE Calibration ?


//*****
// WSA specific values
//*****
#define WSA4000 "WSA4000"
#define WSA4000_INST_BW 125000000ULL
#define WSA4000_MIN_SAMPLE_SIZE 128
#define WSA4000_MAX_SAMPLE_SIZE (2560 * 1024) // RANDOM NUMBER FOR NOW. CHECK VRT & DDR SIZE.

#define WSA4000_MIN_SAMPLES_PER_PACKET 128

// VRT header field for packet size is 16 bits,
// so maximum number that can be stored is 2^16 - 1
// and also need to allow room for VRT header and trailer bytes
#define WSA4000_MAX_SAMPLES_PER_PACKET (65536 - 1 - VRT_HEADER_SIZE - VRT_TRAILER_SIZE)
#define WSA4000_MIN_PACKETS_PER_BLOCK 1
#define WSA4000_MAX_PACKETS_PER_BLOCK INT_MAX

// PLL Values
#define PLL_ENT 1
#define PLL_EXT 2

// *****
// RFE0440 SPECIFIC
// *****
#define WSA_RFE0440 "RFE0440"
#define WSA_RFE0440_MAX_FREQ 4000000000ULL
#define WSA_RFE0440_MIN_FREQ 200000000ULL
#define WSA_RFE0440_FREQRES	10000ULL
#define WSA_RFE0440_ABS_AMP_HIGH -15
#define WSA_RFE0440_ABS_AMP_MED 0
#define WSA_RFE0440_ABS_AMP_LOW 13
#define WSA_RFE0440_ABS_AMP_VLOW 20


// *****
// RFE0560 SPECIFIC
// *****
#define WSA_RFE0560 "RFE0560"
#define WSA_RFE0560_MAX_FREQ 10000ULL // MHz here b/c of large # issue
#define WSA_RFE0560_MIN_FREQ 0ULL  // Hz
#define WSA_RFE0560_MAX_IF_GAIN 34
#define WSA_RFE0560_MIN_IF_GAIN -10
#define WSA_RFE0560_MAX_DECIMATION 1023
#define WSA_RFE0560_MIN_DECIMATION 4
#define WSA_RFE0560_FREQRES	100000ULL // to read in the register
#define WSA_RFE0560_MAX_ANT_PORT 2


// TODO: TO BE DETERMINE W/ NIKHIL FOR THE FOLLOWING #S -> Read from eeprom
#define WSA_RFE0560_ABS_AMP_HIGH 0
#define WSA_RFE0560_ABS_AMP_MED 15
#define WSA_RFE0560_ABS_AMP_LOW 15
#define WSA_RFE0560_ABS_AMP_VLOW 15

// Control commands syntax supported types
#define SCPI "SCPI"	/* SCPI control commands syntax */


enum wsa_gain {
	WSA_GAIN_HIGH = 1,
	WSA_GAIN_MED,
	WSA_GAIN_LOW,
	WSA_GAIN_VLOW
};

// ////////////////////////////////////////////////////////////////////////////
// STRUCTS DEFINES                                                           //
// ////////////////////////////////////////////////////////////////////////////

struct wsa_descriptor {
	char prod_name[50];
	char prod_serial[20];
	char prod_version[20];
	char rfe_name[50];
	char rfe_version[20];
	char fw_version[20];
	char intf_type[20];
	uint64_t inst_bw;
	uint32_t max_sample_size;
	uint64_t max_tune_freq;
	uint64_t min_tune_freq;
	uint64_t freq_resolution;
	int32_t max_if_gain;
	int32_t min_if_gain;
	int32_t min_decimation;
	int32_t max_decimation;
	float abs_max_amp[NUM_RF_GAINS];
};


struct wsa_time {
	uint32_t sec;
	uint64_t psec;
};


struct wsa_vrt_packet_header {
	uint8_t packet_order_indicator;
	uint32_t samples_per_packet;
	struct wsa_time time_stamp;
};



//structure to hold reciever packet data
struct wsa_reciever_packet {
	int32_t indicator_field;
	int32_t reference_point;
	long double frequency;
	double gain_if;
	double gain_rf;
	double temperature;
};

//structure to hold digitizer packet data
struct wsa_digitizer_packet {
	int32_t indicator_field;
	long double bandwidth;
	double reference_level;
	long double rf_frequency_offset;
};

// These values will be defined in a future release
struct wsa_vrt_packet_trailer {
	uint8_t valid_data_indicator;
	uint8_t ref_lock_indicator;
	uint8_t over_range_indicator;
	uint8_t sample_loss_indicator;
};

struct wsa_socket {
	int32_t cmd;
	int32_t data;
};


struct wsa_device {
	struct wsa_descriptor descr;
	struct wsa_socket sock;
	//struct wsa_trig *trig_list; 
};


struct wsa_resp {
	int64_t status;
	char output[MAX_STR_LEN];
};


// ////////////////////////////////////////////////////////////////////////////
// List of functions                                                         //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method);
int16_t wsa_disconnect(struct wsa_device *dev);

int16_t wsa_verify_addr(const char *sock_addr, const char *sock_port);

int16_t wsa_send_command(struct wsa_device *dev, char *command);
int16_t wsa_send_command_file(struct wsa_device *dev, char *file_name);
int16_t wsa_send_query(struct wsa_device *dev, char *command, 
						struct wsa_resp *resp);
int16_t wsa_read_iq_packet_raw(struct wsa_device* const device, 
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		struct wsa_reciever_packet* const reciever,
		struct wsa_digitizer_packet* const digitizer,
		uint8_t* const data_buffer,	const uint16_t samples_per_packet,
		uint8_t* context_present);

int32_t wsa_decode_frame(uint8_t* data_buf, int16_t *i_buf, int16_t *q_buf, 
						 uint32_t sample_size);

int16_t wsa_read_status(struct wsa_device *dev, char *output);
		

int16_t wsa_read_iq_packet_raw_matlab(struct wsa_device* const device, 
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		uint8_t* const data_buffer, 
		const uint16_t samples_per_packet, uint8_t* context_present,
	int32_t* rec_indicator_field, int32_t* rec_reference_point, int64_t* rec_frequency, int16_t* rec_gain_if, int16_t* rec_gain_rf,	int32_t* rec_temperature,
	int32_t* dig_indicator_field, long double* dig_bandwidth, double* dig_reference_level, long double* digrf_frequency_offset);		
			
int32_t wsa_decode_frame(uint8_t* data_buf, int16_t *i_buf, int16_t *q_buf, 
						 uint32_t sample_size);
int16_t wsa_read_status(struct wsa_device *dev, char *output);
int16_t extract_reciever_frequency( uint8_t* temp_buffer, int64_t* reciever_frequency); 


const char *wsa_get_error_msg(int16_t err_code);

#endif
