#ifndef __WSA_LIB_H__
#define __WSA_LIB_H__

#include "wsa_commons.h"

#include <limits.h>
#include <math.h>

#define MAX_STR_LEN 512

#define NUM_RF_GAINS 5	// including 0 but not use
#define MHZ 1000000ULL


// *****
// VRT PACKET related
// *****
#define VRT_HEADER_SIZE 5
#define VRT_TRAILER_SIZE 1
#define BYTES_PER_VRT_WORD 4

#define MAX_VRT_PKT_COUNT 15
#define MIN_VRT_PKT_COUNT 0

#define MAX_BLOCK_SIZE 70000000
// VRT packet stream indentifier
#define RECEIVER_STREAM_ID 0x90000001
#define DIGITIZER_STREAM_ID 0x90000002
#define I16Q16_DATA_STREAM_ID 0x90000003
#define I16_DATA_STREAM_ID 0x90000005
#define I32_DATA_STREAM_ID 0x90000006
#define EXTENSION_STREAM_ID 0x90000004

// Packet types
#define IF_PACKET_TYPE 1
#define CONTEXT_PACKET_TYPE 4
#define EXTENSION_PACKET_TYPE 5

// Receiver context data field indicator masks
#define REF_POINT_INDICATOR_MASK 0x40000000
#define FREQ_INDICATOR_MASK 0x08000000
#define GAIN_INDICATOR_MASK 0x00800000

// Digitizer context data field indicator masks
#define BW_INDICATOR_MASK 0xa0000000
#define RF_FREQ_OFFSET_INDICATOR_MASK 0x04000000
#define REF_LEVEL_INDICATOR_MASK 0x01000000

// extension packet data field indicator masks
#define SWEEP_START_ID_INDICATOR_MASK 0x00000001
#define STREAM_START_ID_INDICATOR_MASK 0x00000002

// *****
// Attenuation Control Types
// *****
#define WSA5000_408_ATTEN 0
#define WSA5000_427_ATTEN 1
#define R5500_408_ATTEN 2
#define R5500_427 3

// *****
// SCPI related registers/bits
// *****

// Control commands syntax supported types
#define SCPI "SCPI"	/* SCPI control commands syntax */
#define SCPI_QUERY_CMD "?"
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
#define SCPI_OSR_CORR 0x0080	// Correcting which? IQ or DC ?

// Questionable Status Register QSR
#define SCPI_QSR_POW 0x0002		// Battery power ?
#define SCPI_QSR_TEMP 0x0010	// Temperature ?
#define SCPI_QSR_FREQ 0x0020	// Frequency unlocked ?
#define SCPI_QSR_PHAS 0x0040	// IQ out of phase ?
#define SCPI_QSR_CALI 0x0100	// RFE Calibration ?


//*****
// WSA specific values
//*****

#define WSA_CONNECT_TIMEOUT 5000
#define WSA_PING_TIMEOUT 1

#define WSA_IBW 125000000ULL
#define WSA_MAX_CAPTURE_BLOCK (32 * 1024 * 1024)

// VRT header field for packet size is 16 bits,
// so maximum number that can be stored is 2^16 - 1
// and also need to allow room for VRT header and trailer bytes
#define WSA_MAX_SPP 64000
#define WSA_SPP_MULTIPLE 32
#define WSA_MIN_SPP 256
#define WSA_MIN_PPB 1
#define WSA_MAX_PPB UINT_MAX

// sweep states
#define WSA_SWEEP_STATE_RUNNING "RUNNING"
#define WSA_SWEEP_STATE_STOPPED "STOPPED"

// capture modes
#define WSA_BLOCK_CAPTURE_MODE "BLOCK"
#define WSA_STREAM_CAPTURE_MODE "STREAMING"
#define WSA_SWEEP_CAPTURE_MODE "SWEEPING"

// trigger types
#define WSA_NONE_TRIGGER_TYPE "NONE"
#define WSA_LEVEL_TRIGGER_TYPE "LEVEL"
#define WSA_PULSE_TRIGGER_TYPE "PULSE"

// trigger synchronization options
#define WSA_MASTER_TRIGGER "MASTER"
#define WSA_SLAVE_TRIGGER "SLAVE"

// delay limitations
#define WSA_trigger_SYNC_DELAY_MAX      0x7ffffff8
#define WSA_trigger_SYNC_DELAY_MIN      0
#define WSA_trigger_SYNC_DELAY_MULTIPLE 8

// Decimation Range
#define WSA_MAX_DECIMATION 1024
#define WSA_MIN_DECIMATION 4

// Offset of KISS FFT
#define KISS_FFT_OFFSET 2.64;

// R5500 SPECIFIC
#define R5500 "R5500"
// *****
// RTSA7500 SPECIFIC
// *****
#define RTSA7500 "RTSA7500"
#define RTSA75008 "RTSA7500-8"
#define RTSA75008P "RTSA7500-8P"
#define RTSA750018 "RTSA7500-18"
#define RTSA750027 "RTSA7500-27"


// *****
// WSA5000 SPECIFIC
// *****
#define WSA5000 "WSA5000"
#define WSA5000108 "108"
#define WSA5000208 "208"
#define WSA5000220 "220"
#define WSA5000308 "308"
#define WSA5000408 "408"
#define WSA5000408P "408P"
#define WSA5000418 "418"
#define WSA5000427 "427"
#define WSA_5000108_MAX_FREQ 8000ULL 
#define WSA_5000208_MAX_FREQ 8000ULL 
#define WSA_5000220_MAX_FREQ 20000ULL 
#define WSA_5000408_MAX_FREQ 8000ULL
#define WSA_5000418_MAX_FREQ 18000ULL
#define WSA_5000427_MAX_FREQ 27000ULL
#define WSA_5000_MIN_FREQ 0ULL  // Hz
#define WSA_5000_MAX_DECIMATION 1023
#define WSA_5000_MIN_DECIMATION 4
#define WSA_5000_FREQRES 100000ULL // to read in the register

// RFE modes
#define WSA_RFE_DD_STRING   "DD"
#define WSA_RFE_IQIN_STRING "IQIN"
#define WSA_RFE_HDR_STRING  "HDR"
#define WSA_RFE_SH_STRING   "SH"
#define WSA_RFE_SHN_STRING  "SHN"
#define WSA_RFE_ZIF_STRING  "ZIF"

#define  WSA_CURRENT_LAN_CONFIG "CURRENT"
#define  WSA_OPTION_LAN_CONFIG ""

// IQ Output mode
#define WSA_IQ_DIGITIZER_STRING "DIGITIZER"
#define WSA_IQ_CONNECTOR_STRING "CONNECTOR"

#define WSA_ATTEN_ENABLED 1
#define WSA_ATTEN_DISABLED 0

// ////////////////////////////////////////////////////////////////////////////
// STRUCTS DEFINES                                                           //
// ////////////////////////////////////////////////////////////////////////////

struct wsa_descriptor {
	char prod_model[MAX_STR_LEN];
	char dev_model[MAX_STR_LEN];
	char mac_addr[MAX_STR_LEN];
	char fw_version[MAX_STR_LEN];
	char intf_type[MAX_STR_LEN];
	char ATTENUATION_CONTROL[MAX_STR_LEN];
	uint64_t inst_bw;
	int32_t max_sample_size;
	int64_t max_tune_freq;
	int64_t min_tune_freq;
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

//structure to hold the header of a packet
struct wsa_vrt_packet_header {
	uint8_t pkt_count;
	uint16_t samples_per_packet;
	uint8_t packet_type;
	uint32_t stream_id;
	struct wsa_time time_stamp;
};

//structure to hold receiver packet data
struct wsa_receiver_packet {
	int32_t indicator_field;
	uint8_t pkt_count;
	int32_t reference_point;
	long double freq;
	double gain_if;
	double gain_rf;
	double temperature;
};

//structure to hold digitizer packet data
struct wsa_digitizer_packet {
	int32_t indicator_field;
	uint8_t pkt_count;
	long double bandwidth;
	int16_t reference_level;
	long double rf_freq_offset;
};

//structure to hold extension packet data
struct wsa_extension_packet {
	int32_t indicator_field;
	uint8_t pkt_count;
	uint32_t sweep_start_id;
	uint32_t stream_start_id;
};

// These values will be defined in a future release
struct wsa_vrt_packet_trailer {
	uint8_t valid_data_indicator;
	uint8_t ref_lock_indicator;
	uint8_t spectral_inversion_indicator;
	uint8_t over_range_indicator;
	uint8_t sample_loss_indicator;
};

// Structure to hold sweep list data
struct wsa_sweep_list {
	char rfe_mode[MAX_STR_LEN];
	int64_t start_freq;
	int64_t stop_freq;
	float fshift;
	int64_t fstep;
	int32_t decimation_rate;
	int32_t ant_port;
	int32_t attenuator;
	int32_t gain_hdr;
	int32_t gain_if;
	int32_t dwell_seconds;
	int32_t dwell_microseconds;
	int32_t samples_per_packet;
	int32_t packets_per_block;
	char trigger_type[MAX_STR_LEN];
	int64_t trigger_start_freq;
	int64_t trigger_stop_freq;
	int32_t trigger_amplitude;
	char trigger_sync_state[MAX_STR_LEN];
	int32_t trigger_sync_delay;
	char gain_rf[MAX_STR_LEN];
};
 
struct wsa_socket {
	int32_t cmd;
	int32_t data;
};

struct wsa_device {
	struct wsa_descriptor descr;
	struct wsa_socket sock;
};

struct wsa_resp {
	int64_t status;
	char output[MAX_STR_LEN];
};


// ////////////////////////////////////////////////////////////////////////////
// List of functions                                                         //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_connect(struct wsa_device *dev, char const *cmd_syntax, char *intf_method, int16_t timeout);
int16_t wsa_disconnect(struct wsa_device *dev);
int16_t wsa_verify_addr(const char *sock_addr, const char *sock_port);

int16_t wsa_send_command(struct wsa_device *dev, char const *command);
int16_t wsa_send_command_file(struct wsa_device *dev, char const *file_name);
int16_t wsa_send_query(struct wsa_device *dev, char const *command, struct wsa_resp *resp);

int16_t wsa_read_vrt_packet_raw(struct wsa_device * const device, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet * const digitizer,
		struct wsa_extension_packet * const extension,
		uint8_t * const data_buffer,
		uint32_t timeout);
		
int32_t wsa_decode_zif_frame(uint8_t *data_buf, int16_t *i_buf, int16_t *q_buf, 
						 int32_t sample_size);

int32_t wsa_decode_i_only_frame(uint32_t stream_id, uint8_t *data_buf, int16_t *i16_buf, int32_t *i32_buf, int32_t sample_size);

int16_t wsa_read_status(struct wsa_device *dev, char *output);

const char *wsa_get_error_msg(int16_t err_code);

#endif
