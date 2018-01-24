//*****************************************************************************
// A VRT data read function test code to illustrate how to
// capture a block of data
// 
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include "test_util.h"
#include "wsa_error.h"

#define RFE_MODE "SH"
#define FREQ     2450*MHZ
#define DEC      1
#define SPP      32768
#define PPB      32


/**
* Test block capture feature
* Results are stored in the pass/fail count variables
*/
int16_t block_capture_tests(struct wsa_device *dev, struct test_data *test_info)
{
	int16_t acq_result;
	int32_t array_size = 0;
	float max_buffer = -200;
	int8_t exit_loop = 0;
	int16_t result;
	int i, k;

	/*
	* Create different VRT structures & packet types
	*/

	struct wsa_vrt_packet_header *header;
	struct wsa_vrt_packet_trailer *trailer;
	struct wsa_receiver_packet *receiver;
	struct wsa_digitizer_packet *digitizer;
	struct wsa_extension_packet *extension;

	/*
	* Create buffers to store the decoded I & Q from the raw data
	*/

	int16_t *i16_buffer;
	int16_t *q16_buffer;
	int32_t *i32_buffer;
	float *fft_buffer;


	init_test_data(test_info);

	/*
	* Allocate memories for VRT structs needed for the wsa_read_vrt_packet function
	*/

	header = (struct wsa_vrt_packet_header *) malloc(sizeof(struct wsa_vrt_packet_header));
	if (header == NULL) {
		printf("failed to allocate memory for wsa_vrt_packet_header\n");

		return WSA_ERR_MALLOCFAILED;
	}

	trailer = (struct wsa_vrt_packet_trailer *) malloc(sizeof(struct wsa_vrt_packet_trailer));
	if (trailer == NULL) {
		printf("failed to allocate memory for wsa_vrt_packet_trailer\n");
		free(header);

		return WSA_ERR_MALLOCFAILED;
	}

	receiver = (struct wsa_receiver_packet *) malloc(sizeof(struct wsa_receiver_packet));
	if (receiver == NULL) {
		printf("failed to allocate memory for wsa_receiver_packet\n");
		free(header);
		free(trailer);

		return WSA_ERR_MALLOCFAILED;
	}

	digitizer = (struct wsa_digitizer_packet *) malloc(sizeof(struct wsa_digitizer_packet));
	if (digitizer == NULL) {
		printf("failed to allocate memory for wsa_digitizer_packet\n");
		free(header);
		free(trailer);
		free(receiver);

		return WSA_ERR_MALLOCFAILED;
	}

	extension = (struct wsa_extension_packet *) malloc(sizeof(struct wsa_extension_packet));
	if (extension == NULL) {
		printf("failed to allocate memory for wsa_extension_packet\n");
		free(header);
		free(trailer);
		free(receiver);
		free(digitizer);

		return WSA_ERR_MALLOCFAILED;
	}

	i16_buffer = (int16_t *) malloc(sizeof(int16_t) * SPP);
	if (i16_buffer == NULL) {
		printf("failed to allocate memory for i_buffer\n");
		free(header);
		free(trailer);
		free(receiver);
		free(digitizer);
		free(extension);

		return WSA_ERR_MALLOCFAILED;
	}

	q16_buffer = (int16_t *) malloc(sizeof(int16_t) * SPP);
	if (q16_buffer == NULL) {
		printf("failed to allocate memory for q_buffer\n");
		free(header);
		free(trailer);
		free(receiver);
		free(digitizer);
		free(extension);
		free(i16_buffer);

		return WSA_ERR_MALLOCFAILED;
	}

	i32_buffer = (int32_t *) malloc(sizeof(int32_t) * SPP);
	if (i32_buffer == NULL) {
		printf("failed to allocate memory for q_buffer\n");
		free(header);
		free(trailer);
		free(receiver);
		free(digitizer);
		free(extension);
		free(i16_buffer);
		free(q16_buffer);
		return WSA_ERR_MALLOCFAILED;
	}


	/*
	* reset the device and clear the internal memory before starting capture
	*/

	result = wsa_send_scpi(dev, "*RST");
	result = wsa_system_request_acq_access(dev, &acq_result);
	result = wsa_system_abort_capture(dev);
	if (result < 0) {
		printf("Failed aborting existing capture!\n");
		test_info->fail_count++;
		return -1;
	}

	result = wsa_flush_data(dev);
	if (result < 0) {
		printf("Failed flushing memory buffer!\n");
		test_info->fail_count++;
		return -1;
	}

	/*
	* do some device settings
	*/

	result = wsa_set_samples_per_packet(dev, SPP);
	result = wsa_set_packets_per_block(dev, PPB);
	result = wsa_set_decimation(dev, DEC);
	result = wsa_set_rfe_input_mode(dev, RFE_MODE);
	result = wsa_set_freq(dev, FREQ);
	if (result < 0) {
		printf("Failed setting freq %ld\n", FREQ);
		test_info->fail_count++;
		return -1;
	}

	printf("Start the block capture process:\n");
	result = wsa_capture_block(dev);
	if (result < 0) {
		printf("Failed capture a packet!\n");
		test_info->fail_count++;
		return -1;
	}

	// Loop to get all the requested packets, PPB
	for (k = 0; k < PPB; k++) {
		exit_loop = 0;
		// loop through all of the network VRT packets until an IF data packet is received
		while (!exit_loop) {
			result = wsa_read_vrt_packet(dev, header, trailer, receiver, digitizer,
				extension, i16_buffer, q16_buffer, i32_buffer, SPP, 5000);
			if (result < 0) {
				printf("Failed to read stream data!\n");
				test_info->fail_count++;
				break;
			}

			//printf ("%d: Get packet type %d\n", k, header->packet_type);
			// handle extension context packets
			if (header->packet_type != IF_PACKET_TYPE) {
				continue;
			}
			// handle IF packet types
			else if (header->packet_type == IF_PACKET_TYPE) {
				exit_loop = TRUE;
			}
		} // end save data while loop

		// do some processing
		result = wsa_get_fft_size(SPP, header->stream_id, &array_size);
		fft_buffer = (float *) malloc(sizeof(float) * array_size);
		result = wsa_compute_fft(SPP,
			array_size,
			header->stream_id,
			digitizer->reference_level,
			trailer->spectral_inversion_indicator,
			i16_buffer,
			q16_buffer,
			i32_buffer,
			fft_buffer);
		for (i = 0; i < array_size; i++) {
			if (max_buffer < fft_buffer[i] && i > 10) {
				max_buffer = fft_buffer[i];
				printf("%0.2f, i: %d \n", fft_buffer[i], i);
			}
		}
        
		free(fft_buffer);
	}

	
	printf("Center Frequency: %llu Hz\n", receiver->freq);
	printf("Bandwidth: %llu Hz\n", digitizer->bandwidth);
	printf("Power Peak: %0.6f dBm\n", max_buffer);
	//system("PAUSE");

	printf("----- all capture done.\n");

	free(header);
	free(trailer);
	free(receiver);
	free(digitizer);
	free(extension);
	free(i16_buffer);
	free(q16_buffer);
	free(i32_buffer);
    
	// just to indicate the test pass
	test_info->pass_count++;

	// update the test count
	test_info->test_count = test_info->pass_count + test_info->fail_count;

	return 0;
}
