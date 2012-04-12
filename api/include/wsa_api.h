#ifndef __WSA_API_H__
#define __WSA_API_H__


#include "wsa_lib.h"


// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_open(struct wsa_device *dev, char *intf_method);
void wsa_close(struct wsa_device *dev);

int16_t wsa_check_addr(char *ip_addr);
int16_t wsa_check_addrandport(char *ip_addr, char *port);
int16_t wsa_is_connected(struct wsa_device *dev);
const char *wsa_get_err_msg(int16_t err_code);

int16_t wsa_set_command_file(struct wsa_device *dev, char *file_name);


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
						  float *value);


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

int32_t wsa_read_frame_raw(struct wsa_device *dev, struct wsa_frame_header 
		*header, uint8_t* data_buf, const int32_t sample_size);
int32_t wsa_frame_decode(struct wsa_device *dev, uint8_t* data_buf, int16_t *i_buf,
						 int16_t *q_buf, const int32_t sample_size);
int32_t wsa_read_frame_int(struct wsa_device *dev, struct wsa_frame_header 
		*header, int16_t *i_buf, int16_t *q_buf, const int32_t sample_size);

int16_t wsa_capture_block(struct wsa_device* const device);
int16_t wsa_read_iq_packet (struct wsa_device* const device, 
		struct wsa_frame_header* const header, 
		int16_t* const i_buffer, 
		int16_t* const q_buffer,
		const uint16_t samples_per_packet);

int16_t wsa_get_sample_size(struct wsa_device *dev, int32_t *sample_size);
int16_t wsa_set_sample_size(struct wsa_device *dev, int32_t sample_size);

int16_t wsa_get_samples_per_packet(struct wsa_device* device, uint16_t* samples_per_packet);
int16_t wsa_set_samples_per_packet(struct wsa_device *dev, uint16_t samples_per_packet);
int16_t wsa_set_packets_per_block(struct wsa_device *dev, uint32_t packets_per_block);

int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate);
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate);


// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_freq(struct wsa_device *dev, int64_t *cfreq);
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq);


// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_gain_if(struct wsa_device *dev, int32_t *gain);
int16_t wsa_set_gain_if(struct wsa_device *dev, int32_t gain);

int16_t wsa_get_gain_rf(struct wsa_device *dev, enum wsa_gain *gain);
int16_t wsa_set_gain_rf(struct wsa_device *dev, enum wsa_gain gain);


// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_antenna(struct wsa_device *dev, int32_t *port_num);
int16_t wsa_set_antenna(struct wsa_device *dev, int32_t port_num);

int16_t wsa_get_bpf_mode(struct wsa_device *dev, int32_t *mode);
int16_t wsa_set_bpf_mode(struct wsa_device *dev, int32_t mode);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////

#endif

