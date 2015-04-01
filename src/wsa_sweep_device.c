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
#define MHZ 1000000
#define GHZ 1000000000

// #define FIXED_POINT 32
#include "kiss_fft.h"


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
		printf("refpoint=%ld ", pkt->reference_point);

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
		printf("freqoffset=%Ld ", (float) pkt->rf_freq_offset);

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
 * @param pscfg - a pointer to an unallocated power spectrum config struct
 * @returns - negative on error, otherwise the number of bytes allocated
 */
int wsa_power_spectrum_alloc(
	struct wsa_sweep_device *sweep_device,
	uint64_t fstart,
	uint64_t fstop,
	uint32_t rbw,
	char const *mode,
	struct wsa_power_spectrum_config **pscfg
)
{
	struct wsa_power_spectrum_config *ptr;

	// right now, we don't need sweep_device or mode, so just pretend to use it to get rid of compile warnings
	sweep_device = sweep_device;
	mode = mode;

	// alloc some memory for it
	ptr = malloc(sizeof(struct wsa_power_spectrum_config));
	if (ptr == NULL)
		return -1;
	*pscfg = ptr;

	// copy the sweep settings into the cfg object
	ptr->fstart = fstart;
	ptr->fstop = fstop;
	ptr->rbw = rbw;
	// fix buffer size to 2048 for now
	//	ptr->buflen = (fstop - fstart) / rbw;
	ptr->buflen = 2048;
	ptr->buf = malloc(sizeof(float) * ptr->buflen);
	if (ptr->buf == NULL) {
		free(ptr);
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
