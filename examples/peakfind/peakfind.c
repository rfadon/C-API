#include <stdint.h>
#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "wsa_api.h"
#include "wsa_sweep_device.h"

#define MHZ 1000000ULL
#define KHZ 1000ULL

#define MAXPEAKS 256

#define OPTIONMAXLEN 16

/**
 * this program is mostly a test case for testing the behaviour of the sweep 
 * functions.  It has one mandatory parameter which is the IP of the device.
 * the optional parameters are the sweep start, stop, rbw and number of peaks to find
 */


/**
 * syntax display function
 */
void show_syntax(void)
{
	puts("Syntax: wsa_peakfind [options] <IP>");
	puts("connects to a box at <IP> and performs a sweep, printout out peaks found");
	puts("");
	puts("Options:");
	puts("--help\tshows this help text");
	puts("--mode=<n>\twhich mode do we perform the sweep in? possible values are: shn");
	puts("--start=n\tstart frequency of sweep");
	puts("--stop=n\tstop frequency of sweep");
	puts("--rbw=n\trbw to use for the sweep");
	puts("--peaks=n\thow many peaks to find");
	puts("");
}


/**
 * parses options
 * 
 * @param option -- the string to parse
 * @param name -- pointer to a record the pointer in
 * @param value -- pointer to a record the pointer in
 * @returns -- 2 if it found an option AND value, 1 if just an option, 0 if it didn't, -1 on error
 */
int parse_option(char *option, char *name, char *value)
{
	char *ptr;
	unsigned int len, tocopy;

	// null string is an error
	if (option == NULL)
		return -1;

	// check for and trim the starting "--"
	if (strncmp(option, "--", 2))
		return 0;
	option += 2;
	len = strlen(option);

	// search for the '='.  it divides the name and value
	ptr = index(option, '=');
	if (ptr) {
		// copy name
		tocopy = ptr - option;
		if (tocopy > OPTIONMAXLEN)
			return -2;
		strncpy(name, option, tocopy);
		name[tocopy] = 0;

		// copy value
		tocopy = len - (ptr - option) - 1;
		if (tocopy > OPTIONMAXLEN)
			return -2;
		strncpy(value, ptr+1, tocopy);
		value[tocopy] = 0;

		return 2;

	} else {
		// no value found.  copy name
		if (len > OPTIONMAXLEN)
			tocopy = OPTIONMAXLEN;
		else
			tocopy = len;
		strncpy(name, option, tocopy);
		name[tocopy] = 0;

		return 1;
	}

	// how did you get here?
	return -1;
}


/**
 * Shift the contents of an array over by one item, starting at "start"
 */
void array_shift(void *buf, unsigned int itemsize, unsigned int buflen, int start)
{
	void *startptr, *curptr;

	// test buflen and start for errors
	if (buflen == 0)
		return;

	if (start > (buflen-1))
		return;

	/*
	 * start at end of array, loop in reverse until we got to "start" position. end there.
	 */
	startptr = (void *) ( ((unsigned long) buf) + (itemsize * start));
	curptr = (void *) ( ((unsigned long) buf) + (itemsize * (buflen - 1)));
	for(;;) {

		// got back to buf ptr?  we're done.
		if (curptr == startptr)
			return;

		// copy previous value to current value
		memcpy(curptr, (void *) ( ((unsigned long) curptr) - itemsize), itemsize);

		// inc pointer to prev in array
		curptr = (void *) ( ((unsigned long) curptr) - itemsize);
	}
}


int array_getpos(float *buf, int buflen, float item)
{
	int i;

	for (i=0; i<buflen; i++) {
		if (item > buf[i])
			return i;
	}

	return -1;
}


int peakfind(float *buf, uint32_t buflen, uint32_t hzperbin, int peaks, uint64_t *pfreq, float *pamp)
{
	int i, j, n;

	// make sure they asked for less peaks than we have data values
	if (peaks > buflen) {
		fprintf(stderr, "error: not enough data to find this many peaks: (%d > %d)\n", peaks, buflen);
		return -1;
	}

	// populate the result arrays with 0s
	for (i=0; i<peaks; i++) {
		pfreq[i] = 0;
		pamp[i] = -99999;
	}

	// loop through the amplitude data
	for (i=0; i<buflen; i++) {
	
		// find the spot in the array to place the item
		n = array_getpos(pamp, peaks, buf[i]);

		// shift over the array to make space for the new element
		if (n >= 0) {
			array_shift(pfreq, sizeof(uint64_t), peaks, n);
			array_shift(pamp, sizeof(float), peaks, n);

			// insert amplitude
			pamp[n] = buf[i];
			pfreq[n] = hzperbin * i;
		}
	}

	return 0;
}


/**
 * main
 */
int main(int argc, char *argv[])
{
	int i, j, n;
	char optname[OPTIONMAXLEN];
	char optvalue[OPTIONMAXLEN];
	uint64_t fstart, fstop, rbw;
	uint32_t peaks;
	char *ptr;
	char *host;
	char intf_str[40];
	char mode[16];
	struct wsa_device wsa_device;
	struct wsa_device *wsadev = &wsa_device;
	struct wsa_sweep_device wsa_sweep_device;
	struct wsa_sweep_device *wsasweepdev = &wsa_sweep_device;
	int16_t result;
	struct wsa_power_spectrum_config *pscfg;
	float *psbuf;
	uint64_t pfreq[MAXPEAKS];
	float pamp[MAXPEAKS];

	// init options
	fstart = 2000ULL * MHZ;
	fstop = 3000ULL * MHZ;
	rbw = 100 * KHZ;
	peaks = 1;
	strcpy(mode, "SH");

	// parse args
	// NOTE: this method is easy but mangles argv
	for(i=1; i<argc; i++) {
		// parse option..
		n = parse_option(argv[i], optname, optvalue);
		if (n == 0) {
			break;
		} else if (n < 0) {
			fprintf(stderr, "unexpected error 1\n");
			return -1;

		}

		// show help if they asked for it
		if (!strcmp(optname, "help")) {
			show_syntax();
			return 0;

		// set sweep mode
		} else if (!strcmp(optname, "mode")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --mode missing\n\n");
				show_syntax();
				return -1;
			}

			// copy and convert to uppercase
			for(j=0; j<OPTIONMAXLEN; j++) {
				mode[j] = toupper(optvalue[j]);

				// break on string terminated
				if (!optvalue[j])
					break;
			}
			mode[15] = 0;

		// set start frequency
		} else if (!strcmp(optname, "start")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --start missing\n\n");
				show_syntax();
				return -1;
			}
			errno = 0;
			fstart = (uint64_t) strtod(optvalue, NULL);
			if (errno) {
				fprintf(stderr, "error: could not parse fstart value: %s\n", optvalue);
				return -1;
			}

		// set stop frequency
		} else if (!strcmp(optname, "stop")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --stop missing\n\n");
				show_syntax();
				return -1;
			}
			errno = 0;
			fstop = (uint64_t) strtod(optvalue, NULL);
			if (errno) {
				fprintf(stderr, "error: could not parse fstop value: %s\n", optvalue);
				return -1;
			}

		// set rbw value
		} else if (!strcmp(optname, "rbw")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --rbw missing\n\n");
				show_syntax();
				return -1;
			}
			errno = 0;
			rbw = (uint32_t) strtod(optvalue, NULL);
			if (errno) {
				fprintf(stderr, "error: could not parse rbw value: %s\n", optvalue);
				return -1;
			}

		// set peaks value
		} else if (!strcmp(optname, "peaks")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --peaks missing\n\n");
				show_syntax();
				return -1;
			}
			errno = 0;
			peaks = strtoul(optvalue, NULL, 10);
			if (errno) {
				fprintf(stderr, "error: could not parse peaks value: %s\n", optvalue);
				return -1;
			}

		// unrecognized options
		} else {
			fprintf(stderr, "error: unrecognized option: %s\n", optname);
			show_syntax();
			return -1;
		}
	}

	/*
	 * we're done parsing options.  There must be one more, and it must be the IP
	 */
	if (i >= argc) {
		fprintf(stderr, "error: <IP> not found\n\n");
		show_syntax();
		return -1;
	}
	host = argv[i];

	printf("host: %s\n", host);
	printf("mode: %s\n", mode);
	printf("fstart: %" PRIu64 "\n", fstart);
	printf("fstop: %" PRIu64 "\n", fstop);
	printf("rbw: %" PRIu32 "\n", rbw);
	printf("peaks: %" PRIu32 "\n", peaks);

	// connect to a WSA
	printf("Connecting to WSA at %s... ", host);
	snprintf(intf_str, 39, "TCPIP::%s", host);
	result = wsa_open(wsadev, intf_str);
	if (result < 0) {
		fprintf(stderr, "error: wsa_open() failed: %d\n", result);
		return -1;
	}
	printf("connected.\n");

	// initialize WSA
	wsa_system_request_acq_access(wsadev, &result);
	wsa_system_abort_capture(wsadev);
	wsa_flush_data(wsadev);

	// create the sweep device
	wsasweepdev = wsa_sweep_device_new(wsadev);
	if (wsasweepdev == NULL) {
		fprintf(stderr, "error: unable to create sweep device\n");
		wsa_close(wsadev);
		return -1;
	}

	// allocate memory for our ffts to go in
	result = wsa_power_spectrum_alloc(wsasweepdev, fstart, fstop, rbw, mode, &pscfg);
	if (result < 0) {
		wsa_sweep_device_free(wsasweepdev);
		wsa_close(wsadev);
		return -1;
	}

	// set some settings the sweep device
	wsa_sweep_device_set_attenuator(wsasweepdev, 0);

	// capture some spectrum
	wsa_capture_power_spectrum(wsasweepdev, pscfg, &psbuf);

	// find the peaks
	peakfind(pscfg->buf, pscfg->buflen, ((fstop - fstart) / rbw), peaks, pfreq, pamp);

	// print results
	printf("\nPeaks found:\n");
	for (i=0; i<peaks; i++) {
		printf("  %0.2f dBm @ %llu\n", pamp[i], pfreq[i] + fstart);
	}

	// clean up
	wsa_power_spectrum_free(pscfg);
	wsa_sweep_device_free(wsasweepdev);
	wsa_close(wsadev);

	return 0;
}
