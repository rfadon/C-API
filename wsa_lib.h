#ifndef __WSA_LIB_H__
#define __WSA_LIB_H__

#include "stdint.h"
#include "wsa_client.h"


#define FALSE	0
#define TRUE	1

// Control commands syntax supported types
#define SCPI "SCPI"	/* SCPI control commands syntax */


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
	uint64_t inst_bw;
	uint64_t max_pkt_size;
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


struct wsa_resp {
	int32_t status;
	char *result;
};


// ////////////////////////////////////////////////////////////////////////////
// List of functions                                                         //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_connect(struct wsa_device *dev, char *cmd_syntax, 
					char *intf_method);
int16_t wsa_disconnect(struct wsa_device *dev);
int16_t wsa_list_devs(char **wsa_list);
//int16_t wsa_help(struct wsa_device dev);
//int16_t wsa_send_command_file(struct wsa_device *dev, char *file_name);

int16_t wsa_send_command(struct wsa_device *dev, char *command);
struct wsa_resp wsa_send_query(struct wsa_device *dev, char *command);
int16_t wsa_query_error(struct wsa_device *dev);
int64_t wsa_get_frame(struct wsa_device *dev, struct wsa_frame_header *header, 
				 int32_t *i_buf, int32_t *q_buf, uint64_t frame_size);

#endif