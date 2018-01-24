//*****************************************************************************
// A sweep function test code
// VRT data is read but not process as this test is to illustrate how to
// set up a sweep capture with multiple sweep entries
//
// Refer to the Programmer's Guide, Sweep section for more information
//
// Notes:
// - Not used in this example is wsa_sweep_start_id() function, which could 
// be used to set unique ID for the sweep.  See the Programmer's Guide
// :SWEep:LIST:STARt for more information
// - :SWEep:LIST:ITERations is left as default 0 for indefinite runs until 
// :SWEep:LIST:STOP is issued
// - Sweep with trigger example is also not illustrated here
// - Set all entries to use the same SPP to simplify the data read back 
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include "test_util.h"
#include "wsa_error.h"


#define SPP       32768
#define PPB       5
#define ITERATION 20000000
#define MAX_FREQ  8000*MHZ

/**
 * Test sweep capture feature
 * Results are stored in the pass/fail count variables
 */
int16_t sweep_tests(struct wsa_device *dev, struct test_data *test_info)
{    
	int16_t acq_result;
	int32_t array_size = 0;
    float max_buffer = -200;
	int8_t exit_loop = 0;
    int16_t result;
    int i = 0;
	
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
     * reset the device and clear the internal memory before starting 
     * the sweep setup and capture
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
     * do some sweep entries settings
     * Notes:
     * - result should be checked but not done here to simplify the reading
     *
     * - "The entry saving is done by inserting either the new entry 
     *   BEFORE the specified index value or to the end of the list when no 
     *   index (or 0) value is given". So to have frequency range saved
     *   from lowest to highest freq, either:
     *   1) Save the frequency range from highest to lowest to index 0 always
     *   2) a dummy entry is created first so that the new entries' index 
     *   get pushed into the queue in the desired right order. Then removed that
     *   dummy entry which locates at the highest index.
     *   This example use the 2nd method
     */
     
    // remove any existing sweep entries
    result = wsa_sweep_entry_delete_all(dev);
    
    // set to default all the settings under sweep *optional after delete*
    //result = wsa_sweep_entry_new(dev);
    
    // save a dummy entry
    result = wsa_sweep_entry_save(dev, 0);
       
    // first entry
    result = wsa_set_sweep_samples_per_packet(dev, SPP);
    result = wsa_set_sweep_packets_per_block(dev, PPB);
    result = wsa_set_sweep_attenuation(dev, 30);
    result = wsa_set_sweep_decimation(dev, 4);
    result = wsa_set_sweep_rfe_input_mode(dev, "SH");
    result = wsa_set_sweep_freq(dev, 100*MHZ, MAX_FREQ);//950*MHZ);
    result = wsa_set_sweep_freq_step(dev, 100*MHZ);
    
    // if index 0 is given, each save move the existing entries 
    // to the next higher count
    result = wsa_sweep_entry_save(dev, 1);
    if (result < 0) {
        printf("Failed sweep save 1!\n");
        test_info->fail_count++;
        return -1;
    }
    /*
    // second entry
    result = wsa_set_sweep_samples_per_packet(dev, SPP);
    result = wsa_set_sweep_packets_per_block(dev, 1);
    result = wsa_set_sweep_decimation(dev, 4);
    result = wsa_set_sweep_attenuation(dev, 0);
    result = wsa_set_sweep_rfe_input_mode(dev, "SH");
    result = wsa_set_sweep_freq(dev, 1400*MHZ, 4000*MHZ);
    result = wsa_set_sweep_freq_step(dev, 50*MHZ);
    result = wsa_sweep_entry_save(dev, 2);
    if (result < 0) {
        printf("Failed sweep save 2!\n");
        test_info->fail_count++;
        return -1;
    }
    
    // third entry, use copy method
    result = wsa_sweep_entry_copy(dev, 2);
    result = wsa_set_sweep_freq(dev, 4000*MHZ, 6000*MHZ);
    result = wsa_set_sweep_freq_step(dev, 40*MHZ);
    result = wsa_sweep_entry_save(dev, 3);
    if (result < 0) {
        printf("Failed sweep save 3!\n");
        test_info->fail_count++;
        return -1;
    }
    
    // forth entry, use copy method
    result = wsa_sweep_entry_copy(dev, 2);
    result = wsa_set_sweep_freq(dev, 6000*MHZ, MAX_FREQ);
    result = wsa_set_sweep_freq_step(dev, 20*MHZ);
    result = wsa_sweep_entry_save(dev, 4);
    if (result < 0) {
        printf("Failed sweep save 4!\n");
        test_info->fail_count++;
        return -1;
    }
    */
    // delete the dummy entry at the 5th position
    result = wsa_sweep_entry_delete(dev, 2);
    if (result < 0) {
        printf("Failed sweep delete 5!\n");
        test_info->fail_count++;
        return -1;
    }
    
    // do 2 iteration only
    //result = wsa_set_sweep_iteration(dev, ITERATION);
    
    printf("Start the sweep process:\n");
	result = wsa_sweep_start(dev);
    // alternately with unique start id: result = wsa_sweep_start_id(dev, 10);
    if (result < 0) {
        printf("Failed starting sweep!\n");
        test_info->fail_count++;
        return -1;
    }
    
    /* Loop for awhile just to test the sweep
     * Capture some data but not processing data here
     * Just illustrating that sweep is working
     */
    do {
        exit_loop = 0;
        // loop through all of the network VRT packets until an IF data packet is received
        while (!exit_loop) {
            result = wsa_read_vrt_packet(dev, header, trailer, receiver, digitizer,
                        extension, i16_buffer, q16_buffer, i32_buffer, SPP, 5000);
            if (result < 0) {
                printf("Failed to read sweep data!\n");
                i = ITERATION;
                break;
            }
            
            // handle extension context packets
            if (header->packet_type != IF_PACKET_TYPE) {
                if (header->stream_id == RECEIVER_STREAM_ID) {
                    // NOTE: "might" get some residual context pkts from previous
                    // capture runs if they are not removed from the queues
                    //printf("Receiver frequency: %lld Hz\n", receiver->freq);
                    printf(".");
                    
                    if (receiver->freq == MAX_FREQ) {
                        i++;
                    }
                }
                else if (header->stream_id == DIGITIZER_STREAM_ID) {
                    // add some code
                }
                    
                continue;
            }
            // handle IF packet types
            else if (header->packet_type == IF_PACKET_TYPE) {
                exit_loop = TRUE;
            }
        } // end save data while loop
    } while (i < ITERATION);
    
    // Stop the sweep process
    result = wsa_sweep_stop(dev);
    if (result < 0) {
        printf("Failed stopping sweep!\n");
        test_info->fail_count++;
        return -1;
    }
    
    // just to indicate the test pass
    test_info->pass_count++;
    
    // update the test count
    test_info->test_count = test_info->pass_count + test_info->fail_count;
    
    return 0;
}
