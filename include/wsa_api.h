#ifndef __WSA_API_H__
#define __WSA_API_H__


#include "wsa_lib.h"


// ////////////////////////////////////////////////////////////////////////////
// DEBUG FUNCTIONS                                                           //
// ////////////////////////////////////////////////////////////////////////////

void wsa_debuglevel(int);
void wsa_debugcallback(void(*callback)(void * pvoid, char const * pstring), void * pvoid);



// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_open(struct wsa_device *dev, char *intf_method);
int16_t wsa_ping(struct wsa_device *dev, char *intf_method);
void wsa_close(struct wsa_device *dev);

int16_t wsa_check_addr(char const *ip_addr);
int16_t wsa_check_addrandport(char const *ip_addr, char const *port);
int16_t wsa_is_connected(struct wsa_device *dev);
const char *wsa_get_err_msg(int16_t err_code);

int16_t wsa_do_scpi_command_file(struct wsa_device *dev, char const *file_name);

int16_t wsa_query_scpi(struct wsa_device *dev, char const *command, char *response);
int16_t wsa_send_scpi(struct wsa_device *dev, char const *command);


// ////////////////////////////////////////////////////////////////////////////
// PROBE SECTION                                                             //
// ////////////////////////////////////////////////////////////////////////////

void * wsa_probe_begin(void);
int    wsa_probe_poll(void * handle, int timeout, char * probe, char * device, char * serial, char * version, int stringsize);
void   wsa_probe_end(void * handle);


// ////////////////////////////////////////////////////////////////////////////
// LAN CONFIGURATION SECTION                                                 //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_lan_config(struct wsa_device *dev, char const *config, char *lan_config);
int16_t wsa_set_lan_config(struct wsa_device *dev, char const *lan_config);

int16_t wsa_get_lan_ip(struct wsa_device *dev, char const *config, char *ip);
int16_t wsa_set_lan_ip(struct wsa_device *dev, char const *ip);

int16_t wsa_get_lan_netmask(struct wsa_device *dev, char const *config, char *netmask);
int16_t wsa_set_lan_netmask(struct wsa_device *dev, char const *netmask);

int16_t wsa_get_lan_gateway(struct wsa_device *dev, char const *config, char *gateway);
int16_t wsa_set_lan_gateway(struct wsa_device *dev, char const *gateway);

int16_t wsa_get_lan_dns(struct wsa_device *dev, char const *config, char *dns);
int16_t wsa_set_lan_dns(struct wsa_device *dev, char const *dns, char const *alternate_dns);

int16_t wsa_apply_lan_config(struct wsa_device *dev);


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
						  float *value);

// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_system_request_acq_access(struct wsa_device *dev, int16_t *status);
int16_t wsa_system_acq_status(struct wsa_device *dev, int16_t *status);

int16_t wsa_get_capture_mode(struct wsa_device * const dev, char *mode);
int16_t wsa_system_abort_capture(struct wsa_device *dev);

int16_t wsa_flush_data(struct wsa_device *dev);
int16_t wsa_clean_data_socket(struct wsa_device *dev);

int16_t wsa_capture_block(struct wsa_device * const dev);

int16_t wsa_read_vrt_packet (struct wsa_device * const dev, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet * const digitizer,
		struct wsa_extension_packet * const sweep,
		int16_t * const i16_buffer,
		int16_t * const q16_buffer,
		int32_t * const i32_buffer,
		int32_t samples_per_packet,
		uint32_t timeout);

int16_t wsa_get_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet);
int16_t wsa_set_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

int16_t wsa_get_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block);
int16_t wsa_set_packets_per_block(struct wsa_device *dev, int32_t packets_per_block);

int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate);
int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate);

// ////////////////////////////////////////////////////////////////////////////
// DSP Section                                                               //
// ////////////////////////////////////////////////////////////////////////////
int16_t wsa_get_fft_size(int32_t const samples_per_packet, int32_t const stream_id, int32_t *array_size);
int16_t wsa_compute_fft(int32_t const samples_per_packet,
						int32_t const stream_id,
						int16_t const reference_level,
						uint8_t const spectral_inversion,
						int16_t * const i16_buffer,
						int16_t * const q16_buffer,
						int32_t * const i32_buffer,
						float * fft_buffer
						);


// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_freq(struct wsa_device *dev, int64_t *cfreq);
int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq);

int16_t wsa_get_freq_shift(struct wsa_device *dev, float *fshift);
int16_t wsa_set_freq_shift(struct wsa_device *dev, float fshift);

int16_t wsa_get_spec_inv(struct wsa_device *dev, int64_t freq, int16_t *inv);

// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_attenuation(struct wsa_device *dev, int32_t *mode);
int16_t wsa_set_attenuation(struct wsa_device *dev, int32_t mode);

int16_t wsa_get_gain_if(struct wsa_device *dev, int32_t *gain);
int16_t wsa_set_gain_if(struct wsa_device *dev, int32_t gain);

int16_t wsa_get_gain_rf(struct wsa_device *dev, char *gain);
int16_t wsa_set_gain_rf(struct wsa_device *dev, char const *gain);

// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_rfe_input_mode(struct wsa_device *dev, char *mode);
int16_t wsa_set_rfe_input_mode(struct wsa_device *dev, char const *mode);

int16_t wsa_get_iq_output_mode(struct wsa_device *dev, char *mode);
int16_t wsa_set_iq_output_mode(struct wsa_device *dev, char const *mode);

int16_t wsa_get_antenna(struct wsa_device *dev, int32_t *port_num);
int16_t wsa_set_antenna(struct wsa_device *dev, int32_t port_num);

int16_t wsa_get_bpf_mode(struct wsa_device *dev, int32_t *mode);
int16_t wsa_set_bpf_mode(struct wsa_device *dev, int32_t mode);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude);
int16_t wsa_set_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);

int16_t wsa_get_trigger_enable(struct wsa_device *dev, int32_t *enable);
int16_t wsa_set_trigger_enable(struct wsa_device *dev, int32_t enable);

int16_t wsa_get_trigger_type(struct wsa_device *dev, char *type);
int16_t wsa_set_trigger_type(struct wsa_device *dev, char const *type);

int16_t wsa_get_trigger_sync_delay(struct wsa_device *dev, int32_t *delay);
int16_t wsa_set_trigger_sync_delay(struct wsa_device *dev, int32_t delay);

int16_t wsa_set_trigger_sync_state(struct wsa_device *dev, char const *sync_state);
int16_t wsa_get_trigger_sync_state(struct wsa_device *dev, char *sync_state);

// ////////////////////////////////////////////////////////////////////////////
// PLL SECTION                                                               //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_reference_pll(struct wsa_device *dev, char *pll_ref);
int16_t wsa_set_reference_pll(struct wsa_device *dev, char const *pll_ref);
int16_t wsa_reset_reference_pll(struct wsa_device *dev);

int16_t wsa_get_lock_ref_pll(struct wsa_device *dev, int32_t *lock_ref);
int16_t wsa_get_lock_rf(struct wsa_device *dev, int32_t *lock_rf);


// ////////////////////////////////////////////////////////////////////////////
// TEMPERATURE SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_temperature(struct wsa_device *dev, float* rfe_temp, float* mixer_temp, float* digital_temp);

///////////////////////////////////////////////////////////////////////////////
// STREAM CONTROL SECTION                                                    //
///////////////////////////////////////////////////////////////////////////////

int16_t wsa_stream_start(struct wsa_device * const dev);
int16_t wsa_stream_start_id(struct wsa_device * const dev, int64_t stream_start_id);
int16_t wsa_stream_stop(struct wsa_device * const dev);


// ////////////////////////////////////////////////////////////////////////////
// SWEEP CONTROL SECTION                                                     //
// ////////////////////////////////////////////////////////////////////////////

int16_t wsa_get_sweep_antenna(struct wsa_device *dev, int32_t *port_num);
int16_t wsa_set_sweep_antenna(struct wsa_device *dev, int32_t port_num);

int16_t wsa_get_sweep_attenuation(struct wsa_device *dev, int32_t *mode);
int16_t wsa_set_sweep_attenuation(struct wsa_device *dev, int32_t mode);

int16_t wsa_get_sweep_decimation(struct wsa_device *dev, int32_t *rate);
int16_t	wsa_set_sweep_decimation(struct wsa_device *dev, int32_t rate);

int16_t wsa_get_sweep_dwell(struct wsa_device *dev, int32_t *seconds, int32_t *microseconds);
int16_t wsa_set_sweep_dwell(struct wsa_device *dev,int32_t seconds, int32_t microseconds);
	
int16_t wsa_get_sweep_freq(struct wsa_device *dev, int64_t *freq_start, int64_t *freq_stop);
int16_t wsa_set_sweep_freq(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq);

int16_t wsa_get_sweep_freq_shift(struct wsa_device *dev, float *fshift);
int16_t wsa_set_sweep_freq_shift(struct wsa_device *dev, float fshift);

int16_t wsa_get_sweep_freq_step(struct wsa_device *dev, int64_t *step);
int16_t wsa_set_sweep_freq_step(struct wsa_device *dev, int64_t step);

int16_t wsa_get_sweep_gain_if(struct wsa_device *dev, int32_t *gain);
int16_t wsa_set_sweep_gain_if(struct wsa_device *dev, int32_t gain);

int16_t wsa_get_sweep_gain_rf(struct wsa_device *dev, char *gain);
int16_t wsa_set_sweep_gain_rf(struct wsa_device *dev, char const *gain);

int16_t wsa_get_sweep_rfe_input_mode(struct wsa_device *dev, char *mode);
int16_t wsa_set_sweep_rfe_input_mode(struct wsa_device *dev, char const *mode);

int16_t wsa_get_sweep_iteration(struct wsa_device *dev, int32_t *iteration);
int16_t wsa_set_sweep_iteration(struct wsa_device *dev, int32_t iteration);
		
int16_t wsa_get_sweep_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet);
int16_t wsa_set_sweep_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

int16_t wsa_get_sweep_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block);
int16_t wsa_set_sweep_packets_per_block(struct wsa_device *dev, int32_t packets_per_block);

int16_t wsa_get_sweep_trigger_type(struct wsa_device *dev, char *type);
int16_t wsa_set_sweep_trigger_type(struct wsa_device *dev, char const *type);

int16_t wsa_set_sweep_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);
int16_t wsa_get_sweep_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude);

int16_t wsa_get_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t *delay);
int16_t wsa_set_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t delay);

int16_t wsa_get_sweep_trigger_sync_state(struct wsa_device *dev, char *delay);
int16_t wsa_set_sweep_trigger_sync_state(struct wsa_device *dev, char const *delay);

int16_t wsa_get_sweep_status(struct wsa_device *dev, char *status);
int16_t wsa_get_sweep_entry_size(struct wsa_device *dev, int32_t *size);

int16_t wsa_sweep_entry_new(struct wsa_device *dev);
int16_t wsa_sweep_entry_save(struct wsa_device *dev, int32_t positon);
int16_t wsa_sweep_entry_copy(struct wsa_device *dev, int32_t id);
int16_t wsa_sweep_entry_delete(struct wsa_device *dev, int32_t id);
int16_t wsa_sweep_entry_delete_all(struct wsa_device *dev);
int16_t wsa_sweep_entry_read(struct wsa_device *dev, int32_t id, struct wsa_sweep_list * const sweep_list);

int16_t wsa_sweep_start(struct wsa_device *dev);
int16_t wsa_sweep_start_id(struct wsa_device *dev, int64_t sweep_start_id);
int16_t wsa_sweep_stop(struct wsa_device *dev);
int16_t wsa_sweep_resume(struct wsa_device *dev);

#endif




