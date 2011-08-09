#ifndef __WSA4K_LIB_H__
#define __WSA4K_LIB_H__

#include "stdint.h"


//*****
//* List of structures
//*****

struct wsa_descriptor {
	char prod_name[50];
	char prod_serial[20];
	char prod_version[20];
	char rfe_name[20];
	char rfe_version[20];
	char fw_version[20];
	//uint64_t inst_bw;
	//uint32_t max_pkt_size;
	//uint64_t max_tune_freq;
	//uint64_t min_tune_freq;
};

/*
struct wsa_trig {
	uint16_t id;	
	bool enable;	
	uint64_t center;
	uint64_t start;	
	uint64_t stop;	
	double amp;		
	uint32_t dwell_time;	
	uint32_t num_pkts;	
};*/

/**
 * Temporary define for now. Will need to be expanded for more details...
 */
struct wsa_pkt_header {
	char prod_serial[20];  //???
	uint64_t freq;
	wsa_gain gain;
	//bool  iq_corr;
	uint32_t pkt_size; 
	wsa_time time_stamp;
	//uint16_t trig_id;
};


/*struct wsa_pkt {
	wsa_pkt_header header;
	int16_t *i_buf;
	int16_t *q_buf;
};*/


struct wsa_device {
	struct wsa_descriptor descr;
	//struct wsa_trig *trig_list; ??? might not belong here ???
};


struct wsa_resp {
	int16_t status;
	char *result;
}


//*****
//* List of functions
//*****
struct wsa_device wsa_connect(char *protocol, char *intf_method);
int16_t wsa_close(struct wsa_device dev);
void wsa_help(struct wsa_device dev);

int16_t wsa_send_command(struct wsa_device dev, char *command);
struct wsa_resp wsa_send_query(struct wsa_device dev, char *command);
int16_t wsa_query_error(struct wsa_device);
// this one need better design base on SCPI?
int32_t wsa_read (struct wsa_device *dev, wsa_pkt_header *header, 
				  int16_t *i_buf, int16_t *q_buf, const uint32_t pkt_size);


#endif