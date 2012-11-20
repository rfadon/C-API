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

int16_t wsa_system_request_acquisition_access(struct wsa_device *dev, int16_t* status);
int16_t wsa_system_read_status(struct wsa_device *dev, int16_t* status); 

// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
						  float *value);


// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_capture_block(struct wsa_device* const device);

int16_t wsa_read_vrt_packet (struct wsa_device* const device, 
		struct wsa_vrt_packet_header* const header, 
		struct wsa_vrt_packet_trailer* const trailer,
		struct wsa_receiver_packet* const receiver,
		struct wsa_digitizer_packet* const digitizer,
		int16_t* const i_buffer,
		int16_t* const q_buffer,
		int32_t samples_per_packet);

int16_t wsa_get_samples_per_packet(struct wsa_device* device, int32_t* samples_per_packet);
int16_t wsa_set_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

int16_t wsa_get_packets_per_block(struct wsa_device* device, int32_t* packets_per_block);
int16_t wsa_set_packets_per_block(struct wsa_device *dev, int32_t packets_per_block);

int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate);
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate);
    
int16_t wsa_flush_data(struct wsa_device *dev);

int16_t wsa_system_abort_capture(struct wsa_device *dev);

// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_freq(struct wsa_device *dev, int64_t *cfreq);
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq);

int16_t wsa_get_freq_shift(struct wsa_device *dev, float *fshift);
int16_t wsa_set_freq_shift(struct wsa_device *dev, float fshift);


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
// DEVICE VERSION SECTION                                                    //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_fw_ver(struct wsa_device *dev, char* fw_ver);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_set_trigger_level(struct wsa_device* dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);
int16_t wsa_get_trigger_level(struct wsa_device* dev, int64_t* start_freq, int64_t* stop_freq, int32_t* amplitude);

int16_t wsa_set_trigger_enable(struct wsa_device* dev, int32_t enable);
int16_t wsa_get_trigger_enable(struct wsa_device* dev, int32_t* enable);

int16_t wsa_get_trigger_type(struct wsa_device *dev, char *type);
int16_t wsa_set_trigger_type(struct wsa_device *dev, char *type);
   
// ////////////////////////////////////////////////////////////////////////////
// PLL Section                                                               //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_reference_pll(struct wsa_device* dev, char* pll_ref);
int16_t wsa_set_reference_pll(struct wsa_device* dev, char* pll_ref);

int16_t wsa_reset_reference_pll(struct wsa_device* dev);
int16_t wsa_get_lock_ref_pll(struct wsa_device* dev, int32_t* lock_ref);

int16_t wsa_get_lock_rf(struct wsa_device* dev, int32_t* lock_rf);

// ////////////////////////////////////////////////////////////////////////////
// SWEEP FUNCTIONS                                                           //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_get_sweep_antenna(struct wsa_device *dev, int32_t *port_num);
int16_t wsa_set_sweep_antenna(struct wsa_device *dev, int32_t port_num);

int16_t wsa_get_sweep_gain_if(struct wsa_device *dev, int32_t *gain);
int16_t wsa_set_sweep_gain_if(struct wsa_device *dev, int32_t gain);

int16_t wsa_get_sweep_gain_rf(struct wsa_device *dev, enum wsa_gain *gain);
int16_t wsa_set_sweep_gain_rf(struct wsa_device *dev, enum wsa_gain gain);
		
int16_t wsa_get_sweep_samples_per_packet(struct wsa_device* device, int32_t* samples_per_packet);
int16_t wsa_set_sweep_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

int16_t wsa_get_sweep_packets_per_block(struct wsa_device* device, uint32_t* packets_per_block);
int16_t wsa_set_sweep_packets_per_block(struct wsa_device *dev, uint32_t packets_per_block);

int16_t wsa_get_sweep_decimation(struct wsa_device *dev, int32_t* rate);
int16_t	wsa_set_sweep_decimation(struct wsa_device *dev, int32_t rate);
	
int16_t wsa_get_sweep_freq(struct wsa_device *dev, int64_t *freq_start, int64_t *freq_stop);
int16_t wsa_set_sweep_freq(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq);

int16_t wsa_get_sweep_freq_shift(struct wsa_device *dev, float *fshift);
int16_t wsa_set_sweep_freq_shift(struct wsa_device *dev, float fshift);

int16_t wsa_get_sweep_freq_step(struct wsa_device *dev, int64_t *step);
int16_t wsa_set_sweep_freq_step(struct wsa_device *dev, int64_t step);

int16_t wsa_get_sweep_dwell(struct wsa_device *dev, int32_t *seconds, int32_t *microseconds);
int16_t wsa_set_sweep_dwell(struct wsa_device *dev,int32_t seconds, int32_t microseconds);

int16_t wsa_get_sweep_trigger_type(struct wsa_device *dev, char *type);
int16_t wsa_set_sweep_trigger_type(struct wsa_device *dev, char *type);

int16_t wsa_set_sweep_trigger_level(struct wsa_device* dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);
int16_t wsa_get_sweep_trigger_level(struct wsa_device* dev, int64_t* start_freq, int64_t* stop_freq, int32_t* amplitude);

int16_t wsa_get_sweep_iteration(struct wsa_device *dev, int32_t *iteration);
int16_t wsa_set_sweep_iteration(struct wsa_device *dev, int32_t iteration);

int16_t wsa_get_sweep_status(struct wsa_device *dev, char *status);
int16_t wsa_sweep_entry_new(struct wsa_device *dev);
int16_t wsa_sweep_entry_save(struct wsa_device *dev, int32_t positon);

int16_t wsa_sweep_entry_copy(struct wsa_device *dev, int32_t id);
int16_t wsa_sweep_entry_delete(struct wsa_device *dev, int32_t id);
int16_t wsa_sweep_entry_delete_all(struct wsa_device *dev);
int16_t wsa_sweep_entry_read(struct wsa_device *dev, int32_t id, struct wsa_sweep_list* const sweep_list);
int16_t wsa_sweep_start(struct wsa_device *dev);
int16_t wsa_sweep_stop(struct wsa_device *dev);
int16_t wsa_sweep_resume(struct wsa_device *dev);
int16_t wsa_get_sweep_entry_size(struct wsa_device* device, int32_t *size);

#endif




