#ifndef __WSA_API_H__
#define __WSA_API_H__


#include "wsa_lib.h"


// ////////////////////////////////////////////////////////////////////////////
// CONSTANTS                                                                 //
// ////////////////////////////////////////////////////////////////////////////


#ifndef __WSA_LIB_H__

// ////////////////////////////////////////////////////////////////////////////
// TYPEDEFs                                                                  //
// ////////////////////////////////////////////////////////////////////////////

typedef enum wsa_gain {
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
	uint64_t max_sample_size;
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


// Temporary define for now. Will need to be expanded for more details...
struct wsa_frame_header {
	uint32_t sample_size; 
	struct wsa_time time_stamp;
};

struct wsa_socket {
	SOCKET cmd;
	SOCKET data;
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


#endif


// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_open(struct wsa_device *dev, char *intf_method);
void wsa_close(struct wsa_device *dev);

int16_t wsa_check_addr(char *intf_method);
int16_t wsa_is_connected(struct wsa_device *dev);
const char *wsa_get_err_msg(int16_t err_code);

int16_t wsa_set_command_file(struct wsa_device *dev, char *file_name);


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

float wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain);


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

int32_t wsa_read_frame_raw(struct wsa_device *dev, struct wsa_frame_header 
		*header, char *data_buf, const int32_t sample_size);
int32_t wsa_frame_decode(struct wsa_device *dev, char *data_buf, int16_t *i_buf,
						 int16_t *q_buf, const int32_t sample_size);
int32_t wsa_read_frame_int(struct wsa_device *dev, struct wsa_frame_header 
		*header, int16_t *i_buf, int16_t *q_buf, const int32_t sample_size);

int32_t wsa_get_sample_size(struct wsa_device *dev);
int16_t wsa_set_sample_size(struct wsa_device *dev, int32_t sample_size);

int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate);
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate);


// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int64_t wsa_get_freq (struct wsa_device *dev);
int16_t wsa_set_freq (struct wsa_device *dev, int64_t cfreq);


// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_gain_if (struct wsa_device *dev, int32_t *gain);
int16_t wsa_set_gain_if (struct wsa_device *dev, int32_t gain);

enum wsa_gain wsa_get_gain_rf (struct wsa_device *dev);
int16_t wsa_set_gain_rf (struct wsa_device *dev, enum wsa_gain gain);


// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_antenna(struct wsa_device *dev);
int16_t wsa_set_antenna(struct wsa_device *dev, int16_t port_num);

int16_t wsa_get_bpf(struct wsa_device *dev);
int16_t wsa_set_bpf(struct wsa_device *dev, int16_t mode);

int16_t wsa_query_cal_mode(struct wsa_device *dev);
int16_t wsa_run_cal_mode(struct wsa_device *dev, int16_t mode);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////

#endif

