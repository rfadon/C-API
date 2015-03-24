#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "wsa_api.h"

#define MHZ 1000000ULL
#define KHZ 1000ULL

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
int parse_option(char *option, char **name, char **value)
{
	char *ptr;

	// null string is an error
	if (option == NULL)
		return -1;

	// check for and trim the starting "--"
	if (strncmp(option, "--", 2))
		return 0;
	option += 2;

	// search for the '='.  it divides the name and value
	ptr = index(option, '=');
	if (ptr) {
		// terminate option there. Name is what's before.  value is what's after.
		*ptr = 0;
		*name = option;
		*value = ptr + 1;
		return 2;
	} else {
		// no value found.  just a name
		*name = option;
		return 1;
	}

	// how did you get here?
	return -1;
}


/**
 * main
 */
int main(int argc, char *argv[])
{
	int i, n;
	char *optname, *optvalue;
	uint64_t fstart, fstop, rbw;
	uint32_t peaks;
	char *ptr;
	char *host;
	char intf_str[40];
	char mode[16];
	struct wsa_device wsa_device;
	struct wsa_device *wsadev = &wsa_device;
	int result;

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
		n = parse_option(argv[i], &optname, &optvalue);
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
				fprintf(stderr, "error: value for --start missing\n\n");
				show_syntax();
				return -1;
			}

			// copy and convert to uppercase
			for(i=0;;i++) {
				// break on string terminated
				if (!optvalue[i])
					break;

				// break on max string len
				if (i >= 16)
					break;

				mode[i] = toupper(optvalue[i]);
			}

		// set start frequency
		} else if (!strcmp(optname, "start")) {
			if (n != 2) {
				fprintf(stderr, "error: value for --start missing\n\n");
				show_syntax();
				return -1;
			}
			errno = 0;
			fstart = strtoull(optvalue, NULL, 10);
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
			fstop = strtoull(optvalue, NULL, 10);
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
			rbw = strtoul(optvalue, NULL, 10);
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

	// create the sweep device


	// capture some spectrum


	// find the peaks


	// print results


	// clean up
	wsa_close(wsadev);

	return 0;
}
