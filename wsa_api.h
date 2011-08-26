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

// TODO: add prefix to enum parameters, ex WSA_HIGH....
typedef enum wsa_gain {
	HIGH = 1,
	MEDIUM,
	LOW,
	ULOW
};


// ////////////////////////////////////////////////////////////////////////////
// STRUCTS DEFINES                                                           //
// ////////////////////////////////////////////////////////////////////////////


struct wsa_descriptor {
	char prod_name[50];
	char prod_serial[20];
	char prod_version[20];
	char rfe_name[20];
	char rfe_version[20];
	char fw_version[20];
	char intf_type[20];
	uint64_t inst_bw;
	uint64_t max_sample_size;
	uint64_t max_tune_freq;
	uint64_t min_tune_freq;
};


struct wsa_time {
	int32_t sec;
	uint32_t nsec;
};


// Temporary define for now. Will need to be expanded for more details...
struct wsa_frame_header {
	char prod_serial[20];  //???
	uint64_t freq;
	char gain[10];
	uint32_t frame_size; 
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


#endif


/*
struct wsa_trig 
{
	uint16_t id;	
	bool enable;	
	uint64_t center;
	uint64_t start;	
	uint64_t stop;	
	double amp;		
	uint32_t dwell_time;	
	uint32_t num_pkts;	
};
*/


// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_open(struct wsa_device *dev, char *intf_method);
void wsa_close(struct wsa_device *dev);
int16_t wsa_check_addr(char *intf_method);
int16_t wsa_list(char **wsa_list);
int16_t wsa_is_connected(struct wsa_device *dev);
/*int16_t wsa_set_dc_corr (wsa_device *dev, int8_t dc_corr);*/


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

float wsa_get_abs_max_amp(struct wsa_device *dev, wsa_gain gain);


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

int64_t wsa_read_pkt (struct wsa_device *dev, struct wsa_frame_header *header, 
			int16_t *i_buf, int16_t *q_buf, const uint64_t sample_size);
//int16_t wsa_set_iq_corr (struct wsa_device *dev, bool option);
//int16_t wsa_set_sample_size(struct wsa_device *dev, int64_t sample_size); //???
//int64_t wsa_get_sample_size(struct wsa_device *dev);


// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int64_t wsa_get_freq (struct wsa_device *dev);
int16_t wsa_set_freq (struct wsa_device *dev, uint64_t cfreq);


// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

wsa_gain wsa_get_gain (struct wsa_device *dev);
/*int16_t wsa_get_gain_cal (struct wsa_device *dev, wsa_gain gain, 
			uint64_t freq, double *cal_value);*/
int16_t wsa_set_gain (struct wsa_device *dev, wsa_gain gain);


// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_antenna(struct wsa_device *dev);
int16_t wsa_set_antenna(struct wsa_device *dev, uint8_t port_num);

int16_t wsa_get_bpf(struct wsa_device *dev);
int16_t wsa_set_bpf(struct wsa_device *dev, uint8_t mode);

int16_t wsa_get_lpf(struct wsa_device *dev);
int16_t wsa_set_lpf(struct wsa_device *dev, uint8_t option);

int16_t wsa_check_cal_mode(struct wsa_device *dev);
int16_t wsa_run_cal_mode(struct wsa_device *dev, uint8_t mode);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////
/*
int32_t wsa_add_trig (struct wsa_device *dev, struct wsa_trig *trigger);
int16_t wsa_clear_trig_list (struct wsa_device *dev);
int16_t wsa_disable_trig (struct wsa_device *dev, uint16_t id);
int16_t wsa_enable_trig (struct wsa_device *dev, uint16_t id);
int16_t wsa_get_trig (struct wsa_device *dev, uint16_t id, 
			struct wsa_trig *trig);
int32_t wsa_get_trig_list_size (struct wsa_device *dev);
int16_t wsa_remove_trig (struct wsa_device *dev, uint16_t id);
*/
#endif
