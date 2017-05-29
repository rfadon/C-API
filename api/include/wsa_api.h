#ifndef __WSA_API_H__
#define __WSA_API_H__


#include "wsa_lib.h"

#ifdef _BUILDASDLL_
#ifdef _DLL_
#define DECL _declspec(dllexport)
#else
#define DECL _declspec(dllimport)
#endif // _DLL_
#else
#define DECL
#endif // _BUILDASDLL_


// ////////////////////////////////////////////////////////////////////////////
// DEBUG FUNCTIONS                                                           //
// ////////////////////////////////////////////////////////////////////////////

DECL void wsa_debuglevel(int);
DECL void wsa_debugcallback(void(*callback)(void * pvoid, char const * pstring), void * pvoid);



// ////////////////////////////////////////////////////////////////////////////
// WSA RELATED FUNCTIONS                                                     //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_open(struct wsa_device *dev, char *intf_method);
DECL int16_t wsa_reset(struct wsa_device *dev);
DECL int16_t wsa_ping(struct wsa_device *dev, char *intf_method);
DECL void wsa_close(struct wsa_device *dev);

DECL int16_t wsa_check_addr(char const *ip_addr);
DECL int16_t wsa_check_addrandport(char const *ip_addr, char const *port);
DECL int16_t wsa_is_connected(struct wsa_device *dev);
DECL const char *wsa_get_err_msg(int16_t err_code);

DECL int16_t wsa_do_scpi_command_file(struct wsa_device *dev, char const *file_name);

DECL int16_t wsa_query_scpi(struct wsa_device *dev, char const *command, char *response);
DECL int16_t wsa_send_scpi(struct wsa_device *dev, char const *command);


// ////////////////////////////////////////////////////////////////////////////
// PROBE SECTION                                                             //
// ////////////////////////////////////////////////////////////////////////////

DECL void * wsa_probe_begin(void);
DECL int    wsa_probe_poll(void * handle, int timeout, char * probe, char * device, char * serial, char * version, int stringsize);
DECL void   wsa_probe_end(void * handle);


// ////////////////////////////////////////////////////////////////////////////
// LAN CONFIGURATION SECTION                                                 //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_lan_config(struct wsa_device *dev, char const *config, char *lan_config);
DECL int16_t wsa_set_lan_config(struct wsa_device *dev, char const *lan_config);

DECL int16_t wsa_get_lan_ip(struct wsa_device *dev, char const *config, char *ip);
DECL int16_t wsa_set_lan_ip(struct wsa_device *dev, char const *ip);

DECL int16_t wsa_get_lan_netmask(struct wsa_device *dev, char const *config, char *netmask);
DECL int16_t wsa_set_lan_netmask(struct wsa_device *dev, char const *netmask);

DECL int16_t wsa_get_lan_gateway(struct wsa_device *dev, char const *config, char *gateway);
DECL int16_t wsa_set_lan_gateway(struct wsa_device *dev, char const *gateway);

DECL int16_t wsa_get_lan_dns(struct wsa_device *dev, char const *config, char *dns);
DECL int16_t wsa_set_lan_dns(struct wsa_device *dev, char const *dns, char const *alternate_dns);

DECL int16_t wsa_apply_lan_config(struct wsa_device *dev);


// ////////////////////////////////////////////////////////////////////////////
// AMPLITUDE SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_abs_max_amp(struct wsa_device *dev, enum wsa_gain gain, 
						  float *value);

// ////////////////////////////////////////////////////////////////////////////
// DATA ACQUISITION SECTION                                                  //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_system_request_acq_access(struct wsa_device *dev, int16_t *status);
DECL int16_t wsa_system_acq_status(struct wsa_device *dev, int16_t *status);

DECL int16_t wsa_get_capture_mode(struct wsa_device * const dev, char *mode);
DECL int16_t wsa_system_abort_capture(struct wsa_device *dev);

DECL int16_t wsa_flush_data(struct wsa_device *dev);
DECL int16_t wsa_clean_data_socket(struct wsa_device *dev);

DECL int16_t wsa_capture_block(struct wsa_device * const dev);

DECL int16_t wsa_read_vrt_packet (struct wsa_device * const dev, 
		struct wsa_vrt_packet_header * const header, 
		struct wsa_vrt_packet_trailer * const trailer,
		struct wsa_receiver_packet * const receiver,
		struct wsa_digitizer_packet *digitizer,
		struct wsa_extension_packet * const sweep,
		int16_t * const i16_buffer,
		int16_t * const q16_buffer,
		int32_t * const i32_buffer,
		int32_t samples_per_packet,
		uint32_t timeout);

DECL int16_t wsa_get_fft_size(int32_t const samples_per_packet, uint32_t const stream_id, int32_t *array_size);

DECL int16_t wsa_compute_fft(int32_t const samples_per_packet,
						int32_t const fft_size,
						uint32_t const stream_id,
						int16_t const reference_level,
						uint8_t const spectral_inversion,
						int16_t * const i16_buffer,
						int16_t * const q16_buffer,
						int32_t * const i32_buffer,
						float * fft_buffer
						);

DECL int16_t peak_find(struct wsa_device *dev, 
					uint64_t fstart, 
					uint64_t fstop, 
					uint32_t rbw, 
					char *mode,
					int32_t attenuator,
					uint64_t *peak_freq,
					float *peak_power);

DECL int16_t calculate_channel_power(struct wsa_device *dev, 
					uint64_t fstart, 
					uint64_t fstop, 
					uint32_t rbw, 
					char *mode,
					int32_t attenuator,
					float *channel_power);

DECL int16_t calculate_occupied_bandwidth(struct wsa_device *dev, 
					uint64_t fstart, 
					uint64_t fstop, 
					uint32_t rbw, 
					float occupied_percentage,
					char *mode,
					int32_t attenuator,
					uint64_t *occupied_bw);

DECL int16_t wsa_get_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet);
DECL int16_t wsa_set_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

DECL int16_t wsa_get_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block);
DECL int16_t wsa_set_packets_per_block(struct wsa_device *dev, int32_t packets_per_block);

DECL int16_t wsa_get_decimation(struct wsa_device *dev, int32_t *rate);
DECL int16_t wsa_set_decimation(struct wsa_device *dev, int32_t rate);


// ////////////////////////////////////////////////////////////////////////////
// FREQUENCY SECTION                                                         //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_freq(struct wsa_device *dev, int64_t *cfreq);
DECL int16_t wsa_set_freq(struct wsa_device *dev, int64_t cfreq);

DECL int16_t wsa_get_freq_shift(struct wsa_device *dev, float *fshift);
DECL int16_t wsa_set_freq_shift(struct wsa_device *dev, float fshift);

DECL int16_t wsa_get_spec_inv(struct wsa_device *dev, int64_t freq, int16_t *inv);

// ////////////////////////////////////////////////////////////////////////////
// GAIN SECTION                                                              //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_attenuation(struct wsa_device *dev, int32_t *mode);
DECL int16_t wsa_set_attenuation(struct wsa_device *dev, int32_t mode);

DECL int16_t wsa_get_gain_if(struct wsa_device *dev, int32_t *gain);
DECL int16_t wsa_set_gain_if(struct wsa_device *dev, int32_t gain);

// ////////////////////////////////////////////////////////////////////////////
// RFE CONTROL SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_rfe_input_mode(struct wsa_device *dev, char *mode);
DECL int16_t wsa_set_rfe_input_mode(struct wsa_device *dev, char const *mode);

DECL int16_t wsa_get_iq_output_mode(struct wsa_device *dev, char *mode);
DECL int16_t wsa_set_iq_output_mode(struct wsa_device *dev, char const *mode);


// ////////////////////////////////////////////////////////////////////////////
// TRIGGER SECTION                                                           //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude);
DECL int16_t wsa_set_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);

DECL int16_t wsa_get_trigger_enable(struct wsa_device *dev, int32_t *enable);
DECL int16_t wsa_set_trigger_enable(struct wsa_device *dev, int32_t enable);

DECL int16_t wsa_get_trigger_type(struct wsa_device *dev, char *type);
DECL int16_t wsa_set_trigger_type(struct wsa_device *dev, char const *type);

DECL int16_t wsa_get_trigger_sync_delay(struct wsa_device *dev, int32_t *delay);
DECL int16_t wsa_set_trigger_sync_delay(struct wsa_device *dev, int32_t delay);

DECL int16_t wsa_set_trigger_sync_state(struct wsa_device *dev, int32_t *sync_state);
DECL int16_t wsa_get_trigger_sync_state(struct wsa_device *dev, int32_t *sync_state);

// ////////////////////////////////////////////////////////////////////////////
// PLL SECTION                                                               //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_reference_pll(struct wsa_device *dev, char *pll_ref);
DECL int16_t wsa_set_reference_pll(struct wsa_device *dev, char const *pll_ref);
DECL int16_t wsa_reset_reference_pll(struct wsa_device *dev);

DECL int16_t wsa_get_lock_ref_pll(struct wsa_device *dev, int32_t *lock_ref);
DECL int16_t wsa_get_lock_rf(struct wsa_device *dev, int32_t *lock_rf);


// ////////////////////////////////////////////////////////////////////////////
// TEMPERATURE SECTION                                                       //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_temperature(struct wsa_device *dev, float* rfe_temp, float* mixer_temp, float* digital_temp);

///////////////////////////////////////////////////////////////////////////////
// STREAM CONTROL SECTION                                                    //
///////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_stream_start(struct wsa_device * const dev);
DECL int16_t wsa_stream_start_id(struct wsa_device * const dev, int64_t stream_start_id);
DECL int16_t wsa_stream_stop(struct wsa_device * const dev);


// ////////////////////////////////////////////////////////////////////////////
// SWEEP CONTROL SECTION                                                     //
// ////////////////////////////////////////////////////////////////////////////

DECL int16_t wsa_get_sweep_attenuation(struct wsa_device *dev, int32_t *mode);
DECL int16_t wsa_set_sweep_attenuation(struct wsa_device *dev, int32_t mode);

DECL int16_t wsa_get_sweep_decimation(struct wsa_device *dev, int32_t *rate);
DECL int16_t wsa_set_sweep_decimation(struct wsa_device *dev, int32_t rate);

DECL int16_t wsa_get_sweep_dwell(struct wsa_device *dev, int32_t *seconds, int32_t *microseconds);
DECL int16_t wsa_set_sweep_dwell(struct wsa_device *dev,int32_t seconds, int32_t microseconds);
	
DECL int16_t wsa_get_sweep_freq(struct wsa_device *dev, int64_t *freq_start, int64_t *freq_stop);
DECL int16_t wsa_set_sweep_freq(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq);

DECL int16_t wsa_get_sweep_freq_shift(struct wsa_device *dev, float *fshift);
DECL int16_t wsa_set_sweep_freq_shift(struct wsa_device *dev, float fshift);

DECL int16_t wsa_get_sweep_freq_step(struct wsa_device *dev, int64_t *step);
DECL int16_t wsa_set_sweep_freq_step(struct wsa_device *dev, int64_t step);

DECL int16_t wsa_get_sweep_gain_if(struct wsa_device *dev, int32_t *gain);
DECL int16_t wsa_set_sweep_gain_if(struct wsa_device *dev, int32_t gain);

DECL int16_t wsa_get_sweep_rfe_input_mode(struct wsa_device *dev, char *mode);
DECL int16_t wsa_set_sweep_rfe_input_mode(struct wsa_device *dev, char const *mode);

DECL int16_t wsa_get_sweep_iteration(struct wsa_device *dev, int32_t *iteration);
DECL int16_t wsa_set_sweep_iteration(struct wsa_device *dev, int32_t iteration);
		
DECL int16_t wsa_get_sweep_samples_per_packet(struct wsa_device *dev, int32_t *samples_per_packet);
DECL int16_t wsa_set_sweep_samples_per_packet(struct wsa_device *dev, int32_t samples_per_packet);

DECL int16_t wsa_get_sweep_packets_per_block(struct wsa_device *dev, int32_t *packets_per_block);
DECL int16_t wsa_set_sweep_packets_per_block(struct wsa_device *dev, int32_t packets_per_block);

DECL int16_t wsa_get_sweep_trigger_type(struct wsa_device *dev, char *type);
DECL int16_t wsa_set_sweep_trigger_type(struct wsa_device *dev, char const *type);

DECL int16_t wsa_set_sweep_trigger_level(struct wsa_device *dev, int64_t start_freq, int64_t stop_freq, int32_t amplitude);
DECL int16_t wsa_get_sweep_trigger_level(struct wsa_device *dev, int64_t *start_freq, int64_t *stop_freq, int32_t *amplitude);

DECL int16_t wsa_get_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t *delay);
DECL int16_t wsa_set_sweep_trigger_sync_delay(struct wsa_device *dev, int32_t delay);

DECL int16_t wsa_get_sweep_trigger_sync_state(struct wsa_device *dev, char *delay);
DECL int16_t wsa_set_sweep_trigger_sync_state(struct wsa_device *dev, char const *delay);

DECL int16_t wsa_get_sweep_status(struct wsa_device *dev, char *status);
DECL int16_t wsa_get_sweep_entry_size(struct wsa_device *dev, int32_t *size);

DECL int16_t wsa_sweep_entry_new(struct wsa_device *dev);
DECL int16_t wsa_sweep_entry_save(struct wsa_device *dev, int32_t positon);
DECL int16_t wsa_sweep_entry_copy(struct wsa_device *dev, int32_t id);
DECL int16_t wsa_sweep_entry_delete(struct wsa_device *dev, int32_t id);
DECL int16_t wsa_sweep_entry_delete_all(struct wsa_device *dev);
DECL int16_t wsa_sweep_entry_read(struct wsa_device *dev, int32_t id, struct wsa_sweep_list * const sweep_list);

DECL int16_t wsa_sweep_start(struct wsa_device *dev);
DECL int16_t wsa_sweep_start_id(struct wsa_device *dev, int64_t sweep_start_id);
DECL int16_t wsa_sweep_stop(struct wsa_device *dev);
DECL int16_t wsa_sweep_resume(struct wsa_device *dev);

#endif




