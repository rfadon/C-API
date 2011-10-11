#ifndef __WSA_LIB_H__
#define __WSA_LIB_H__

#include "wsa_client.h"


#define FALSE	0
#define TRUE	1

#define NUM_RF_GAINS 5	// including 0 but not use

// Control commands syntax supported types
#define SCPI "SCPI"	/* SCPI control commands syntax */


typedef enum wsa_gain {
	WSA_GAIN_HIGH = 1,
	WSA_GAIN_MEDIUM,
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
	float max_if_gain;
	float min_if_gain;
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
	//struct wsa_trig *trig_list; ??? might not belong here ???
};


struct wsa_resp {
	int64_t status;
	char result[MAX_STR_LEN];
};


// ////////////////////////////////////////////////////////////////////////////
// List of functions                                                         //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method);
int16_t wsa_disconnect(struct wsa_device *dev);
int16_t wsa_list_devs(char **wsa_list);
//int16_t wsa_help(struct wsa_device dev);

int32_t wsa_send_command(struct wsa_device *dev, char *command);
int16_t wsa_send_command_file(struct wsa_device *dev, char *file_name);
struct wsa_resp wsa_send_query(struct wsa_device *dev, char *command);
//int16_t wsa_clear_query_resp(struct wsa_device *dev);
int32_t wsa_query_error(struct wsa_device *dev);
const char *wsa_get_error_msg(int16_t err_code);

int16_t wsa_get_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 char *data_buf, uint32_t sample_size);
int32_t wsa_decode_frame(char *data_buf, int16_t *i_buf, int16_t *q_buf, 
						 uint32_t sample_size);

#endif
