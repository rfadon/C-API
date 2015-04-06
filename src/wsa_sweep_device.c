#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "wsa_sweep_device.h"


#define VRT_DEBUG 0
#define LOG_DATA_TO_FILE 1
#define LOG_DATA_TO_STDOUT 0

#define HZ 1
#define KHZ 1000
// #define MHZ 1000000
#define GHZ 1000000000

// #define FIXED_POINT 32
#include "kiss_fft.h"

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
#define EBANDTOOSMALL 3

/*
 * define internal functions
 */
static int wsa_plan_sweep(struct wsa_power_spectrum_config *);


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
		50ULL*MHZ, 20ULL*GHZ, 100*KHZ,
		62500*KHZ, 10*MHZ, 19600*KHZ, 14600*KHZ, 24600*KHZ, 
		4, 512
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
uint32_t mode_string_to_const(const char *modestr)
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
const char *mode_const_to_string(uint32_t modeint)
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
 * performs a hanning window on a scalar value
 *
 * @values - a pointer to the array of scalar values
 * @len - the length of the array that is being windowed
 * @index - the index of this item in the window
 * @returns the windowed value
 */
kiss_fft_scalar window_hanning_scalar(kiss_fft_scalar value, int len, int index)
{
	return value * 0.5 * (1 - cosf(2 * M_PI * index / (len - 1)));
}


/**
 * performs a hanning window on a list of scalar values, windowing is done in place
 *
 * @param values - a pointer to the array of scalar values
 * @param len - the length of the array
 */
void window_hanning_scalar_array(kiss_fft_scalar *values, int len)
{
	int i;

	for(i=0; i<len; i++) {
		values[i] = window_hanning_scalar(values[i], len, i);
	}
}


/**
 * performs a hanning window on a complex value in place
 *
 * @values - a pointer to the array of complex values
 * @len - the length of the array
 * @index - the index in the array we are windowing
 */
void window_hanning_cpx(kiss_fft_cpx *value, int len, int index)
{
	float mult = 0.5 * (1 - cosf(2 * M_PI * index / (len - 1)));

	value->r = value->r * mult;
	value->i = value->i * mult;
}


/**
 * performs a hanning window on a list of complex values, windowing is done in place
 *
 * @values - a pointer to the array of complex values
 * @len - the length of the array
 */
void window_hanning_cpx_array(kiss_fft_cpx *values, int len)
{
	int i;
	
	for(i=0; i<len; i++) {
		window_hanning_cpx(&values[i], len, i);
	}
}


/**
 * normalize a value
 *
 * @param value - the value to normalize
 * @param maxval - the maximum we are normalizing over
 * @returns - the normalized value
 */
kiss_fft_scalar normalize_scalar(kiss_fft_scalar value, kiss_fft_scalar maxval)
{
	return value / maxval;
}

/**
 * normalize a complex value in place
 *
 * @param value - the value to normalize
 * @param maxval - the maximum we are normalizing over
 */
void normalize_cpx(kiss_fft_cpx *value, kiss_fft_scalar maxval)
{
	value->r = value->r / maxval;
	value->i = value->i / maxval;
}


/**
 * converts a complex value to a power value
 *
 * @param value - the complex value to convert
 * @returns - the power value
 */
kiss_fft_scalar cpx_to_power(kiss_fft_cpx value)
{
	return sqrt((value.r * value.r) + (value.i * value.i));
}


kiss_fft_scalar power_to_logpower(kiss_fft_scalar value) {
	return 10 * log10(value);
}


void dump_cpx_to_file(char *filename, kiss_fft_cpx *values, int len)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "error: could not dump to file\n");
		return;
	}

	for (i=0; i<len; i++)
		fprintf(fp, "%d, %0.2f, %0.2f\n", i, values[i].r, values[i].i);

	fclose(fp);
}


void dump_scalar_to_file(char *filename, kiss_fft_scalar *values, int len)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "error: could not dump to file\n");
		return;
	}

	for (i=0; i<len; i++)
		fprintf(fp, "%d, %0.2f\n", i, values[i]);

	fclose(fp);
}


void benchmark(struct timeval *since, char *msg)
{
	struct timeval now;
	time_t diff_sec;
	suseconds_t diff_usec;

	gettimeofday(&now, NULL);

	// diff seconds
	diff_sec = now.tv_sec - since->tv_sec;

	// diff nsec.. account for wrap
	if (now.tv_usec >= since->tv_usec) {
		diff_usec = now.tv_usec - since->tv_usec;
	} else {
		diff_usec = (now.tv_usec + 1000000) - since->tv_usec;
		diff_sec -= 1;
	}

	printf("Mark -- %s -- %ld.%06u\n", msg, (long signed) diff_sec, (unsigned) diff_usec);
}

/**
 * dumps a vrt packet header to stdout
 *
 * @param header - the struct wsa_vrt_packet_header to dump
 */
void wsa_dump_vrt_packet_header(struct wsa_vrt_packet_header *header)
{
	printf("VRT Header");
	
	if (header->stream_id == RECEIVER_STREAM_ID)
		printf("(CTX_RECEIVER): ");
	else if (header->stream_id == DIGITIZER_STREAM_ID)
		printf("(CTX_DIGITIZER): ");
	else if (header->stream_id == EXTENSION_STREAM_ID)
		printf("(CTX_EXTENSION): ");
	else if (header->stream_id == I16Q16_DATA_STREAM_ID)
		printf("(DATA_I16Q16): ");
	else if (header->stream_id == I16_DATA_STREAM_ID)
		printf("(DATA_I16): ");
	else if (header->stream_id == I32_DATA_STREAM_ID)
		printf("(DATA_I32): ");
	else
		printf("(UNKNOWN=0x%08x): ", header->stream_id);

	if (header->packet_type == IF_PACKET_TYPE)
		printf("type=IF, ");
	else if (header->packet_type == CONTEXT_PACKET_TYPE)
		printf("type=CONTEXT, ");
	else if (header->packet_type == EXTENSION_PACKET_TYPE)
		printf("type=EXTENSION, ");
	else
		printf("type=UNKNOWN(%d), ", header->packet_type);
	
	printf("count=%d, spp=%u, ts:%u.%012llus\n",
		header->pkt_count,
		header->samples_per_packet,
		header->time_stamp.sec,
		header->time_stamp.psec
	);
}


/**
 * dumps a vrt receiver packet to stdout
 */
void wsa_dump_vrt_receiver_packet(struct wsa_receiver_packet *pkt)
{
	printf("  Receiver Context[");
	
	if ((pkt->indicator_field & FREQ_INDICATOR_MASK) == FREQ_INDICATOR_MASK)
		printf("freq=%0.2f ", (float)pkt->freq);

	if ((pkt->indicator_field & REF_POINT_INDICATOR_MASK) == REF_POINT_INDICATOR_MASK)
		printf("refpoint=%d ", pkt->reference_point);

	if ((pkt->indicator_field & GAIN_INDICATOR_MASK) == GAIN_INDICATOR_MASK)
		printf("gain=%lf/%lf ", pkt->gain_if, pkt->gain_rf);

	puts("]");
}


/**
 * dumps a vrt digitizer packet to stdout
 */
void wsa_dump_vrt_digitizer_packet(struct wsa_digitizer_packet *pkt)
{
	printf("  Digitizer Context[");
	
	if ((pkt->indicator_field & BW_INDICATOR_MASK) == BW_INDICATOR_MASK)
		printf("bw=%0.2f ", (float) pkt->bandwidth);

	if ((pkt->indicator_field & RF_FREQ_OFFSET_INDICATOR_MASK) == RF_FREQ_OFFSET_INDICATOR_MASK)
		printf("freqoffset=%ld ", (long int) pkt->rf_freq_offset);

	if ((pkt->indicator_field & REF_LEVEL_INDICATOR_MASK) == REF_LEVEL_INDICATOR_MASK)
		printf("reflevel=%d ", pkt->reference_level);

	puts("]");
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
	if (pscfg == NULL)
		return -1;
	*pscfgptr = pscfg;

	// init things in it that must be initted
	pscfg->sweep_plan = NULL;
	pscfg->buf = NULL;

	// copy the sweep settings into the cfg object
	pscfg->mode = mode_string_to_const(mode);
	pscfg->fstart = fstart;
	pscfg->fstop = fstop;
	pscfg->rbw = rbw;

	// figure out a way to get that spectrum
	result = wsa_plan_sweep(pscfg);
	if (result < 0)
		return result;

	// now allocate enough buffer for the spectrum
	pscfg->buflen = (pscfg->fstop - pscfg->fstart) / pscfg->rbw;
	pscfg->buf = malloc(sizeof(float) * pscfg->buflen);
	if (pscfg->buf == NULL) {
		free(pscfg);
		return -1;
	}

	return 0;
}


/**
 * destroys a power spectrum config object
 *
 * @param cfg - the config oject to destroy
 */
void wsa_power_spectrum_free(struct wsa_power_spectrum_config *cfg)
{
	// free the buffer and then the struct
	free(cfg->buf);
	free(cfg);
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
	int16_t i16_buffer[2048];
	int16_t q16_buffer[2048];
	int32_t i32_buffer[2048];
	struct timeval start;
	kiss_fft_cfg fftcfg;
	kiss_fft_cpx iq[2048];
	kiss_fft_cpx fftout[2048];
	float reflevel = 0;
	kiss_fft_scalar tmpscalar;

	gettimeofday(&start, NULL);
	benchmark(&start, "start");

	// assign their convienence pointer
	if (*buf)
		*buf = cfg->buf;

	/*
	 * do a single capture
	 */

	// flush buffer
	wsa_flush_data(dev);

	// just autotune to a good band for now
	wsa_set_rfe_input_mode(dev, WSA_RFE_SHN_STRING);
	wsa_set_freq(dev, 433 * MHZ);
	wsa_set_attenuation(dev, 0);

	// do a capture
	wsa_set_samples_per_packet(dev, 2048);
	wsa_set_packets_per_block(dev, 1);
	result = wsa_capture_block(dev);
	if (result < 0) {
		fprintf(stderr, "error: wsa_capture_block(): %d\n", result);
		return -1;
	}
	benchmark(&start, "capture");

	/*
	 * read out packets until we get a data packet
	 */
	while(1) {

		// poison the buffers
		for(i=0; i<2048; i++)
			i16_buffer[i] = 9999;

		// read a packet
		result = wsa_read_vrt_packet(
			dev,
			&header, &trailer, &receiver, &digitizer, &sweep,
			i16_buffer, q16_buffer, i32_buffer,
			2048,
			5000
		);
		if (result < 0) {
			fprintf(stderr, "error: wsa_read_vrt_packet(): %d\n", result);
			return -1;
		}
#if VRT_DEBUG
		wsa_dump_vrt_packet_header(&header);
		if (header.stream_id == RECEIVER_STREAM_ID)
			wsa_dump_vrt_receiver_packet(&receiver);
		else if (header.stream_id == DIGITIZER_STREAM_ID)
			wsa_dump_vrt_digitizer_packet(&digitizer);
#endif

		// grab the reflevel for each capture, so we can translate our FFTs
		if (header.stream_id == DIGITIZER_STREAM_ID) {
			if ((digitizer.indicator_field & REF_LEVEL_INDICATOR_MASK) == REF_LEVEL_INDICATOR_MASK) {
				reflevel = (float) digitizer.reference_level;
			}
		}

		if (header.packet_type == IF_PACKET_TYPE)
			break;
	}
	benchmark(&start, "read");

	// for now, just copy the data into the fft array.  Later, we'll figure out how to do this without the extra copy
	for (i=0; i<2048; i++) {
		iq[i].r = i16_buffer[i];
		iq[i].i = 0;
	}
	benchmark(&start, "copy");

#if LOG_DATA_TO_STDOUT
	// print
	for (i=0; i<2048; i++)
		printf("%d, %d\n", iq[i].r, iq[i].i);
#elif LOG_DATA_TO_FILE
	dump_cpx_to_file("iq.dat", iq, 2048);
#endif

	/*
	 * window and normalize our data
	 */
	for (i=0; i<2048; i++) {
		window_hanning_cpx(&iq[i], 2048, i);
		normalize_cpx(&iq[i], 8192);
	}
#if LOG_DATA_TO_STDOUT
	// print
	for (i=0; i<2048; i++)
		printf("%0.2f, %0.2f\n", fftout[i].r, fftout[i].i);
#elif LOG_DATA_TO_FILE
	dump_cpx_to_file("windowed.dat", fftout, 2048);
#endif

	/*
	 * fft that
	 */
	fftcfg = kiss_fft_alloc(2048, 0, 0, 0);
	benchmark(&start, "fft_alloc");

	kiss_fft(fftcfg, iq, fftout);
	benchmark(&start, "fft_compute");
#if LOG_DATA_TO_STDOUT
	// print
	for (i=0; i<2048; i++)
		printf("%0.2f, %0.2f\n", fftout[i].r, fftout[i].i);
#elif LOG_DATA_TO_FILE
	dump_cpx_to_file("fft.dat", fftout, 2048);
#endif

	free(fftcfg);
	benchmark(&start, "fft_free");

	/* 
	 * convert to power and apply reflevel
	 */
	for (i=0; i<2048; i++) {
		tmpscalar = cpx_to_power(fftout[i]) / 2048.0;
		tmpscalar = 2 * power_to_logpower(tmpscalar);
		cfg->buf[i] = tmpscalar + reflevel;
	}
	benchmark(&start, "copy");
#if LOG_DATA_TO_STDOUT
	// print
	for (i=0; i<2048; i++)
		printf("%d  %0.2f\n", i, cfg->buf[i]);
#elif LOG_DATA_TO_FILE
	dump_scalar_to_file("psd.dat", cfg->buf, 2048);
#endif

	return 0;
}


/**
 * retrieves the appropriate property struct for the mode requested
 *
 * @param mode - a string holding the mode
 * @return - pointer to the properties struct, or NULL if not supported
 */
struct wsa_sweep_device_properties *wsa_get_sweep_device_properties(uint32_t mode)
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
	uint64_t fcstart, fcstop;
	uint32_t fstep;
	uint32_t binsize;
	uint32_t half_usable_bw;
	unsigned int points;

	// try to get device properties for this mode
	prop = wsa_get_sweep_device_properties(pscfg->mode);
	if (prop == NULL) {
		fprintf(stderr, "error: unsupported rfe mode: %d - %s\n", pscfg->mode, mode_const_to_string(pscfg->mode));
		return -EUNSUPPORTED;
	}

	/*
	 * calculate some helper variables we'll need
	 */

	// how wide is halfband for this mode?
	half_usable_bw = prop->usable_bw >> 1;

	// how many points (fft bins) are in a full band
	points = prop->full_bw / pscfg->rbw;
	points = (uint32_t) pow(2, ceil(log2(points)));
	if (points < WSA_MIN_SPP)
		return -EBANDTOOSMALL;
	binsize = prop->full_bw / points;

	// change the start and stop they want into center start and stops
	fcstart = pscfg->fstart + half_usable_bw;
	fcstop = pscfg->fstop - half_usable_bw;

	/*
	 * commence the planning!
	 */

	// test if start and stop are valid
	if ( (pscfg->fstart > pscfg->fstop) || (fcstart < prop->min_tunable) || (fcstop > prop->max_tunable) )
		return -EFREQOUTOFRANGE;

	// figure out our sweep step size
	fstep = prop->usable_bw;

	printf("sweep list: start=%llu stop=%llu step=%lu points=%d binsize=%u\n", fcstart, fcstop, fstep, points, binsize);

	return 0;
}


/**
 * sets the attenatuor in the sweep device
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


