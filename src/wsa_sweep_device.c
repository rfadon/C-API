#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <sys/time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "wsa_sweep_device.h"

#define HZ 1
#define KHZ 1000
// #define MHZ 1000000
#define GHZ 1000000000

// #define FIXED_POINT 32
#include "kiss_fft.h"
#include "wsa_dsp.h"
#include "wsa_debug.h"
#ifndef _TIMES_H
#define _TIMES_H

#ifdef _WIN32
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

// Calculates log2 of number.  
double log2( double n )  
{  
    // log(n)/log(2) is log2.  
    return log( n ) / log( 2 );  
}

int gettimeofday(struct timeval* t,void* timezone);

// from linux's sys/times.h

//#include <features.h>

#define __need_clock_t
#include <time.h>


/* Structure describing CPU time used by a process and its children.  */
struct tms
  {
    clock_t tms_utime;          /* User CPU time.  */
    clock_t tms_stime;          /* System CPU time.  */

    clock_t tms_cutime;         /* User CPU time of dead children.  */
    clock_t tms_cstime;         /* System CPU time of dead children.  */
  };

/* Store the CPU time used by this process and all its
   dead children (and their dead children) in BUFFER.
   Return the elapsed real time, or (clock_t) -1 for errors.
   All times are in CLK_TCKths of a second.  */
clock_t times (struct tms *__buffer);

typedef long long suseconds_t ;

#endif
#endif

/*
 * define internal constants
 */

/// mode strings
#define MODE_ZIF 1
#define MODE_HDR 2
#define MODE_SH 3
#define MODE_SHN 4
#define MODE_DECSH 5
#define MODE_DECSHN 6
#define MODE_IQIN 7
#define MODE_DD 8
#define MODE_AUTO 255

/// sample types
#define SAMPLETYPE_IQ 1
#define SAMPLETYPE_I_ONLY 2

/// error values
#define EFREQOUTOFRANGE 1
#define EUNSUPPORTED 2
#define EINVCAPTSIZE 3
#define ENOMEM 4

/*
 * define internal functions
 */
static int wsa_plan_sweep(struct wsa_power_spectrum_config *);
static int wsa_sweep_plan_load(struct wsa_sweep_device *, struct wsa_power_spectrum_config *);
static struct wsa_sweep_device_properties *wsa_get_sweep_device_properties(uint32_t);


/// a list of properties that are attributed to each mode
static struct wsa_sweep_device_properties wsa_sweep_device_properties[] = {

	// column list
	// { 
	// 	mode, sample_type, fshift_available
	// 	min_tunable, max_tunable, tuning_resolution,
	//	full_bw, usable_bw, passband_center, usable_left, usable_right
	// 	min_decimation, max_decimation,
	// }

	// SHN
	{ 
		MODE_SHN, SAMPLETYPE_I_ONLY, 1,
		50ULL*MHZ, 27ULL*GHZ, 10,
		62500*KHZ, 10*MHZ, 35*MHZ, 30*MHZ, 40*MHZ, 
		4, 512
	},
	
	// SH
	{ 
		MODE_SH, SAMPLETYPE_I_ONLY, 1,
		50ULL*MHZ, 27ULL*GHZ, 10,
		62500*KHZ, 40*MHZ, 35*MHZ, 15*MHZ, 55*MHZ, 
		4, 512
	},
	// DD
	{ 
		MODE_DD, SAMPLETYPE_I_ONLY, 1,
		50ULL*MHZ, 27ULL*GHZ, 10,
		62500*KHZ, 50*MHZ, 31.25*MHZ, 0*MHZ, 50*MHZ, 
		1, 1
	},
	// list terminator
	{ 
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0
	}
};


/**
 * convert a mode from a string to a const
 *
 * @param modestr - a string containing a mode
 * @returns 0 if the mode string is invalid, otherwise the const for that string
 */
static uint32_t mode_string_to_const(const char *modestr)
{
	if (!strcmp(modestr, "ZIF"))
		return MODE_ZIF;
	else if (!strcmp(modestr, "HDR"))
		return MODE_HDR;
	else if (!strcmp(modestr, "SH"))
		return MODE_SH;
	else if (!strcmp(modestr, "SHN"))
		return MODE_SHN;
	else if (!strcmp(modestr, "DECSH"))
		return MODE_DECSH;
	else if (!strcmp(modestr, "DECSHN"))
		return MODE_DECSHN;
	else if (!strcmp(modestr, "IQIN"))
		return MODE_IQIN;
	else if (!strcmp(modestr, "DD"))
		return MODE_DD;
	else if (!strcmp(modestr, "AUTO"))
		return MODE_AUTO;

	return 0;
}


/**
 * convert a mode from a const to a string
 *
 * @param modeint - an int containing a mode const
 * @returns the mode string if valid, otherwise NULL
 */
static const char *mode_const_to_string(uint32_t modeint)
{
	if (modeint == MODE_ZIF)
		return "ZIF";
	else if (modeint == MODE_HDR)
		return "HDR";
	else if (modeint == MODE_SH)
		return "SH";
	else if (modeint == MODE_SHN)
		return "SHN";
	else if (modeint == MODE_DECSH)
		return "DECSH";
	else if (modeint == MODE_DECSHN)
		return "DECSHN";
	else if (modeint == MODE_IQIN)
		return "IQIN";
	else if (modeint == MODE_DD)
		return "DD";
	else if (modeint == MODE_AUTO)
		return "AUTO";

	return NULL;
}

/**
 * creates a new sweep plan entry and initializes it with values given
 *
 * @param device - a pointer to the wsa we've connected to
 * @return - a pointer to the allocated sweep plan struct, or NULL on failure
 */
struct wsa_sweep_plan *wsa_sweep_plan_entry_new(uint64_t fcstart, uint64_t fcstop, uint32_t fstep, uint32_t spp, uint32_t ppb, uint8_t dd_mode)
{
	struct wsa_sweep_plan *plan;

	// alloc memory for object
	plan = malloc(sizeof(struct wsa_sweep_plan));
	if (plan == NULL)
		return NULL;

	// set all the settings in the plan
	plan->next_entry = NULL;
	plan->fcstart = fcstart;
	plan->fcstop = fcstop;
	plan->fstep = fstep;
	plan->spp = spp;
	plan->ppb = ppb;

	plan->dd_mode = dd_mode;

	return plan;
}


/**
 * creates a new sweep device object and returns it
 *
 * @param device - a pointer to the wsa we've connected to
 * @return - a pointer to the allocated struct, or NULL on failure
 */
struct wsa_sweep_device *wsa_sweep_device_new(struct wsa_device *device)
{
	struct wsa_sweep_device *sweepdev;

	// alloc memory for our object
	sweepdev = malloc(sizeof(struct wsa_sweep_device));
	if (sweepdev == NULL)
		return NULL;

	// initialize everything in the struct
	sweepdev->real_device = device;

	return sweepdev;
}


/**
 * destroys a sweep device.  This does not free the device param that was passed in initially.  only the sweep device
 *
 * @sweepdev - the object to destroy
 */
void wsa_sweep_device_free(struct wsa_sweep_device *sweepdev)
{
	// free the memory of the sweep device, (but not the real device, it came from the parent, so it's their problem)
	free(sweepdev);
}


/**
 * allocates memory to do power spectrum domain captures on the bandwidths indicated
 *
 * @param sweep_device - the sweep device
 * @param fstart - the start of the band to sweep
 * @param fstop - the end of the band to sweep
 * @param rbw - the minimum resolution bandwidth desired
 * @param mode - which mode to perform the sweep in
 * @param pscfgptr - a pointer to an unallocated power spectrum config struct
 * @returns - negative on error, otherwise the number of bytes allocated
 */
int wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	uint64_t fstart,
	uint64_t fstop,
	uint32_t rbw,
	char const *mode,
	struct wsa_power_spectrum_config **pscfgptr
)
{
	struct wsa_power_spectrum_config *pscfg;
	int result;

	// right now, we don't need sweep_device or mode, so just pretend to use it to get rid of compile warnings
	sweep_device = sweep_device;
	// alloc some memory for it
	pscfg = malloc(sizeof(struct wsa_power_spectrum_config));
	if (pscfg == NULL){
		doutf(DHIGH, "wsa_power_spectrum_alloc: Failed to initialize struct wsa_power_spectrum_config\n");
		return -15;
	}


	// init things in it that must be initted
	pscfg->sweep_plan = NULL;
	pscfg->buf = NULL;

	// copy the sweep settings into the cfg object
	pscfg->mode = mode_string_to_const(mode);
	pscfg->fstart = fstart;
	pscfg->fstop = fstop;
	pscfg->rbw = (double) rbw;

	// figure out a way to get that spectrum

	result = wsa_plan_sweep(pscfg);
	if (result < 0){
		return result;
	}

	// now allocate enough buffer for the spectrum
	pscfg->buflen = (pscfg->fstop - pscfg->fstart) / pscfg->rbw;
	pscfg->buf = malloc(sizeof(float) * pscfg->buflen);
	if (pscfg->buf == NULL) {
		free(pscfg);
		return -1;
	}
	*pscfgptr = pscfg;
	return 0;
}


/**
 * destroys a power spectrum config object
 *
 * @param cfg - the config oject to destroy
 */
void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg)
{
	struct wsa_sweep_plan *plan, *next;

	// free the plan, if there is one
	plan = cfg->sweep_plan;
	for (;;) {
		// list is null terminated
		if (plan == NULL)
			break;

		// store next pointer before freeing
		next = plan->next_entry;

		// free the memory
		free(plan);

		// go to next item
		plan = next;
	}

	// free the buffer
	if (cfg->buf)
		free(cfg->buf);

	// free the struct
	free(cfg);
}

/**
 * Configure the WSA to the configuration in the power spectrum config structure
 *
 * @param sweep_device - the sweep device to use
 * @param cfg - the power spectrum config to use
 */
void wsa_configure_sweep(struct wsa_sweep_device *sweep_device, struct wsa_power_spectrum_config *pscfg)
{
	// load the sweep plan
	wsa_sweep_plan_load(sweep_device, pscfg);
}

/**
 * captures some power spectrum using the configuration supplied
 *
 * @param sweep_device - the sweep device to use
 * @param cfg - the power spectrum config to use
 * @param buf - if buf is not NULL, a pointer to the allocated memory is stored there for convience
 * @return - 0 on success, negative on error
 */
int wsa_capture_power_spectrum(
	struct wsa_sweep_device *sweep_device,
	struct wsa_power_spectrum_config *cfg,
	float **buf
)
{
	int i;
	int16_t result;
	struct wsa_device *dev = sweep_device->real_device;
	struct wsa_vrt_packet_header header;
	struct wsa_vrt_packet_trailer trailer;
	struct wsa_receiver_packet receiver;
	struct wsa_digitizer_packet digitizer;
	struct wsa_extension_packet sweep;
	int16_t i16_buffer[32768];
	int16_t q16_buffer[32768];
	int32_t i32_buffer[32768];
	kiss_fft_scalar idata[32768];
	kiss_fft_scalar qdata[32768];
	kiss_fft_cpx fftout[32768];
	float pkt_reflevel = 0;
	uint64_t pkt_fcenter = 0;
	uint32_t buf_offset = 0;
	kiss_fft_scalar tmpscalar;
	uint32_t packet_count;
	struct wsa_sweep_device_properties *prop;
	struct wsa_sweep_device_properties *dd_prop;
	uint32_t istart, tmp_start, istop, ilen;
	uint32_t spp, fftlen;
	int16_t dd_packet = 0;

	// assign their convienence pointer
	if (*buf)
		*buf = cfg->buf;

	// poison our buffer
	for (i=0; i<cfg->buflen; i++)
		cfg->buf[i] = 77;

	// try to get device properties for this mode
	prop = wsa_get_sweep_device_properties(cfg->mode);

	if (prop == NULL) {
		fprintf(stderr, "error: unsupported rfe mode: %d - %s\n", cfg->mode, mode_const_to_string(cfg->mode));
		return -EUNSUPPORTED;
	}
	// get the properties for DD mode
	dd_prop = wsa_get_sweep_device_properties(8);
	// start the sweep
	wsa_sweep_start(sweep_device->real_device);
	// read out all the data
	packet_count = 0;
	while(1) {
		if (packet_count == 0 && cfg->sweep_plan->dd_mode == 1)
			dd_packet = 1;
		else
			dd_packet = 0;
		// poison the buffers if last packet was a data packet
		if (header.packet_type == IF_PACKET_TYPE) {
			for(i=0; i<32768; i++)
				i16_buffer[i] = 9999;
		}
		// read a packet

		result = wsa_read_vrt_packet(
			dev,
			&header, &trailer, &receiver, &digitizer, &sweep,
			i16_buffer, q16_buffer, i32_buffer,
			32768,
			5000
		);
		if (result < 0) {
			fprintf(stderr, "error: wsa_read_vrt_packet(): %d\n", result);
			return -1;
		}

		// capture digitizer context packets we need
		if (header.stream_id == DIGITIZER_STREAM_ID) {
		// grab the reflevel for each capture, so we can translate our FFTs
			if ((digitizer.indicator_field & REF_LEVEL_INDICATOR_MASK) == REF_LEVEL_INDICATOR_MASK) {
				pkt_reflevel = (float) digitizer.reference_level;
			}

		//  capture receiver context packets we need
		} else if (header.stream_id == RECEIVER_STREAM_ID) {
			// grab the center frequency for each capture
			if ((receiver.indicator_field & FREQ_INDICATOR_MASK) == FREQ_INDICATOR_MASK) {
				pkt_fcenter = (uint64_t) receiver.freq;

			}
		}

		// data packets need to be parsed
		if (header.packet_type == IF_PACKET_TYPE) {
		
			spp = header.samples_per_packet;

			/*
			 * for now, we assume it's an I16 packet
			 */

			// window and normalize the data
			normalize_iq_data(spp,
					header.stream_id,
					i16_buffer,
					q16_buffer,
					i32_buffer,
					idata,
					qdata);

			window_hanning_scalar_array(idata, spp);

			// fft this data
			rfft(idata, fftout, spp);
			fftlen = spp >> 1;

			/*
			 * we used to be in superhet mode, but after a complex FFT, we have twice 
			 * the spectrum at twice the RBW.
			 * our fcenter is now moved from center to $passband_center so our start and stop 
			 * indexes are calculated given that fact
			 */

			// check for inversion and calculate indexes of our good data
			if (trailer.spectral_inversion_indicator && dd_packet == 0) {
				reverse_cpx(fftout, fftlen);

				istart = (prop->full_bw - prop->usable_right) / cfg->rbw;
				istop = (prop->full_bw - prop->usable_left) / cfg->rbw;

			} else {

				istart = prop->usable_left / cfg->rbw;
				istop = prop->usable_right / cfg->rbw;
			}

			ilen = istop - istart;
			// make sure we don't copy beyond end of buffer
			if ((buf_offset + ilen) >= cfg->buflen) {
				// reduce istop by how much it's past
				istop = istop - ((buf_offset + ilen) - cfg->buflen);
				ilen = istop - istart;
			}

			// if we are in DD mode
			if (dd_packet == 1){
				istart = (uint32_t) (((float)cfg->fstart /  (float) prop->full_bw) * (spp / 2));
				istop = (uint32_t) (0.8 * spp / 2);
				buf_offset = 0;
				ilen = istop - istart;

			}
			
			// for the usable section, convert to power, apply reflevel and copy into buffer
			for (i=0; i<ilen; i++) {
				tmpscalar = cpx_to_power(fftout[i+ (int) istart]) / spp;

				tmpscalar = 2 * power_to_logpower(tmpscalar);
				cfg->buf[buf_offset + i] = tmpscalar + pkt_reflevel;
			}
			buf_offset = buf_offset + ilen;
			// do we have all our packets?
			packet_count++;
			
			if (packet_count >= cfg->packet_total)
				break;
		}
	}

	return 0;
}


/**
 * retrieves the appropriate property struct for the mode requested
 *
 * @param mode - a string holding the mode
 * @return - pointer to the properties struct, or NULL if not supported
 */
static struct wsa_sweep_device_properties *wsa_get_sweep_device_properties(uint32_t mode)
{
	struct wsa_sweep_device_properties *ptr;

	// loop through properties and find ours
	for (ptr = wsa_sweep_device_properties; ptr->mode; ptr++) {

		// if this is our mode, return a pointer to the entry
		if (ptr->mode == mode)
			return ptr;
	}

	// couldn't find it, return null
	return NULL;
}


/**
 * given a desired sweep configuration, this functions figures out how to achieve it
 *
 * @param sweep_device - the sweep device object we're operating on
 * @param pscfg - the config object which describes what we're trying to sweep
 * @return - negative on error, zero on success
 */
static int wsa_plan_sweep(struct wsa_power_spectrum_config *pscfg)
{
	struct wsa_sweep_device_properties *prop;
	struct wsa_sweep_plan *plan;
	uint64_t fcstart, fcstop, tmpfreq;
	uint32_t fstep;
	uint32_t half_usable_bw;
	uint8_t dd_mode = 0;
	unsigned int points;

	// try to get device properties for this mode
	prop = wsa_get_sweep_device_properties(pscfg->mode);
	if (prop == NULL) {
		doutf(DHIGH, "wsa_plan_sweep: Unsupported RFE MODE \n");
		return -EUNSUPPORTED;
	}

	/*
	 * calculate some helper variables we'll need
	 */



	// how wide is halfband for this mode?
	half_usable_bw = prop->usable_bw >> 1;

	// how many points (fft bins) are in a full band
	points = prop->full_bw / pscfg->rbw;

	// double points because superheet
	points = points << 1;

	// test value for size
	if (points < WSA_MIN_SPP)
		points = WSA_MIN_SPP;
	if (points > WSA_MAX_SPP)
		points = WSA_MAX_SPP;
    
	// make sure it's a power of 2
	points = (uint32_t) pow(2, ((unsigned int)log2(points)));

	// recalc what that actually results in for the rbw
	pscfg->rbw = ((double) prop->full_bw) / (points / 2);
	
	// change the start and stop they want into center start and stops
	fcstart = pscfg->fstart + half_usable_bw;
	// determine if DD mode is required
	if (pscfg->fstart < prop->min_tunable)
	{
		dd_mode = 1;
		fcstart = prop->min_tunable + half_usable_bw - prop->tuning_resolution;
	}
	fcstop = pscfg->fstop;
	
	// figure out our sweep step size (a bit less than usable bw.  one rbw less, yet still a multiple of tuning res)
	fstep = prop->usable_bw - pscfg->rbw;
	fstep = (fstep / prop->tuning_resolution) * prop->tuning_resolution;
	// force fcstart to a multiple of tuning resolution
	fcstart = (fcstart / prop->tuning_resolution) * prop->tuning_resolution;

	// force fcstop to a multiple of fstep past fcstart (this may cause us to need another sweep entry to clean up)
    tmpfreq = ((fcstop - fcstart) / fstep) * fstep;

	fcstop = fcstart + tmpfreq;

	// test if start frequency and stop frequency are valid
	if ((pscfg->fstart > pscfg->fstop) || (fcstart < prop->min_tunable && dd_mode == 0) || (fcstop > prop->max_tunable)){
		doutf(DHIGH, "wsa_plan_sweep: Invalid Frequency setting \n");
		return -EFREQOUTOFRANGE;
	}
	if ( dd_mode == 1 && pscfg->fstop < (prop->min_tunable + half_usable_bw))
	{
		fcstart = prop->min_tunable + half_usable_bw;
		fcstop = prop->min_tunable + half_usable_bw;
	}
	// create sweep plan objects for each entry
	pscfg->sweep_plan = wsa_sweep_plan_entry_new(fcstart, fcstop, fstep, points, 1, dd_mode);

	// do we need a cleanup entry?
	if ((fcstop + half_usable_bw) < pscfg->fstop) {
		// how much is left over? (it should be less than usable_bw)
		tmpfreq = pscfg->fstop - (fcstop + half_usable_bw);

		// make a sweep entry for fcstop + half of that
		tmpfreq = pscfg->fstop - (tmpfreq / 2);

		// which must be a multiple of freq resolution
		tmpfreq = (tmpfreq / prop->tuning_resolution) * prop->tuning_resolution;
		
		// If dd mode is used, move spectrum by half the bandwidth
		if (dd_mode == 1)
			tmpfreq = pscfg->fstop + (half_usable_bw / 2);
		// now create the entry
		pscfg->sweep_plan->next_entry = wsa_sweep_plan_entry_new(tmpfreq, tmpfreq, fstep, points, 1, dd_mode);
	}

	
	// how many steps on in this plan? loop through the list and count 'em

	pscfg->packet_total = 0;

	// add a packet for DD mode
	if (dd_mode == 1)
		pscfg->packet_total = 1;
	for (plan=pscfg->sweep_plan; plan; plan=plan->next_entry) {

		// inc packet total by (steps * ppb)
		if ((plan->fcstop - plan->fcstart) < plan->fstep)
			pscfg->packet_total += plan->ppb;
		else
			pscfg->packet_total += (((plan->fcstop - plan->fcstart) / plan->fstep) * plan->ppb) + 1;
	}
	doutf(DHIGH, "wsa_plan_sweep: finished planning the sweep\n");
	return 0;
}


/**
 * sets the attenuator in the sweep device
 *
 * @param sweep_device - the sweep device to use
 * @param value - the attenuator setting, 0 = out, 1 = in
 */
void wsa_sweep_device_set_attenuator(struct wsa_sweep_device *sweep_device, unsigned int val)
{
	sweep_device->device_settings.attenuator = val;
}


/**
 * gets the attenatuor in the sweep device
 *
 * @param sweep_device - the sweep device to use
 * @return - the attenuator setting, 0 = out, 1 = in
 */
unsigned int wsa_sweep_device_get_attenuator(struct wsa_sweep_device *sweep_device)
{
	return sweep_device->device_settings.attenuator;
}


/**
 * converts a sweep plan into a list of sweep entries and loads them onto the device
 *
 * @param sweep_device - the sweep device to use
 * @param cfg - the sweep configuration which holds all sweep info, including the sweep plan
 * @return - negative on error, 0 on success
 */
static int wsa_sweep_plan_load(struct wsa_sweep_device *wsasweepdev, struct wsa_power_spectrum_config *cfg)
{
	int result;
	struct wsa_device *wsadev = wsasweepdev->real_device;
	struct wsa_sweep_plan *plan_entry;
	char dd[255] = "DD";
	plan_entry=cfg->sweep_plan;
	// clear any existing sweep entries
	wsa_sweep_entry_delete_all(wsadev);

	// setup the sweep list to only run once
	wsa_set_sweep_iteration(wsadev, 1);

	// if DD mode is required, create one sweep entry with DD mode
	if (cfg->sweep_plan->dd_mode == 1){
		// create new entry with all the sweep entry devices
		wsa_sweep_entry_new(wsadev);
		result = wsa_set_sweep_attenuation(wsadev, wsasweepdev->device_settings.attenuator);
		// set ppb/spp settings
		result = wsa_set_sweep_rfe_input_mode(wsadev, dd);
		result = wsa_set_sweep_samples_per_packet(wsadev, (int32_t) (plan_entry->spp));
		result = wsa_set_sweep_packets_per_block(wsadev, (int32_t) (plan_entry->ppb));
		result = wsa_sweep_entry_save(wsadev, 0);
	}
	// create new entry with all the sweep entry devices
	result = wsa_sweep_entry_new(wsadev);

	// set sweep wide settings
	result = wsa_set_sweep_rfe_input_mode(wsadev, mode_const_to_string(cfg->mode));
	result = wsa_set_sweep_attenuation(wsadev, wsasweepdev->device_settings.attenuator);

	// loop over sweep plan, convert to entries and save
	for (plan_entry=cfg->sweep_plan; plan_entry; plan_entry = plan_entry->next_entry) {
		
		// set settings
		result = wsa_set_sweep_freq(wsadev, (int64_t) plan_entry->fcstart, (int64_t) plan_entry->fcstop);
		if (result < 0) fprintf(stderr, "ERROR fstart fstop\n");
		wsa_set_sweep_freq_step(wsadev, (int64_t)  plan_entry->fstep);
		if (result < 0) fprintf(stderr, "ERROR fstep\n");
		wsa_set_sweep_samples_per_packet(wsadev, plan_entry->spp);
		if (result < 0) fprintf(stderr, "ERROR spp\n");
		wsa_set_sweep_packets_per_block(wsadev, plan_entry->ppb);
		if (result < 0) fprintf(stderr, "ERROR ppb\n");

		// save to end of list
		wsa_sweep_entry_save(wsadev, 0);
	}

	return 0;
}

