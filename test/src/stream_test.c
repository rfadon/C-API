//*****************************************************************************
// A stream function test code
// VRT data is read but not process as this test is to illustrate how to
// start & stop a stream capture
//
// Not used in this example is wsa_stream_start_id() function, which could 
// be used to set unique ID for the stream.  See the Programmer's Guide
// :TRACe:STReam:STARt for more information
//
// Time stamp of each data packet is used to detect sample overflow/loss
//
// NOTE: 
// - Decimation is used in this test to slow down the data returned since 
// streaming might over flow the internal buffer.  If necessary, see "Trailer 
// Word Format" section of the Programmer's Guide for the Sample Loss Indicator
// of the trailer to verify if data has been dropped.
// 
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <time.h>
#include "test_util.h"
#include "wsa_error.h"

#define RFE_MODE   "SHN"
#define FREQ       2450*MHZ
#define DEC        6
#define SPP        32768

#define MAX_LOOP   9000000
#define PICOS_SEC  1000000000000
#define SAMPLE_CLK 8000 // 1 / (125*MHZ) * 1_000_000_000_000 = 8000 psec

struct wsa_time get_timestamp_diff(struct wsa_time start, struct wsa_time stop);

/**
 * Test stream feature
 */
int16_t stream_tests(struct wsa_device *dev, struct test_data *test_info)
{    
	int16_t acq_result;
    int16_t result;
    int8_t exit_loop = 0;
    uint64_t i = 0;
	
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
	
    /*
     * For timing how long the stream lasted until first data loss
     */
	clock_t run_start_time;
	clock_t run_end_time;
	float run_time_ms = 0;
    
    
    /*
     * For finding the time between 2 consecutive packets, using pkt timestamp
     */
    struct wsa_time prev_pkt_ts;
    struct wsa_time pkts_ts_diff;
    uint64_t pkt_sample_time;
    uint64_t ts_diff_err;
    
    pkt_sample_time = (uint64_t) SPP * DEC * SAMPLE_CLK;
    printf("Per packet sample time: %lld psec\n\n", pkt_sample_time);
    
    prev_pkt_ts.sec = 0;
    prev_pkt_ts.psec = 0;
    
    // initialize the test info struct
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
     * reset the device and clear the internal memory before starting stream
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
     
    result = wsa_set_rfe_input_mode(dev, RFE_MODE);
    result = wsa_set_samples_per_packet(dev, SPP);
    result = wsa_set_decimation(dev, DEC);
    result = wsa_set_freq(dev, FREQ);
    if (result < 0) {
        printf("Failed setting freq %ld\n", FREQ);
        test_info->fail_count++;
        return -1;
    }
    
    printf("Start the stream process ...\n");
    result = wsa_stream_start(dev);
    //alternately, using unique ID: result = wsa_stream_start_id(dev, 10);
    if (result < 0) {
        printf("Failed starting stream!\n");
        test_info->fail_count++;
        return -1;
    }
    
	run_start_time = clock();
    
    /* 
     * Loop for awhile just to test the stream
     * Capture some data but not processing data here
     * Just illustrating that stream is working
     */
    do {
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
            
            // handle extension context packets
            if (header->packet_type != IF_PACKET_TYPE) {
                continue;
            }
            // handle IF packet types
            else if (header->packet_type == IF_PACKET_TYPE) {
                exit_loop = TRUE;
            }
        }
        
        // Calculate the time diff bwn 2 consecutive pkts, starting on 
        // the 2nd capture.
        if (i > 0) {
            pkts_ts_diff = get_timestamp_diff(prev_pkt_ts, header->time_stamp);
            
            // find the time error when compare with the expected packet 
            // sample time
            if (pkts_ts_diff.psec > pkt_sample_time)
                ts_diff_err = pkts_ts_diff.psec - pkt_sample_time;
            else
                ts_diff_err = pkt_sample_time - pkts_ts_diff.psec;
            
            // if the error is greater than 1 sample clock (8000 psec), stop
            if ((ts_diff_err > SAMPLE_CLK) || pkts_ts_diff.sec > 0) {
                printf("%d ", i);
                /*printf("Capture time error: %lld psec. Data loss at packet number %d.\n", ts_diff_err, i+1);
                printf("Prev pkt time: %d sec %lld psec\n", prev_pkt_ts.sec, prev_pkt_ts.psec);
                printf("Current pkt time: %d sec %lld psec\n", header->time_stamp.sec, header->time_stamp.psec);
                printf("Packets time diff: %d sec %lld psec\n", pkts_ts_diff.sec, pkts_ts_diff.psec);*/
                
                //break;
            }
        }
        // update the prev pkt timestamp
        prev_pkt_ts = header->time_stamp;
        
        if (trailer->sample_loss_indicator) {
            printf("Sample loss: %d\n", trailer->sample_loss_indicator);
            break;
        }
        
        i++;
        if ((i%10000) == 0) printf(".");
    } while ((i < MAX_LOOP));
    
    run_end_time = clock();
    
    // Stop the stream process
    result = wsa_stream_stop(dev);
    if (result < 0) {
        printf("Failed stopping stream!\n");
        test_info->fail_count++;
        return -1;
    }
    
    run_time_ms = ((float)(run_end_time - run_start_time) / 1000000.0F ) * 1000;
    
    printf("\nTotal run time: %.3f sec\n", run_time_ms);
    
    // just to indicate the test pass
    test_info->pass_count++;
    
    // update the test count
    test_info->test_count = test_info->pass_count + test_info->fail_count;
    
    return 0;
}

/**
 * Find the time different between to \b wsa_time structs
 */
struct wsa_time get_timestamp_diff(struct wsa_time start, struct wsa_time stop)
{
    struct wsa_time pkts_ts_diff;
    
    pkts_ts_diff.sec = stop.sec - start.sec;
    if (stop.psec > start.psec)
        pkts_ts_diff.psec = stop.psec - start.psec;
    else {
        pkts_ts_diff.psec = PICOS_SEC - start.psec + stop.psec;
        pkts_ts_diff.sec = pkts_ts_diff.sec - 1;
    }
    
    return pkts_ts_diff;
}
