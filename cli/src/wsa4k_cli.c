/**
 * @mainpage Introduction
 *
 * This documentation, compiled using Doxygen, shows in details the code
 * structure of the CLI (Command Line Interface) tool. It provides information 
 * on all the libraries involved. \n \n
 * The following diagram illustrates the different layers and libraries 
 * involved in interfacing with a WSA on the PC side.  
 *
 * @image html wsa4000_cli_2.PNG
 * @image latex wsa4000_cli_2.PNG "Interface Layers to WSA on PC Side" width=11cm
 *
 * The CLI interfaces to a WSA through the \b wsa_api library, which provides
 * functions to set/get particular settings or data from the WSA.  The \b 
 * wsa_api encodes the commands into SCPI syntax scripts, which are sent 
 * to a WSA through the \b wsa_lib library.  Subsequently decodes any 
 * responses or packets coming back from the WSA through the \b wsa_lib. \n \n
 *
 * The \b wsa_lib, thus, is the main gateway to a WSA box from a PC.  The 
 * \b wsa_lib has functions to open, close, send/receive commands, 
 * querry the WSA box status, and get data.  In this version, \b wsa_lib 
 * calls the wsa_client's functions in the transport layer to establish TCP/IP 
 * specific connections.  Other connection methods such as USB could be 
 * added to the transport layer later on.  The \b wsa_lib, thus, abstracts 
 * away the interface method from any application/presentation program 
 * calling it.
 *
 * The CLI, hence, is a direct example of how the \b wsa_api library could be 
 * used.  VRT data packet will be decoded before saving into a file.
 *  
 * The WSA4000 CLI is designed using mixed C/C++ languages.
 * The CLI when executed will run in a Windows command prompt console. List 
 * of commands available with the CLI is listed in the print_cli_menu() 
 * function. \n \n
 *
 * @section update Release v1.1
 * - Can set various sample size. Use get max to determine the limit.
 * - IF gain set/get is now available.
 * 
 * @section limitation Limitations in Release v1.1
 * The following features are not yet supported with the CLI:
 *  - VRT trailer extraction. Bit fields are yet to be defined.
 *  - Data streaming. Currently only supports block mode.
 *  - DC correction.  
 *  - IQ correction.  
 *  - Automatic finding of a WSA box(s) on a network.
 *  - Triggers.
 *  - Gain calibrarion. TBD with triggers.
 *  - USB interface method.
 *
 * @section usage Usage
 * wsa4k_cli.exe ran as a Windows console application. Use 'h' to see the 
 * options available.  An IP address is required at the first prompt to 
 * connect to the box.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_error.h"
#include "wsa_commons.h"

//*****
// Local functions
//*****
void print_cli_menu(struct wsa_device *dev);
void print_wsa_stat(struct wsa_device *dev);

int16_t gain_rf_to_str(enum wsa_gain gain, char *gain_str);

char* get_input_cmd(uint8_t pretext);
int16_t process_cmd_string(struct wsa_device *dev, char *cmd_str);
int8_t process_cmd_words(struct wsa_device *dev, char *cmd_words[], 
					int16_t num_words);
int16_t wsa_set_cli_command_file(struct wsa_device *dev, char *file_name);
int16_t save_data_to_file(struct wsa_device *dev, char *prefix, char *ext);
void print_sweep_entry(struct wsa_device *dev);
void print_sweep_entry_information(struct wsa_device *dev, int32_t position);

/**
 * Print out the CLI options menu
 *
 * @param dev - a wsa device structure.
 *
 * @return None
 */
void print_cli_menu(struct wsa_device *dev)
{
	uint64_t MIN_FREQ = dev->descr.min_tune_freq;
	uint64_t MAX_FREQ = dev->descr.max_tune_freq;
	int32_t MAX_IF_GAIN = dev->descr.min_if_gain;
	int32_t MIN_IF_GAIN = dev->descr.max_if_gain;
	uint64_t FREQ_RES = dev->descr.freq_resolution;

	printf("\n---------------------------\n");
	printf("\nCommand Options Available (case insensitive, < > required, "
		"[ ] optional):\n\n");
	printf(" dir                    List the captured file path.\n");
	printf(" h                      Show the list of available options.\n");
	printf(" o                      Open the folder of captured file(s).\n");
	printf(" q                      Quit or exit this console.\n");
	printf(" run cmdf <scpi | cli> <file name> \n"
		   "                        Run commands stored in a text file.\n"
		   "                        Note: Process only set or get commands.\n");
	printf(" save [name] [ext:<type>]\n"
		   "                        Save data to a file with optional "
									"prefix string.\n"
		   "                        Output: [name] YYYY-MM-DD_HHMMSSmmm.[ext]\n"
		   "                        - ext type: csv (default), xsl, dat, ...\n"
		   "                        ex: save Test trial ext:xsl\n"
		   "                            save\n");
	printf("\n");
	printf("//////////////////////////////////////////////////////////////////\n");
	printf("//////////////////Manual Capture Commands/////////////////////////\n");
	printf("//////////////////////////////////////////////////////////////////\n");
	printf("\n"); 
	printf(" get ant                Show the current antenna port in use.\n");
	printf(" get bpf                Show the current RFE's preselect BPF "
									"state.\n");
	printf(" get dec [max | min]    Get the decimation rate (0 = off).\n");
	
	printf(" get freq [max | min]   Show the current running centre frequency "  
									"(in MHz).\n");	
	printf(" get fshift [max | min] Get the frequency shift value (in MHz).\n");  

	printf(" get gain <rf | if> [max | min] \n"
		   "                        Show the current RF front end or IF gain "
									"level.\n");
	printf(" get ppb                Show the current packets per block.\n");
	printf(" get spp [max | min]    Show the current samples per packet.\n");
	printf(" get trigger level      Show the current level trigger settings\n");
	printf(" get trigger enable     Check whether trigger mode is enabled\n");
	printf("\n");
	printf(" set ant <1 | 2>        Select the antenna port, available 1 to "
									"%d.\n", WSA_RFE0560_MAX_ANT_PORT);
	printf(" set bpf <on | off>     Turn the RFE's preselect BPF stage on "
									"or off.\n");
	printf(" set dec <rate>	        Set decimation rate.\n"
		   "                        - Range: 0 (for off), %d - %d.\n", 
		   dev->descr.min_decimation, dev->descr.max_decimation);
	printf(" set freq <freq>        Set the centre frequency in MHz \n"
		   "                        - Range: %.2f - %.2f MHz inclusively, but\n"
		   "                          excluding 40.1 - 89.9 MHz.\n"
		   "                        - Resolution %.2f MHz.\n"
		   "                        ex: set freq 441.5\n", 
		   (float) MIN_FREQ/MHZ, (float) MAX_FREQ/MHZ, (float) FREQ_RES/MHZ);
	printf(" set fshift <freq>      Set the frequency shift in MHz \n"
		   "                        - Range: %f - %f MHz, \"exclusive\".\n"
		   "                        ex: set fshift 10\n", 
		   (float) dev->descr.inst_bw/MHZ * -1, (float) dev->descr.inst_bw/MHZ);
	printf(" set gain <rf | if> <val> Set gain level for RF front end or IF\n"
		   "                        - RF options: HIGH, MEDium, LOW, VLOW.\n"
		   "                        - IF range: %d to %d dBm, inclusive.\n"
		   "                        ex: set gain rf high;\n"
		   "                            set gain if -20.\n", 
		   MIN_IF_GAIN, MAX_IF_GAIN);
	printf(" set ppb <packets>      Set the number of packets per block to be "
									"captured\n"
		   "                        - The maximum value will depend on the\n"
		   "                          \"samples per packet\" setting\n"
		   "                        ex: set ppb 100\n");
	printf(" set spp <samples>      Set the number of samples per packet to be"
									" captured\n"
		   "                        - Range: %hu - %hu, inclusive.\n"
		   "                        ex: set spp 2000\n",
		   WSA4000_MIN_SAMPLES_PER_PACKET, WSA4000_MAX_SAMPLES_PER_PACKET);
	printf(" set trigger enable <on | off>\n"
		   "                        Set trigger mode on or off.\n"
		   "                        - When set to off, WSA will be freerun\n"
		   "                        ex: set trigger enable on\n");
	printf(" set trigger level <start,stop,amplitude>\n"
		   "                        Configure the level trigger options:\n"
		   "                          1) Start frequnecy (in MHz)\n"
		   "                          2) Stop frequnecy (in MHz)\n"
		   "                          3) Amplitude (in dBm)\n"
		   "                        ex: set trigger level 2410,2450,-50\n");
	printf("\n");
	printf("//////////////////////////////////////////////////////////////////\n");
	printf("//////////////////Sweep Capture Commands//////////////////////////\n");
	printf("//////////////////////////////////////////////////////////////////\n");
	printf("\n"); 
	printf(" get sweep entry        Shows the current settings in the user's\n"
		   "                        sweep entry list\n");
	printf(" get sweep status       Shows the current sweep status\n");

	printf(" set sweep entry ant <1 | 2>  Set the current antenna port used in\n"
		   "                        the user's sweep working entry\n");
	printf(" set sweep entry dec <rate>   Set the decimation rate used in\n"
		   "                        the the user's working sweep entry\n");
	printf(" set sweep entry gain <rf | if> \n"
		   "                        Set the current RF front end or IF gain level in\n"
		   "                        the the user's working sweep entry\n");
	printf(" set sweep entry freq <freq>  Set the current running centre frequency used in\n"
		   "                        the user's working sweep entry \n");
	printf(" set sweep entry fshift<freq> Set the frequency shift value in\n"
		   "                        the user's working sweep entry\n");
	printf(" set sweep entry ppb <packets> Set the current packets per block in\n"
		   "                        the user's working sweep entry\n");
	printf(" set sweep entry spp <samples> Set the current samples per packet in\n"
		   "                        the user's working sweep entry\n");	
	printf(" sweep list copy <sweep entry> Copy a sweep entry from the wsa into\n"
		   "                        the user's working entry\n");	
	printf(" sweep list delete <sweep entry> Delete a sweep entry\n");
	printf(" sweep list read <sweep entry> Read the settings of a specific sweep entry\n");
	printf(" sweep list size        Shows the size of the sweep list\n");
	printf(" sweep entry new        Reset's the user's working sweep entry\n");
	printf(" sweep entry save <position>	Save the user's sweep entry into the sweep list\n");
	printf(" sweep resume           Resume Sweeping after stop\n");
	printf(" sweep start            Start to sweep through the entries set in the WSA\n");
	printf(" sweep stop             Stop the sweep process\n");
	
	printf("\n");

}


/**
 * Get input characters/string from the console and return the string 
 * all capitalized when the return key is pressed.
 *
 * @param pretext - A TRUE or FALSE flag to indicate if the default "enter a
 * command" text is to be printed.
 *
 * @return The characters inputted.
 */
char* get_input_cmd(uint8_t pretext)
{
	char ch;	// store user's option
	char *input_opt;
	int	cnt_ch = 0;					// count # of chars entered	

	// Initialized the option
	input_opt = (char *) malloc(sizeof(char) * MAX_STRING_LEN);
	if (input_opt == NULL) {
		printf("Error allocation memory. The program will be closed.\n");
		exit(1);
	}

	// Get command loop for string input terminated by "enter"
	if (pretext) 
		printf("\n> Enter a command (or 'h'): ");

	// Conver the command to upper case.... <- should do this?
	while (((ch = (char) toupper(getchar())) != EOF) && (ch != '\n'))
		input_opt[cnt_ch++] = (char) ch;
	input_opt[cnt_ch] = '\0';	// Terminate string with a null char

	return input_opt;
}


/**
 * Process CLI (only) commands in a text file.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param file_name - A char pointer for storing the file name string.
 *
 * @return Number of lines processed or negative number if failed.
 */
int16_t wsa_set_cli_command_file(struct wsa_device *dev, char *file_name) 
{
	int16_t result = 0;
	int16_t lines = 0;
	char *cmd_strs[MAX_FILE_LINES]; // store user's input words
	FILE *cmd_fptr;
	int i;

	if((cmd_fptr = fopen(file_name, "r")) == NULL) {
		printf("Error opening '%s'.\n", file_name);
		return WSA_ERR_FILEOPENFAILED;
	}

	// Allocate memory
	for (i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STRING_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	fclose(cmd_fptr);

	if (result < 0) {
		// free memory
		for (i = 0; i < MAX_FILE_LINES; i++)
			free(cmd_strs[i]);
		return WSA_ERR_FILEREADFAILED;
	}

	// Send each command line to WSA
	lines = result;
	for (i = 0; i < lines; i++) {
		result = process_cmd_string(dev, cmd_strs[i]);
		//Sleep(20); // delay the send a little bit
		//usleep(2000); // -> for gcc
		
		// If a bad command is detected, continue? Prefer not.
		if (result < 0) {
			printf("Error at line %d: '%s'.\n", i + 1, cmd_strs[i]);
			break;
		}
	}
	result = lines;

	// Free memory
	for (i = 0; i < MAX_FILE_LINES; i++)
		free(cmd_strs[i]);

	return result;
}


/**
 * Process any command (only) string.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cmd_str - A char pointer for a command string.
 *
 * @return 1 for quit, 0 for no error or a negative number if failed
 */
int16_t process_cmd_string(struct wsa_device *dev, char *cmd_str) 
{
	int16_t result = 0;
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words
	char temp_str[MAX_STRING_LEN];
	char *temp_ptr;
	uint8_t w = 0;	// an index
	int i;

	// Allocate memory
	for (i = 0; i < MAX_CMD_WORDS; i++)
		cmd_words[i] = (char*) malloc(MAX_STRING_LEN * sizeof(char));

	// clear up the words first
	for (i = 0; i < MAX_CMD_WORDS; i++)
		strcpy(cmd_words[i], "");
	
	// Get string command
	strcpy(temp_str, cmd_str);

	// Tokenized the string into words
	temp_ptr = strtok(temp_str, " \t\r\n");
	while (temp_ptr != NULL) {
		strcpy(cmd_words[w], temp_ptr);
		temp_ptr = strtok(NULL, " \t\r\n");
		w++;
	}
	
	// send cmd words to be processed
	result = process_cmd_words(dev, cmd_words, w);

	// Free the allocation
	for (i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	return result;
}

/**
 * Save data to a file with the file name format as:
 * [prefix] YYYY-MM-DD_HHMMSSmmm.[ext] if the prefix string and/or extension 
 * are given.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param prefix - A char pointer to a prefix string.
 * @param ext - A char pointer an extension string.
 *
 * @return 0 if successful, else a negative value.
 */
int16_t save_data_to_file(struct wsa_device *dev, char *prefix, char *ext)
{
	uint8_t context_is = 0;
	int16_t result = 0;
	int32_t samples_per_packet;
	int32_t field_indicator = 0;	
	int32_t packets_per_block;
	int32_t enable = 0;
	int32_t dec = 0;
	int32_t sweep_status = 0;
	int64_t freq = 0;
	int64_t start_frequency = 0;
	int64_t stop_frequency = 0;
	int64_t amplitude = 0;
	int8_t count = 0;
	int16_t acquisition_status;

	double receiver_temperature = 0;
	int32_t receiver_reference_point = 0;
	double receiver_rf_gain = 0;
	double receiver_if_gain = 0;
	long double receiver_frequency = 0;

	double digitizer_reference_level = 0;
	long double digitizer_bandwidth = 0;
	long double digitizer_rf_frequency_offset = 0;
	
	char file_name[MAX_STRING_LEN];
	FILE *iq_fptr;

	// to calculate run time
	TIME_HOLDER run_start_time;
	TIME_HOLDER run_end_time;
	double run_time_ms = 0;
	
	// to calculate data capture time
	TIME_HOLDER capture_start_time;
	TIME_HOLDER capture_end_time;
	double capture_time_ms = 0;


	// *****
	// Create parameters and buffers to store the raw data and context information
	// *****
	struct wsa_vrt_packet_header* header;
	struct wsa_vrt_packet_trailer* trailer;
	struct wsa_receiver_packet* receiver;
	struct wsa_digitizer_packet* digitizer;

	// *****
	// Create buffers to store the decoded I & Q from the raw data
	// *****
	int16_t *i_buffer;		// To store the integer I data
	int16_t *q_buffer;		// To store the integer Q data
	
	uint8_t expected_packet_order_indicator = 0;
	int32_t iq_pkt_count = 1;
	int32_t i;
	int j;

	// *****
	// Get parameters to stored in the file
	// TODO smarter way of saving header is to allow users to enable
	// which header info to include...
	// *****
	
	printf("Gathering WSA settings...");

	//determine if the another user is capturing data
	result = wsa_system_read_status(dev, &acquisition_status);
	if (result < 0) {
		doutf(DHIGH, "Error in wsa_system_read_status: %s\n", wsa_get_error_msg(result));
		return result;
	} else if (acquisition_status == 0) {
			return WSA_ERR_DATAACCESSDENIED; 
		}

	// Get sweep status
	result = wsa_get_sweep_status(dev, &sweep_status);
	if (result < 0) {
		doutf(DHIGH, "Error in wsa_get_sweep_status: %s\n", wsa_get_error_msg(result));
		return result;
	}

	if (sweep_status == 0) 
	{	
		// Flush content of the data socket
		result = wsa_flush_data(dev);
		if (result < 0)
		{
			doutf(DHIGH, "Error in wsa_flush_data: %s\n", wsa_get_error_msg(result));
			return result;
		}

		// Get samples per packet
		result = wsa_get_samples_per_packet(dev, &samples_per_packet);
		doutf(DMED, "In save_data_to_file: wsa_get_samples_per_packet returned %hd\n", result);
		if (result < 0)
		{
			doutf(DHIGH, "Error in wsa_capture_block: %s\n", wsa_get_error_msg(result));
			return result;
		}

		// Get packets per block
		result = wsa_get_packets_per_block(dev, &packets_per_block);
		doutf(DMED, "In save_data_to_file: wsa_get_packets_per_block returned %hd\n", result);	
		if (result < 0)
		{
			doutf(DHIGH, "Error in wsa_capture_block: %s\n", wsa_get_error_msg(result));
			return result;
		}

		// Get the centre frequency
		result = wsa_get_freq(dev, &freq);
		if (result < 0)
		{
			doutf(DHIGH, "Error in wsa_capture_block: %s\n", wsa_get_error_msg(result));
			return result;
		}	
	}
	// if sweep mode is enabled, samples per packet is set to the 
	// maximum value to hold iq packets with variant sizes
	else
	{
		samples_per_packet = dev->descr.max_sample_size;
		packets_per_block = 10;	// Find a solution to this
	}
	printf(" done.\n");
	
	// create file name in format "[prefix] YYYY-MM-DD_HHMMSSmmm.[ext]" in a 
	// folder called CAPTURES
	generate_file_name(file_name, prefix, ext);
	
	// create a new file for data capture
	if ((iq_fptr = fopen(file_name, "w")) == NULL) {
		printf("\nError creating the file \"%s\"!\n", file_name);
		
		return WSA_ERR_FILECREATEFAILED;
	}

	// Allocate header buffer space
	header = (struct wsa_vrt_packet_header*) malloc(sizeof(struct wsa_vrt_packet_header));
	if (header == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate header\n");
		fclose(iq_fptr); 
		
		return WSA_ERR_MALLOCFAILED;
	}

	// Allocate trailer buffer space
	trailer = (struct wsa_vrt_packet_trailer*) malloc(sizeof(struct wsa_vrt_packet_trailer));
	if (trailer == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate trailer\n");
		fclose(iq_fptr); 
		free(header);
		
		return WSA_ERR_MALLOCFAILED;
	}

	receiver = (struct wsa_receiver_packet*) malloc(sizeof(struct wsa_receiver_packet));
	if (receiver == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate trailer\n");
		fclose(iq_fptr); 
		free(trailer);
		free(header);
		
		return WSA_ERR_MALLOCFAILED;
	}

	digitizer = (struct wsa_digitizer_packet*) malloc(sizeof(struct wsa_digitizer_packet));
	if (receiver == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate trailer\n");
		fclose(iq_fptr); 
		free(receiver);
		free(trailer);
		free(header);
		
		return WSA_ERR_MALLOCFAILED;
	}

	// Allocate i buffer space
	i_buffer = (int16_t*) malloc(sizeof(int16_t) * samples_per_packet * BYTES_PER_VRT_WORD);
	if (i_buffer == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate i_buffer\n");
		fclose(iq_fptr); 
		free(receiver);
		free(digitizer);
		free(trailer);
		free(header);
		free(i_buffer);
		
		return WSA_ERR_MALLOCFAILED;
	}
	
	// Allocate q buffer space
	q_buffer = (int16_t*) malloc(sizeof(int16_t) * samples_per_packet * BYTES_PER_VRT_WORD);
	if (q_buffer == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate q_buffer\n");
		
		fclose(iq_fptr);
		free(digitizer);
		free(receiver);
		free(trailer);
		free(header);
		free(i_buffer);
		free(q_buffer);
		
		return WSA_ERR_MALLOCFAILED;
	}
	
	printf("Acquiring and saving data:\n ");
	
	// set capture block if not doing sweep
	if (sweep_status == 0) {
		result = wsa_capture_block(dev);
		if (result < 0)
		{
			doutf(DHIGH, "In save_data_to_file: wsa_capture_block returned %d\n", result);

			fclose(iq_fptr);
			free(digitizer);
			free(receiver);
			free(trailer);
			free(header);
			free(i_buffer);
			free(q_buffer);
			
			return result;
		}
	}

	// Get the start time
	get_current_time(&run_start_time);

	// loop to get all the packet per sweep
	for (i = 1; i <= packets_per_block; i++)
	{	
		// Get the start time
		get_current_time(&capture_start_time);

		result = wsa_read_vrt_packet(dev, header, trailer, receiver, digitizer, 
					i_buffer, q_buffer, samples_per_packet);
		if (result < 0)
		{
			break;
		}
		
		// Print only once the header line per file
		// TODO the 2nd condition is temporary for now until save
		// data format is determined. i == 1 cond'n might not applied then...
		if ((i == 1) && (header->stream_id == IF_DATA_STREAM_ID))
		{
			if (sweep_status)
			{
				fprintf(iq_fptr, "#%d, cf:%lld, ss:NA, sec:%d, pico:%lld\n", 
					1, 
					freq,
					header->time_stamp.sec, 
					header->time_stamp.psec);
			}
			else
			{
				fprintf(iq_fptr, "#%d, cf:%lld, ss:%ld, sec:%d, pico:%lld\n", 
					1, 
					freq, 
					packets_per_block * header->samples_per_packet, 
					header->time_stamp.sec, 
					header->time_stamp.psec);
			}
		}
		
		// get the end time of each data capture
		get_current_time(&capture_end_time);
		// sum it up
		capture_time_ms += get_time_difference(&capture_start_time, &capture_end_time);

		// TODO handle the packet order indicator for rec'r & dig'r as well
		if (header->stream_id == RECEIVER_STREAM_ID) 
		{
			/*fprintf(iq_fptr, "receiver Packet Found\n");
			field_indicator = receiver->indicator_field;

			if((field_indicator & 0xf0000000) == 0xc0000000) 
			{
				receiver_reference_point = receiver->reference_point;
				fprintf(iq_fptr, "Reference Point: %u\n", receiver_reference_point);			
			}

			if ((field_indicator & 0x0f000000) == 0x08000000) 
			{
				receiver_frequency = receiver->frequency;
				fprintf(iq_fptr, "Frequency: %.12E \n", receiver_frequency);		
			}

			if ((field_indicator & 0x00f00000) == 0x00800000) 
			{
				receiver_if_gain= receiver->gain_if;
				receiver_rf_gain= receiver->gain_rf;
				fprintf(iq_fptr, "Reference IF Gain: %E\n", receiver_if_gain);
				fprintf(iq_fptr, "Reference RF Gain: %E\n", receiver_rf_gain);
			} 	
			
			if ((field_indicator & 0x0f000000) == 0x04000000) 
			{				
				receiver_temperature = receiver->temperature;
				fprintf(iq_fptr, "Temperature: %u\n", receiver_temperature);			
			} */
			i--;
			continue;
		} 
		else if (header->stream_id == DIGITIZER_STREAM_ID) 
		{
			/*field_indicator = digitizer->indicator_field;

			fprintf(iq_fptr, "Digitizer Packet Found\n");
			if ((field_indicator & 0xf0000000) == 0xa0000000) 
			{
				digitizer_bandwidth = digitizer->bandwidth;
				fprintf(iq_fptr, "Bandwidth: %.12E \n", digitizer_bandwidth);
			} 			
			
			if (( field_indicator & 0x0f000000) == 0x05000000 || (field_indicator & 0x0f000000) == 0x04000000) 
			{
				digitizer_rf_frequency_offset = digitizer->rf_frequency_offset;
				fprintf(iq_fptr, "RF Frequency Offset: %.12E \n", digitizer_rf_frequency_offset);
			}

			if (( field_indicator & 0x0f000000) == 0x05000000 || (field_indicator & 0x0f000000) == 0x01000000) {
				digitizer_reference_level = digitizer->reference_level;
				fprintf(iq_fptr, "Reference Level: %f\n", digitizer_reference_level);		
			} */
			i--;
			continue;
		} 
		else if (header->stream_id == IF_DATA_STREAM_ID) 
		{
			if (i == 1)
			{
				expected_packet_order_indicator = header->packet_order_indicator;
			}

			if (header->packet_order_indicator != expected_packet_order_indicator)
			{				
				result = WSA_ERR_PACKETOUTOFORDER;
				break;
			}

			expected_packet_order_indicator++;	
			if (expected_packet_order_indicator > MAX_PACKET_ORDER_INDICATOR)
				expected_packet_order_indicator = 0;

			for (j = 0; j < header->samples_per_packet; j++)
			{			
				fprintf(iq_fptr, "%d,%d\n", i_buffer[j], q_buffer[j]);
				//printf("%d,%d\n", i_buffer[j], q_buffer[j]);
			}
		
			if (!(iq_pkt_count % 10))
			{
				printf("saved packet #%u\n", iq_pkt_count);
				if (iq_pkt_count != packets_per_block)
					printf(" ");
			}
			else 
				printf(".");

			iq_pkt_count++;
		}
	}

	if (result >= 0) {
		// get the total run end time
		get_current_time(&run_end_time);
		run_time_ms = get_time_difference(&run_start_time, &run_end_time);
		
		// a little tweak to make print out readable
		if (packets_per_block % 10)
			printf("\n");
		printf("done.\n\n");
		
		//printf("(Data capture time: %.3f sec; Total run time: %.3f sec)\n", 
		//	capture_time_ms,
		//	run_time_ms);
	}
		
	fclose(iq_fptr);
	free(digitizer);
	free(receiver);
	free(trailer);
	free(header);
	free(i_buffer);
	free(q_buffer);
	
	return result;
}

/**
 * Process command words.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cmd_words - A char pointer to char array for storing command words.
 * @param num_words - Number of words within the command.
 *
 * @return 1 if 'q'uit is set, 0 for no error.
 */
int8_t process_cmd_words(struct wsa_device *dev, char *cmd_words[], 
					int16_t num_words)
{
	int16_t result = 0;			// result returned from a function
	int64_t freq = 0;
	float fshift = 0;
	int int_result = 0;
	int8_t user_quit = FALSE;	// determine if user has entered 'q' command
	char msg[100];
	int i;
	long temp_number;
	double temp_double;
	char* strtok_result;
	int32_t rate;
	int32_t if_gain_value;
	int32_t dwell_seconds_value = 15;
	int32_t dwell_miliseconds_value = 16;
	int32_t samples_per_packet;
	int32_t packets_per_block;
	int64_t start_frequency;
	int64_t stop_frequency;
	int64_t amplitude;
	int16_t acquisition_status;
	uint8_t valid;
	//DIR *temp;

	strcpy(msg,"");

	//*****
	// Handle GET commands
	//*****

	if (strcmp(cmd_words[0], "GET") == 0) {
		if (strcmp(cmd_words[1], "ANT") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum antenna ports: 2\n");
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum antenna ports: 1\n");
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			
			result = wsa_get_antenna(dev, &int_result);
			if (result >= 0)
				printf("Currently using antenna port: %d\n", int_result);
		} // end get ANT 

		else if (strcmp(cmd_words[1], "BPF") == 0) {			
			if(num_words > 2)
				printf("Extra parameters ignored (max/min not available)!\n");
			
			result = wsa_get_bpf_mode(dev, &int_result);
			if (result >= 0) {
				printf("RFE's preselect BPF state: ");
				if (int_result) printf("On\n");
				else if (!int_result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get BPF

		else if (strcmp(cmd_words[1], "DEC") == 0) {
			int32_t rate = 0;

			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum decimation rate: %d\n", 
						dev->descr.max_decimation);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum decimation rate: %d\n", 
						dev->descr.min_decimation);
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}

			result = wsa_get_decimation(dev, &rate);
			if (result >= 0)
				printf("The current decimation rate: %d\n", rate);
		} // end get decimation rate

		else if (strcmp(cmd_words[1], "FREQ") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum frequency: %0.2f MHz\n", 
						(float) dev->descr.max_tune_freq / MHZ);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum frequency: %0.2f MHz\n", 
						(float) dev->descr.min_tune_freq / MHZ);
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}

			result = wsa_get_freq(dev, &freq);
	
			if (result >= 0)
				printf("Current centre frequency: %0.3f MHz\n", 
					(float) freq/MHZ);
		} // end get FREQ

		else if (strcmp(cmd_words[1], "FSHIFT") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum shift: %0.2f MHz\n", 
						(float) dev->descr.inst_bw / MHZ);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum frequency: %0.2f MHz\n", 
					(float) dev->descr.inst_bw / MHZ * (-1.0) - 0.1);
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}

			result = wsa_get_freq_shift(dev, &fshift);
			if (result >= 0)
				printf("Current frequency shift: %0.8f MHz\n", 
					(float) fshift / MHZ);
		} // end get FSHIFT

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				enum wsa_gain gain;
				if (strcmp(cmd_words[3], "") != 0) {
					if (strcmp(cmd_words[3], "MAX") == 0) {
						printf("Maximum RF gain: HIGH\n");
						return 0;
					}
					else if (strcmp(cmd_words[3], "MIN") == 0) {
						printf("Minimum RF gain: VLOW\n");
						return 0;
					}
					else
						printf("Did you mean \"min\" or \"max\"?\n");
				}

				result = wsa_get_gain_rf(dev, &gain);
				if (result >= 0) {
					char temp[10];
					gain_rf_to_str(gain, &temp[0]);
					printf("Current RF gain: %s\n", temp);
				}
			}  // end get GAIN RF

			else if (strcmp(cmd_words[2], "IF") == 0) {
				int_result = -1000;
				if (strcmp(cmd_words[3], "") != 0) {
					if (strcmp(cmd_words[3], "MAX") == 0) {
						printf("Maximum IF gain: %d dB\n", 
							dev->descr.max_if_gain);
						return 0;
					}
					else if (strcmp(cmd_words[3], "MIN") == 0) {
						printf("Minimum IF gain: %d dB\n", 
							dev->descr.min_if_gain);
						return 0;
					}
					else
						printf("Did you mean \"min\" or \"max\"?\n");
				}
				
				result = wsa_get_gain_if (dev, &int_result);
				if (result >= 0)
					printf("Current IF gain: %d dB\n", int_result);
			} // end get GAIN IF

			else 
				printf("Incorrect get GAIN. Specify RF or IF or see 'h'.\n");
		} // end get GAIN

		else if (strcmp(cmd_words[1], "PPB") == 0) {
			if (num_words > 2) {
				printf("Extra parameters ignored (max/min not available)!\n");
			}

			result = wsa_get_packets_per_block(dev, &packets_per_block);
			if (result >= 0) {
				printf("Current packets per block: %d\n", packets_per_block);
			}
		} // end get PPB

		else if (strcmp(cmd_words[1], "SPP") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum samples per packet: %hu\n", 
						WSA4000_MAX_SAMPLES_PER_PACKET);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum samples per packet: %hu\n", 
						WSA4000_MIN_SAMPLES_PER_PACKET);
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else {
				result = wsa_get_samples_per_packet(dev, &samples_per_packet);
				if (result >= 0) {
					printf("Current samples per packet: %d\n", 
						samples_per_packet);
				}
			}
		} // end get SPP
		else if (strcmp(cmd_words[1], "READ") == 0) {
			 if (strcmp(cmd_words[2], "ACCESS") == 0) {
				 result = wsa_system_request_read_access(dev,&acquisition_status);
				 if (acquisition_status == 1) {
					 printf("Read access obtained. \n");
				 } else 
					 printf("Read access denied. \n");
			 }
		}//end get READ ACCESS

		else if (strcmp(cmd_words[1], "TRIGGER") == 0) {
			if (strcmp(cmd_words[2], "ENABLE") == 0) {
				result = wsa_get_trigger_enable(dev, &int_result);
				if (result >= 0) {
					printf("Trigger mode: ");
					if (int_result == 1) {
						printf("On\n");
					}
					else if (int_result == 0) {
						printf("Off\n");
					}
					else {
						printf("Unknown state\n");
					}
				}
			}
			else if (strcmp(cmd_words[2], "LEVEL") == 0) {
				result = wsa_get_trigger_level(dev, &start_frequency, &stop_frequency, &amplitude);
				if (result >= 0) {
					printf("Trigger configuration:\n");
					printf("   Start frequency: %f MHz\n", (float) (start_frequency / MHZ));
					printf("   Stop frequency: %f MHz\n", (float) (stop_frequency / MHZ));
					printf("   Amplitude: %lld dBm\n", amplitude);
				}
			}
			else {
				printf("Usage: 'get trigger <level | enable>'");
			}
		} // end get TRIGGER
		else if (strcmp(cmd_words[1], "SWEEP") == 0) {
			if (strcmp(cmd_words[2], "STATUS") == 0) {
				result = wsa_get_sweep_status(dev, &int_result);
				if (result >= 0) {
					if (int_result)
						printf("Sweep mode is on.\n");
					else
						printf("Sweep mode is off.\n");
				}
			} 
			else if (strcmp(cmd_words[2], "LIST") == 0) {
				 if (strcmp(cmd_words[3], "SIZE") == 0) {
					 result = wsa_get_sweep_list_size(dev, &int_result);
					 printf("The Sweep list size is %d \n",int_result);
				 }
			}
			else if (strcmp(cmd_words[2], "ENTRY") == 0) {
				print_sweep_entry(dev);
			}
			else {
				printf("Invalid 'get sweep'. Try 'h'.\n");
			}
		} // end get SWEEP
		else {
			printf("Invalid 'get'. Try 'h'.\n");
		}
	} // end GET
	
	else if (strcmp(cmd_words[0], "RUN") == 0) {
		if (strcmp(cmd_words[1], "CMDF") == 0) {
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the syntax type and file name.\n");
			else {
				if (strcmp(cmd_words[3], "") == 0) 
					printf("Missing the file name.\n");
				else {
					// TODO fix this declaration
					char *file_name = cmd_words[3];
					if (num_words > 3) {
						for (i = 4; i < num_words; i++) {
							strcat(file_name, " ");
							strcat(file_name, cmd_words[i]);
						}
					}

					if (strcmp(cmd_words[2], "CLI") == 0) 
						result = wsa_set_cli_command_file(dev, file_name);
					else if (strcmp(cmd_words[2], "SCPI") == 0) 
						result = wsa_set_command_file(dev, file_name);
					else
						printf("Use 'cli' or scpi' for syntax type.\n");
				}
			}
		} // end run CMDF
		else {
			printf("'run cmdf <scpi | cli> <file name>'?");
		}
	} // end RUN

	//*****
	// Handle SET commands
	//*****
	else if (strcmp(cmd_words[0], "SET") == 0) {
		if (strcmp(cmd_words[1], "ANT") == 0) {
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the antenna port value. See 'h'.\n");
			else
				result = wsa_set_antenna(dev, atoi(cmd_words[2]));
				if (result == WSA_ERR_INVANTENNAPORT)
					sprintf(msg, "\n\t- Valid ports: 1 to %d.", 
					WSA_RFE0560_MAX_ANT_PORT);
		} // end set ANT

		else if (strcmp(cmd_words[1], "BPF") == 0) {
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_set_bpf_mode(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_set_bpf_mode(dev, 0);
			else 
				printf("Use 'on' or 'off' mode.\n");
		} // end set BPF

		else if (strcmp(cmd_words[1], "DEC") == 0) {
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the decimation rate. See 'h'.\n");
			
			rate = (int32_t) atof(cmd_words[2]);

			result = wsa_set_decimation(dev, rate);
			if (result == WSA_ERR_INVDECIMATIONRATE)
				sprintf(msg, "\n\t- Valid range: %d to %d.",	// TODO #s
				dev->descr.min_decimation, dev->descr.max_decimation);
		} // end set decimation rate

		else if (strcmp(cmd_words[1], "FREQ") == 0) {
			if (strcmp(cmd_words[2], "") == 0) {
				printf("Missing the frequency value. See 'h'.\n");
			}
			else {
				freq = (int64_t) (atof(cmd_words[2]) * MHZ);
				result = wsa_set_freq(dev, freq);
				if (result == WSA_ERR_FREQOUTOFBOUND)
					sprintf(msg, "\n\t- Valid range: %0.2lf to %0.2lf MHz.",
						(double) dev->descr.min_tune_freq / MHZ, 
						(double) dev->descr.max_tune_freq / MHZ);
			}
		} // end set FREQ

		else if (strcmp(cmd_words[1], "FSHIFT") == 0) {
			if (strcmp(cmd_words[2], "") == 0) {
				printf("Missing the frequency value. See 'h'.\n");
			}
			else {
				fshift = (float) (atof(cmd_words[2]) * MHZ);
				result = wsa_set_freq_shift(dev, fshift);
				if (result == WSA_ERR_FREQOUTOFBOUND)
					sprintf(msg, "\n\t- Valid range: %0.2f to %0.2f MHz.",
						0.0, (float) dev->descr.inst_bw / MHZ);
			}
		} // end set FSHIFT

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				enum wsa_gain gain = (enum wsa_gain) NULL;
				valid = TRUE;

				// Convert to wsa_gain type
				if (strstr(cmd_words[3], "HIGH") != NULL)
					gain = WSA_GAIN_HIGH;
				else if (strstr(cmd_words[3], "MED") != NULL)
					gain = WSA_GAIN_MED;
				else if (strstr(cmd_words[3], "VLOW") != NULL)
					gain = WSA_GAIN_VLOW;
				else if (strstr(cmd_words[3], "LOW") != NULL)
					gain = WSA_GAIN_LOW;
				else if (strcmp(cmd_words[3], "") == 0) {
					printf("Missing the gain paramter. See 'h'.\n");
					valid = FALSE;
				}
				else { 
					printf("Invalid RF gain setting. See 'h'.\n");
					valid = FALSE;
				}

				if (valid)
					result = wsa_set_gain_rf(dev, gain);
			} // end set GAIN RF

			else if (strcmp(cmd_words[2], "IF") == 0) {
				if (strcmp(cmd_words[3], "") == 0) {
					printf("Missing the gain in dB value. See 'h'.\n");
				}
				else if (!to_int(cmd_words[3], &temp_number)) {
					if_gain_value = (int32_t) temp_number;
					result = wsa_set_gain_if(dev, if_gain_value);
					if (result == WSA_ERR_INVIFGAIN) {
						sprintf(msg, "\n\t- Valid range: %d to %d dB.", 
							dev->descr.min_if_gain, dev->descr.max_if_gain);
					}
				}
				else {
					printf("The IF gain value must be an integer. See 'h'.\n");
				}
			} // end set GAIN IF
			
			else {
				printf("Incorrect set GAIN. Specify RF or IF. See 'h'.\n");
			}
		} // end set GAIN

		else if (strcmp(cmd_words[1], "PPB") == 0) {
			if (num_words < 3) {
				printf("Missing the packets per block value. See 'h'.\n");
			}
			else {
				result = to_int(cmd_words[2], &temp_number);

				if (!result) {
					
					if (temp_number < WSA4000_MIN_PACKETS_PER_BLOCK ||
						temp_number > WSA4000_MAX_PACKETS_PER_BLOCK) {
						sprintf(msg, "\n\t- The integer is out of range.");

						result = WSA_ERR_INVNUMBER;
					}
					else {
						packets_per_block = (int32_t) temp_number;
						result = wsa_set_packets_per_block(dev,	
									packets_per_block);
					}
				}
			}
		} // end set PPB

		else if (strcmp(cmd_words[1], "SPP") == 0) {
			if (num_words < 3) {
				printf("Missing the samples per packet value. See 'h'.\n");
			}
			else {
				result = to_int(cmd_words[2], &temp_number);

				if (!result) {
					if (temp_number < WSA4000_MIN_SAMPLES_PER_PACKET ||
						temp_number > WSA4000_MAX_SAMPLES_PER_PACKET) {
						sprintf(msg, "\n\t- Valid range: %hu to %hu.",
							WSA4000_MIN_SAMPLES_PER_PACKET,
							WSA4000_MAX_SAMPLES_PER_PACKET);

						result = WSA_ERR_INVSAMPLESIZE;
					}
					else {
						samples_per_packet = (int32_t) temp_number;
						result = wsa_set_samples_per_packet(dev, 
									samples_per_packet);
					}
				}
			}
		} // end set SPP
		
		else if (strcmp(cmd_words[1], "TRIGGER") == 0) {
			if (strcmp(cmd_words[2], "ENABLE") == 0) {
				if (strcmp(cmd_words[3], "ON") == 0) {
					result = wsa_set_trigger_enable(dev, 1);
				}
				else if (strcmp(cmd_words[3], "OFF") == 0) {
					result = wsa_set_trigger_enable(dev, 0);
				}
				else {
					printf("Usage: 'set trigger enable <on | off>'\n");
				}
			}
			else if (strcmp(cmd_words[2], "LEVEL") == 0) {
				if (num_words < 4) {
					printf("Usage: 'set trigger level <start>,<stop>,<amplitude>'\n");
				}
				else {
					strtok_result = strtok(cmd_words[3], ",");
					if (to_double(strtok_result, &temp_double) < 0) {
						printf("Start frequency must be a valid number\n");
					}
					else {
						start_frequency = (int64_t) (temp_double * MHZ);
						
						strtok_result = strtok(NULL, ",");
						if (to_double(strtok_result, &temp_double) < 0) {
							printf("Stop frequency must be a valid number\n");
						}
						else {
							stop_frequency = (int64_t) (temp_double * MHZ);
							
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0) {
								printf("Amplitude must be a valid number\n");
							}
							else {
								amplitude = (int64_t) temp_double;
								
								result = wsa_set_trigger_level(dev, start_frequency, stop_frequency, amplitude);
							}
						}
					}
				}
			}
			else {
				printf("Usage: 'set trigger level <start,stop,amplitude>'\n"
				       "    or 'set trigger enable <on | off>'\n");
			}
		}// end set TRIGGER
		
		else if (strcmp(cmd_words[1], "SWEEP") == 0) {
			if (strcmp(cmd_words[2], "ENTRY") == 0) {
				if (strcmp(cmd_words[3], "DWELL") == 0) {
					if (num_words < 5){	 	 
						printf("Usage: 'set sweep entry dwell <seconds>,<milliseconds>'\n");
					}
					else {
						strtok_result = strtok(cmd_words[4], ",");
						if (to_double(strtok_result, &temp_double) < 0) {
							printf("Dwell seconds value must be a valid number\n");
						}
						else {
							 dwell_seconds_value = (int32_t) (temp_double);
							
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0) {
								printf("Dwell milliseconds value must be a valid number\n");
							}
							else {
								dwell_miliseconds_value = (int32_t) (temp_double);
						
								result = wsa_set_sweep_dwell(dev, dwell_seconds_value, dwell_miliseconds_value );
							}
						}
					}
				}//end set sweep dwell

				else if (strcmp(cmd_words[3], "ANT") == 0) {
					if (strcmp(cmd_words[4], "") == 0) 
						printf("Missing the antenna port value. See 'h'.\n");
					else
						result = wsa_set_sweep_antenna(dev, atoi(cmd_words[4]));
					
					if (result == WSA_ERR_INVANTENNAPORT)
						sprintf(msg, "\n\t- Valid ports: 1 to %d.", 
								WSA_RFE0560_MAX_ANT_PORT);
				}
				
				//end set ANT
				else if (strcmp(cmd_words[3], "GAIN") == 0) {
					if (strcmp(cmd_words[4], "RF") == 0) {
						enum wsa_gain gain = (enum wsa_gain) NULL;
						valid = TRUE;

						// Convert to wsa_gain type
						if (strstr(cmd_words[5], "HIGH") != NULL)
							gain = WSA_GAIN_HIGH;
						else if (strstr(cmd_words[5], "MED") != NULL)
							gain = WSA_GAIN_MED;
						else if (strstr(cmd_words[5], "VLOW") != NULL)
							gain = WSA_GAIN_VLOW;
						else if (strstr(cmd_words[5], "LOW") != NULL)
							gain = WSA_GAIN_LOW;
						else if (strcmp(cmd_words[5], "") == 0) {
							printf("Missing the gain paramter. See 'h'.\n");
							valid = FALSE;
						}
						else { 
							printf("Invalid RF gain setting. See 'h'.\n");
							valid = FALSE;
						}

						if (valid)
							result = wsa_set_sweep_gain_rf(dev, gain);
					} // end set GAIN RF

					else if (strcmp(cmd_words[4], "IF") == 0) {
						if (strcmp(cmd_words[5], "") == 0) {
							printf("Missing the gain in dB value. See 'h'.\n");
						}
						else if (!to_int(cmd_words[5], &temp_number)) {
							if_gain_value = (int32_t) temp_number;
							result = wsa_set_sweep_gain_if(dev, if_gain_value);
							if (result == WSA_ERR_INVIFGAIN) {
								sprintf(msg, "\n\t- Valid range: %d to %d dB.", 
									dev->descr.min_if_gain, dev->descr.max_if_gain);
							}
						}
						else {
							printf("The IF gain value must be an integer. See 'h'.\n");
						}
					} // end set GAIN IF
				
					else {
						printf("Incorrect set GAIN. Specify RF or IF. See 'h'.\n");
					}
				} // end set GAIN
			
				else if (strcmp(cmd_words[3], "DEC") == 0) {
					if (strcmp(cmd_words[4], "") == 0) 
						printf("Missing the decimation rate. See 'h'.\n");
					
					rate = (int32_t) atof(cmd_words[4]);
					result = wsa_set_sweep_decimation(dev, rate);
					if (result == WSA_ERR_INVDECIMATIONRATE)
						sprintf(msg, "\n\t- Valid range: %d to %d.",	// TODO #s
								dev->descr.min_decimation, dev->descr.max_decimation);
				} // end set decimation rate

				else if (strcmp(cmd_words[3], "TRIGGER") == 0) {
					if (strcmp(cmd_words[4], "ENABLE") == 0) {
						if (strcmp(cmd_words[5], "OFF") == 0) {
							result = wsa_set_sweep_trigger_type(dev, 0);
						} else if (strcmp(cmd_words[5], "ON") == 0) {
							result = wsa_set_sweep_trigger_type(dev, 1);
						} else {
							printf("Invalid 'set sweep trigger type'. See 'h'. \n");
						}
					} else if (strcmp(cmd_words[4], "LEVEL") == 0) {
						if (num_words < 5) {
							printf("Usage: 'set sweep trigger level <start>,<stop>,<amplitude>'\n");
						}
						else {
							strtok_result = strtok(cmd_words[5], ",");
							if (to_double(strtok_result, &temp_double) < 0) {
								printf("Start frequency must be a valid number\n");
							}
							else {
								start_frequency = (int64_t) (temp_double);
								
								strtok_result = strtok(NULL, ",");
								if (to_double(strtok_result, &temp_double) < 0) {
									printf("Stop frequency must be a valid number\n");
								}
								else {
									stop_frequency = (int64_t) (temp_double);
									
									strtok_result = strtok(NULL, ",");
									if (to_double(strtok_result, &temp_double) < 0) {
										printf("Amplitude must be a valid number\n");
									}
									else {
										amplitude = (int64_t) temp_double;
										
										result = wsa_set_sweep_trigger_level(dev, start_frequency, stop_frequency, amplitude);
									}
								}
							}
						}
					} else {
						printf("Invalid 'set sweep trigger level'. See 'h'. \n");
					}// end set trigger level
				}// end set trigger

				else if (strcmp(cmd_words[3], "FREQ") == 0) {
					if (strcmp(cmd_words[4], "") == 0) {
						printf("Missing the frequency value. See 'h'.\n");
					}
					else {
						start_frequency = (int64_t) (atof(cmd_words[4]) * MHZ);
						stop_frequency = (int64_t) (atof(cmd_words[5]) * MHZ);
						result = wsa_set_sweep_freq(dev, start_frequency, stop_frequency);
						if (result == WSA_ERR_FREQOUTOFBOUND)
							sprintf(msg, "\n\t- Valid range: %0.2lf to %0.2lf MHz.",
								(double) dev->descr.min_tune_freq / MHZ, 
								(double) dev->descr.max_tune_freq / MHZ);
					}
				} // end set FREQ

				else if (strcmp(cmd_words[3], "FSHIFT") == 0) {
					if (strcmp(cmd_words[4], "") == 0) {
						printf("Missing the frequency value. See 'h'.\n");
					}
					else {
						fshift = (float) (atof(cmd_words[4]) * MHZ);
						result = wsa_set_sweep_freq_shift(dev, fshift);
						if (result == WSA_ERR_FREQOUTOFBOUND)
							sprintf(msg, "\n\t- Valid range: %0.2f to %0.2f MHz.",
								0.0, (float) dev->descr.inst_bw / MHZ);
					}
				} // end set FSHIFT

				else if (strcmp(cmd_words[3], "FSTEP") == 0) {
					if (strcmp(cmd_words[4], "") == 0) {
						printf("Missing the frequency step value. See 'h'.\n");
					}
					else {
						freq = (int64_t) (atof(cmd_words[4]) * MHZ);
						result = wsa_set_sweep_freq_step(dev, freq);
						if (result == WSA_ERR_FREQOUTOFBOUND)
							sprintf(msg, "\n\t- Valid range: %0.2lf to %0.2lf MHz.",
								(double) dev->descr.min_tune_freq / MHZ, 
								(double) dev->descr.max_tune_freq / MHZ);
					}
				} // end set FSTEP
				
				else if (strcmp(cmd_words[3], "PPB") == 0) {
					if (num_words < 4) {
						printf("Missing the packets per block value. See 'h'.\n");
					}
					else {
						result = to_int(cmd_words[4], &temp_number);

						if (!result) {
							
							if (temp_number < WSA4000_MIN_PACKETS_PER_BLOCK ||
								temp_number > WSA4000_MAX_PACKETS_PER_BLOCK) {
								sprintf(msg, "\n\t- The integer is out of range.");

								result = WSA_ERR_INVNUMBER;
							}
							else {
								packets_per_block = (int32_t) temp_number;
								result = wsa_set_sweep_packets_per_block(dev,	
											packets_per_block);
							}
						}
					}
				} // end set PPB

				else if (strcmp(cmd_words[3], "SPP") == 0) {
					if (num_words < 4) {
						printf("Missing the samples per packet value. See 'h'.\n");
					}
					else {
						result = to_int(cmd_words[4], &temp_number);

						if (!result) {
							if (temp_number < WSA4000_MIN_SAMPLES_PER_PACKET ||
								temp_number > WSA4000_MAX_SAMPLES_PER_PACKET) {
								sprintf(msg, "\n\t- Valid range: %hu to %hu.",
									WSA4000_MIN_SAMPLES_PER_PACKET,
									WSA4000_MAX_SAMPLES_PER_PACKET);

								result = WSA_ERR_INVSAMPLESIZE;
							}
							else {
								samples_per_packet = (int32_t) temp_number;
								result = wsa_set_sweep_samples_per_packet(dev, 
											samples_per_packet);
							}
						}
					}
				} // end set SPP
			} // end set sweep ENTRY
			else {
				printf("Invalid 'set sweep entry'. See 'h'.\n");
			}
		}	// end set SWEEP
		
		else {
			printf("Invalid 'set'. See 'h'.\n");
		}
	} // end SET

	//*****
	// Handle SWEEP commands
	//*****
	else if  (strcmp(cmd_words[0], "SWEEP") == 0) {
		if (strcmp(cmd_words[1], "LIST") == 0) {
			if (strcmp(cmd_words[2], "DELETE") == 0) {
				if (num_words < 4) {
					printf("Missing the position of the entry. See 'h'.\n");
				} 
				else {
					result = to_int(cmd_words[3], &temp_number);
					int_result = (uint32_t) temp_number;
					result = wsa_sweep_list_delete(dev,temp_number);
				}
			} 
			else if (strcmp(cmd_words[2], "READ") == 0) {
				if (num_words < 4) {
					printf("Missing the position of the entry. See 'h'.\n");
				}
				else {
					result = to_int(cmd_words[3], &temp_number);
					int_result = (uint32_t) temp_number;
					print_sweep_entry_information(dev,temp_number);
				}
			}	
			else if(strcmp(cmd_words[2], "COPY") == 0) {
				if (num_words < 3) {
					printf("Missing the position of the entry. See 'h'.\n");
				}
				else {
					result = to_int(cmd_words[3], &temp_number);
					int_result = (uint32_t) temp_number;
					result = wsa_sweep_list_copy(dev,temp_number);
					// TODO catch result error here?
				}
			}
			else {
				printf("Invalid 'Sweep List' Command. See 'h'.\n");
			}
		} 
		else if  (strcmp(cmd_words[1], "START") == 0) {
			result = wsa_sweep_start(dev);
			// TODO catch result error here?			
		} 
		else if (strcmp(cmd_words[1], "STOP") == 0){
				//check if the wsa is not sweeping
			result = wsa_get_sweep_status(dev, &int_result);
			if (int_result == 0) {
				printf("Sweep mode is already disabled \n");
			} else {
				result =  wsa_sweep_stop(dev);
			}
			// TODO catch result error here?
		} 
		else if (strcmp(cmd_words[1], "RESUME") == 0){
			result =  wsa_sweep_resume(dev);
			// TODO catch result error here?
		}
		else if (strcmp(cmd_words[1], "ENTRY") == 0) {
			if (strcmp(cmd_words[2], "NEW") == 0) {
				result = wsa_sweep_entry_new(dev);
			}
			else if (strcmp(cmd_words[2], "SAVE") == 0) {
				if (num_words < 3) {
					printf("Missing the position of the entry. See 'h'.\n");
				}
				else {
					result = to_int(cmd_words[3], &temp_number);
					int_result = (uint32_t) temp_number;
					result = wsa_sweep_entry_save(dev,temp_number);
				}
			} 
			else {
				printf("Invalid 'Sweep entry' Command. See 'h'.\n");
			}
		}	// end ENTRY
		else {
			printf("Invalid 'Sweep' Command. See 'h'.\n");
		}
	} // end SWEEP

	//*****
	// Handle non-get/set commands
	//*****
	else {	
		if (strcmp(cmd_words[0], "DIR") == 0) {
			if(num_words > 1)
				printf("Extra parameters ignored!\n");

			print_captures_directory();
		}
		
		else if (strlen(cmd_words[0]) == 1 && strspn(cmd_words[0], "H?") > 0) {
			print_cli_menu(dev);
		} // end print help

		else if (strcmp(cmd_words[0], "O") == 0) {
			open_captures_directory();
		}  // end Open directory

		else if (strcmp(cmd_words[0], "SAVE") == 0) {
			// fix the declaration here?
			char prefix[200];
			char ext[10];
			int n = 1;
			char *temp;

			strcpy(prefix, "");
			strcpy(ext, "csv");

			// Get the [name] &/or [ext:<type>] string if there exists one
			while (strcmp(cmd_words[n], "") != 0) {
				unsigned int j;
				if (strstr(cmd_words[n], "EXT:") != NULL) {
					temp = strchr(cmd_words[n], ':');
					strcpy(ext, strtok(temp, ":"));
					
					// convert to lower case
					for (j = 0; j < strlen(ext); j++)
						ext[j] = (char) tolower(ext[j]);
					//break; // break when reached the ext: line?			
				}
				else {
					// convert to lower case after the first letter
					for (j = 1; j < strlen(cmd_words[n]); j++)
						cmd_words[n][j] = (char) tolower(cmd_words[n][j]);
					strcat(prefix, cmd_words[n]);
					strcat(prefix, " ");
				}

				n++;
			}

			result = save_data_to_file(dev, prefix, ext);
		} // end save data

		// User wants to run away...
		else if (strcmp(cmd_words[0], "Q") == 0) {
			user_quit = TRUE;
		} // end quit

		// Keep going if nothing is entered
		else if (strcmp(cmd_words[0], "") == 0) {
			// Do nothing
		}

		else 
			user_quit = -1;
	} // End handling non get/set cmds.

	// Print out the errors
	if (result < 0) {
		printf("ERROR %d: %s. %s\n", result, wsa_get_err_msg(result), msg);
		if (result == WSA_ERR_QUERYNORESP) {
			printf("Possibly due to loss of Ethernet connection.\n\n");
			user_quit = TRUE;
		}
	}

	return user_quit;
}

/**
 * Setup WSA device variables, start the WSA connection and 
 *
 * @param wsa_addr - A char pointer to the IP address of the WSA
 *
 * @return 0 when successful or a negative number when error occurred
 */
int16_t do_wsa(const char *wsa_addr)
{	
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_device *dev;
	char intf_str[200];			// store the interface method string
	char *in_buf;
	int16_t result = 0;			// result returned from a function
	char temp_str[MAX_STRING_LEN];


	// Create the TCPIP interface method string
	sprintf(intf_str, "TCPIP::%s", wsa_addr);
	//sprintf(intf_str, "TCPIP::%s::%d,%d", wsa_addr, CTRL_PORT, DATA_PORT);

	// Start the WSA connection
	dev = &wsa_dev;
	result = wsa_open(dev, intf_str);
	if (result < 0) {
		doutf(DMED, "Error WSA_ERR_OPENFAILED: %s.\n", 
			wsa_get_err_msg(result));//WSA_ERR_OPENFAILED));
		return result;//WSA_ERR_OPENFAILED;
	}

	// print out the current stat
	print_wsa_stat(dev);

	// Start the control loop
	do {
		in_buf = get_input_cmd(TRUE);
		// Get input string command
		strcpy(temp_str, in_buf);
		
		// send cmd string to be processed
		result = process_cmd_string(dev, temp_str);
		if (result < 0) 
			printf("Command '%s' not recognized.  See 'h'.\n", temp_str);
	} while (result != 1);

	free(in_buf);

	// finish so close the connection
	wsa_close(dev);

	return 0;
}


/**
 * Start the CLI tool. First get a valid IP address from users, verify 
 * and start the WSA connection.
 * @return 0 if successful
 */
int16_t start_cli(void) 
{
	uint8_t user_quit = FALSE;		// determine if user exits the CLI tool
	time_t dateStamp = time(NULL);	// use for display in the start of CLI
	char in_str[MAX_STRING_LEN];	// store user's input string
	char *in_buf;
	const char *wsa_addr;			// store the desired WSA IP address
	int16_t result = 0;				// result returned from a function

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA Command Line Interface Tool_____\n");
	printf("\t\t\t(Version: %s)\n\n", CLI_VERSION);

	do {
		//*****
		// Ask user to enter an IP address
		//*****
		printf("\n> Enter the WSA4000's IP (or type 'h'): ");
		in_buf = get_input_cmd(FALSE);
		strcpy(in_str, in_buf);

		// prevent crashing b/c of strtok in the next line
		if (strtok(in_str, " \t\r\n") == NULL) continue;
		
		// remove spaces or tabs in string
		strcpy(in_str, strtok(in_str, " \t\r\n")); 

		// do nothing if nothing is entered
		if (strcmp(in_str, "") == 0) continue;

		// User wants to run away...
		if (strcmp(in_str, "Q") == 0) 
			return 0;

		// User asked for help
		if (strcmp(in_str, "H") == 0 || strcmp(in_str, "?") == 0) {
			printf("Enter an IP address or host name.\n");
			/*printf("Enter <IP address | host name>[:<command port>,"
				"<data port>]\nCommand and data ports are optional. By "
				"default, they are 37001 and 37000 respectively. Provide ports"
				" only if those ports will be routed to the default ports."
				"Ex: www.yournet.com\n    www.yournet.com:7000,7001\n"
				"    192.168.1.2\n    192.168.1.2:7234,72348\n");
			printf("string, or 'q' to quit.\n");*/
			continue;
		}

		// User has enter an address so verify first
		else if (strchr(in_str, '.') != 0) {
			if (strchr(in_str, ':') != 0) {
				// TODO split up the ports & the address
				printf("Not yet support routed ports. Enter an address"
					" without the ports for now.\n");
				continue;
				//// Make sure both ports work
				//if (wsa_check_addrandport(in_str, ctrl) >= 0) {
				//	if (wsa_check_addrandport(in_str, data) >= 0)
				//	wsa_addr = in_str;
				//}
				//else {
				//	printf("\nInvalid address. Try again or 'h'.\n");
				//	continue;
				//}
			}
			else {
				if (wsa_check_addr(in_str) >= 0)
					wsa_addr = in_str;
				else {
					printf("\nInvalid address. Try again or 'h'.\n");
					continue;
				}
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.# or host name format)\n");
			continue;
		}	

		//*****
		// All are good, start the connection & command part
		//*****
		result = do_wsa(wsa_addr);
		if (result >= 0)
			break;
		else
			printf("ERROR %d: %s.\n", result, wsa_get_err_msg(result));

	} while (!user_quit);
	
	free(in_buf);

	return 0;
}

void call_mode_print_help(char* argv) {
	fprintf(stderr, "Usage:\n %s -c [-h] -ip=<#.#.#.# or host name> "
		"[{h}] [{<cmd1>}] [{<cmd2>}] [{...}]\n\nCase insensitive\n[ ]:"
		" optional parameter\n< >: required parameter\n\n", argv);
}


/**
 * Process the standalone call '-c' method
 * Takes argument string in the form of:
 * <executable name> -c [-h] -ip=<...> [{h}] [{cmd1}] [{cmd2}] [{...}]
 * @param argc - Integer number of argument words
 * @param argv - Pointer to pointer of characters
s * @return 0 if success, negative number if failed
 */
int16_t process_call_mode(int32_t argc, char **argv)
{
	int i, j; // for loop index
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words
	int16_t w = 1;				// skip the executable at index 0
	int16_t c = 0;				// keep track of the words processed
	char temp_str[200];
	char intf_str[200];			// store the interface method string
	uint8_t cmd_end = FALSE, is_cmd = FALSE;	// some cmd words related flags
	uint8_t do_once = TRUE;
	uint8_t has_ipstr = FALSE;

	int16_t result = 0;
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_device *dev;
	
	dev = &wsa_dev;

	// Allocate memory
	for (i = 0; i < MAX_CMD_WORDS; i++) {
		cmd_words[i] = (char*) malloc(MAX_STRING_LEN * sizeof(char));
		strcpy(cmd_words[i], "");
	}

	do {
	//*****
	// Process non-cmd words
	//*****
		if (!is_cmd && !cmd_end) {
			// ignore -c & -h command
			if (strcmp(argv[w], "-c") == 0 || strcmp(argv[w], "-h") == 0) {
				w++;
			}

			// Get the ip address
			if (strncmp(argv[w], "-ip=", 4) == 0) {
				if (strlen(argv[w]) <= 4) {
					printf("\nERROR: Invalid IP address or host name.\n");
					result = WSA_ERR_INVIPHOSTADDRESS;
					break;
				}

				has_ipstr = TRUE;

				strcpy(temp_str, strchr(argv[w], '='));
				strcpy(temp_str, strtok(temp_str, "="));

				// Create the TCPIP interface method string
				sprintf(intf_str, "TCPIP::%s::%d,%d", temp_str, CTRL_PORT, 
					DATA_PORT);

				w++;
			}

			if (argv[w] == NULL)
				break;
		} // end !is_cmd


	//*****
	// get the command words
	//*****
		// clear up the cmd words array before storing new cmds
		if (cmd_end) {
			if (!has_ipstr) {
				printf("ERROR: No IP address is given!\n");
				break;
			}
			for (i = 0; i < MAX_CMD_WORDS; i++)
				strcpy(cmd_words[i], "");
		}

		// case { or {}... or {w}...
		if (argv[w][0] == '{') {
			is_cmd = TRUE;

			if (strlen(argv[w]) == 1) {
				w++;
				// If no more commands
				if (argv[w] == NULL)
					break;
			}
			else 
				argv[w] = strtok(argv[w], "{");	
		}

		// case {w only
		if (strchr(argv[w], '{') != NULL && strchr(argv[w], '}') == NULL){
			strcpy(cmd_words[c], strtok(argv[w], "{"));
			is_cmd = TRUE;
			w++;
			c++;
		}
		
		// case }
		else if (strlen(argv[w]) == 1 && strchr(argv[w], '}') != NULL) {
			is_cmd = FALSE;
			cmd_end = TRUE;
			w++;
		}

		// case w}
		else if (strchr(argv[w], '}') != NULL && strchr(argv[w], '{') == NULL){
			strcpy(cmd_words[c], strtok(argv[w], "}"));	
			is_cmd = FALSE;
			cmd_end = TRUE;
			w++;
		}

		// case contains }{
		else if (strstr(argv[w], "}{") != NULL) {
			// case w}{ or w}{w
			if (argv[w][0] != '}')
				strcpy(cmd_words[c], strtok(argv[w], "}")); // get the front w
			
			// remove the front part } or w} & does not increment w so 
			// { or {w gets processed at the next loop
			argv[w] = strchr(argv[w], '{');	
			is_cmd = FALSE;
			cmd_end = TRUE;
		}

		// case within { w }
		else if (is_cmd) {
			strcpy(cmd_words[c], argv[w]);
			w++;
			c++;
		}
		
		// words not within the bracket
		else {
			if (strncmp(argv[w], "ip=", 3) == 0) {
				printf("Unable to detect IP address.\n\n");
				call_mode_print_help(argv[0]);
				break;
			}
			
			if (has_ipstr)
				printf("Warning: Omit '%s' not contained within { }.\n", 
					argv[w]);
			w++;
		}

		//*****
		// 1st: verify that ip string is included, then
		// 2nd: Send the cmd words to be processed if there are any & tx
		//*****
		if (cmd_end) {
			if (!has_ipstr) {
				printf("Error: No IP address is given!\n\n");
				call_mode_print_help(argv[0]);
				break;
			}

			strcpy(temp_str, "");
			for (i = 0; i <= c; i++) {
				// Copy the words into one complete cmd string for display
				if (strcmp(cmd_words[i], "") != 0)
					strcat(temp_str, cmd_words[i]);
				if (i < c)
					strcat(temp_str, " ");

				// capitalized the words
				for (j = 0; j < (int) strlen(cmd_words[i]); j++)
					cmd_words[i][j] = (char) toupper(cmd_words[i][j]);
			}

			if (strlen(temp_str) > 0) {
				// there are cmds, so establish the connection but only once
				if (do_once) {
					printf("\n");
					// Start the WSA connection
					if ((result = wsa_open(dev, intf_str)) < 0) {
						printf("Error WSA_ERR_OPENFAILED: %s.", 
							wsa_get_err_msg(WSA_ERR_OPENFAILED));
						break;
					}
					do_once = FALSE;
				}

				result = process_cmd_words(dev, cmd_words, c + 1);
				if (result < 0)
					printf("Command '%s' not recognized.  See {h}.\n", 
						temp_str);
			}

			// restart the flag
			cmd_end = FALSE;
			c = 0;

		}  // end if cmd_end
	} while (w < argc);

	// Free the allocation
	for (i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	// if do_once is never started, print warning.
	if (do_once && has_ipstr)
		printf("No command detected. See {h}.\n");
	// finish so close the connection
	else
		wsa_close(dev);

	return result;
}


/**
 * Print out some statistics of the WSA's current settings
 * @param dev - A pointer to the WSA device structure.
 * @return 0
 */
void print_wsa_stat(struct wsa_device *dev) {
	int16_t result;
	int64_t freq;
	int32_t value;
	int32_t samples_per_packet;
	int32_t packets_per_block;
    int64_t start_frequency = 0;
	int64_t stop_frequency = 0;
	int64_t amplitude = 0;
	int32_t enable = 0;
	char dig1 = 0;
	char dig2 = 0;
	char dig3 = 0;
	enum wsa_gain gain;

	printf("\nCurrent WSA's statistics:\n");
	printf("\t- Current settings: \n");

	// TODO handle the errors
	result = wsa_get_fw_ver(dev);
	if (result < 0)
		printf("\t\t- Error: Failed getting the firmware version.\n");

	result = wsa_get_freq(dev, &freq);
	if (result >= 0)
		printf("\t\t- Frequency: %0.3lf MHz\n", (float) freq/MHZ);
	else
		printf("\t\t- Error: Failed getting the frequency value.\n");

	result = wsa_get_gain_if(dev, &value);
	if (result >= 0)
		printf("\t\t- Gain IF: %d dB\n", value);
	else
		printf("\t\t- Error: Failed getting the gain IF value.\n");

	result = wsa_get_gain_rf(dev, &gain);
	if (result >= 0) {
		char temp[10];
		gain_rf_to_str(gain, &temp[0]);
		printf("\t\t- Gain RF: %s\n", temp);
	}
	else
		printf("\t\t- Error: Failed getting the gain RF value.\n");
	
	result = wsa_get_samples_per_packet(dev, &samples_per_packet);
	if (result >= 0) {
		printf("\t\t- Samples per packet: %d\n", samples_per_packet);
	}
	else {
		printf("\t\t- Error: Failed reading the samples per packet value.\n");
	}
	
	result = wsa_get_packets_per_block(dev, &packets_per_block);
	if (result >= 0) {
		printf("\t\t- Packets per block: %d\n", packets_per_block);
	}
	else {
		printf("\t\t- Error: Failed reading the packets per bock value.\n");
	}
	
	printf("\t\t- Trigger Settings: \n");
	result = wsa_get_trigger_enable(dev, &enable);
	if (result >= 0) {
		if (enable == 0) {
			printf("\t\t\t- Trigger Status: Disabled\n");
		}
		else if (enable == 1) {
			printf("\t\t\t- Trigger Status: Enabled\n");
		}
	}
	else {
		printf("\t\t- Error: Failed reading if the triggers are enabled.\n");
	}
	
	result = wsa_get_trigger_level(dev, &start_frequency, &stop_frequency, &amplitude);
	if (result >= 0) {
		printf("\t\t\t- Start Frequency: %u\n", start_frequency/MHZ);
		printf("\t\t\t- Stop Frequency: %u\n", stop_frequency/MHZ);
		printf("\t\t\t- Amplitude: %lld dBm\n", amplitude);
	}
	else {
		printf("\t\t- Error: Failed reading trigger levels.\n");
	}
} 

/**
 * Convert a gain RF setting to a string, useful for printing
 * @param gain - the enum'ed gain value to be converted into a string name
 * @param gain_str - a char pointer to store the string name of the given gain
 * @return 0 if successful, else a negative value
 */
int16_t gain_rf_to_str(enum wsa_gain gain, char *gain_str)
{
	switch(gain) {
		case(WSA_GAIN_HIGH):	strcpy(gain_str, "HIGH"); break;
		case(WSA_GAIN_MED):		strcpy(gain_str, "MEDIUM"); break;
		case(WSA_GAIN_LOW):		strcpy(gain_str, "LOW"); break;
		case(WSA_GAIN_VLOW):	strcpy(gain_str, "VLOW"); break;
		default: strcpy(gain_str, "Unknown"); break;
	}
	
	return 0;
}

/**
 * Print the settings of the user's sweep entry
 * @param dev - a pointer to the wsa device
 * 
 */
void print_sweep_entry(struct wsa_device *dev) 
{	
	int16_t result = 0;			// result returned from a function
	int64_t freq = 0;
	float fshift = 0;
	int int_result = 0;
	int32_t dwell_seconds_value = 15;
	int32_t dwell_useconds_value = 16;
	int32_t samples_per_packet;
	int32_t packets_per_block;
	int64_t start_frequency;
	int64_t stop_frequency;
	int64_t amplitude;
	enum wsa_gain gain;

	//print antenna sweep value
	printf("Sweep Entry Settings:\n");

	//print frequency sweep value
	result = wsa_get_sweep_freq(dev, &start_frequency, &stop_frequency);
	if (result >= 0) {
		printf("   Sweep range (MHz): %0.3d - %0.3d, ", start_frequency / MHZ, stop_frequency / MHZ);
	}

	//print fstep sweep value
	result = wsa_get_sweep_freq_step(dev, &freq);
	if (result >= 0) {
		printf("Step: %0.3d \n", freq/MHZ);
	}

	result = wsa_get_sweep_antenna(dev, &int_result);
	if (result >= 0) {
		printf("   Antenna Port: %d \n", int_result);
	}

	//print samples per packets sweep value
	result = wsa_get_sweep_samples_per_packet(dev, &samples_per_packet);
	if (result >= 0) {
		printf("   Capture Block: %d * ", samples_per_packet);
	}

	//print packets per block
	result = wsa_get_sweep_packets_per_block(dev, &packets_per_block);
	if (result >= 0) {
		printf("%d \n", packets_per_block);
	}

	//print decimation sweep value
	result = wsa_get_sweep_decimation(dev, &int_result);
	if (result >= 0) {
		printf("   Decimation: %d \n", int_result);
	}

	//print fstep sweep value	
	result = wsa_get_sweep_freq_shift(dev, &fshift);
	if (result >= 0) {
		printf("   Frequency Shift: %0.3f MHz \n", fshift/MHZ);
	}

	//print gain if sweep value		
	result = wsa_get_sweep_gain_if(dev, &int_result);
	if (result >= 0) {
		printf("   Gain IF: %d dBm \n", int_result);
	}

	//print gain rf sweep value
	result = wsa_get_sweep_gain_rf(dev, &gain);
	if (result >= 0) {
		char temp[10];
		gain_rf_to_str(gain, &temp[0]);
		printf("   Gain RF: %s\n", temp);
	}

	//print trigger status sweep value
	printf("   Trigger configuration:\n");
	result = wsa_get_sweep_trigger_type(dev, &int_result);
	if (result >= 0) {
		printf("      Trigger mode: ");
		if (int_result == 1) {
			printf("On\n");
			result = wsa_get_sweep_trigger_level(dev, &start_frequency, &stop_frequency, &amplitude);
			if (result >= 0) {
				printf("      Start frequency: %d MHz\n", (start_frequency));
				printf("      Stop frequency: %d MHz\n", (stop_frequency));
				printf("      Amplitude: %d dBm\n", amplitude);
			}
		}
		else if (int_result == 0) {
			printf("Off\n");
		}
		else {
			printf("Unknown state\n");
		}
	}

	//print dwell sweep value
	result = wsa_get_sweep_dwell(dev, &dwell_seconds_value, &dwell_useconds_value);
	if (result >= 0) {
		printf("   Dwell time: %u.%llu seconds\n", dwell_seconds_value, dwell_useconds_value);
	}
}

/**
 *
 */
void print_sweep_entry_information(struct wsa_device *dev, int32_t position) 
{	
	int16_t result;
	char temp[10];
	struct wsa_sweep_list* list_values;

	list_values = (struct wsa_sweep_list*) malloc(sizeof(struct wsa_sweep_list));

	printf("Sweep Entry Settings:\n");
	result = wsa_sweep_list_read(dev, position, list_values);
	printf("  Start frequency: %d MHz\n", (list_values->start_frequency / MHZ));
	printf("  Stop frequency: %d MHz\n", (list_values->stop_frequency / MHZ));
	printf("  Step Frequency: %d MHz\n",list_values->fstep / MHZ);
	printf("  Frequency shift: %f MHz\n",list_values->fshift);
	printf("  decimation rate: %u \n",list_values->decimation_rate);
	printf("  antenna port: %u \n",list_values->ant_port);
	printf("  IF gain: %u \n", list_values->gain_if);

	gain_rf_to_str(list_values->gain_rf, &temp[0]);
	printf("  Current RF gain: %s\n", temp);
	printf("  Samples Per backet: %d \n", list_values->samples_per_packet);
	printf("  Packets per block: %d \n", list_values->packets_per_block);
	printf("  Dwell seconds value is: %u \n", list_values->dwell_seconds_value);
	printf("  Dwell Microseconds value is: %lu \n", list_values->dwell_useconds_value);
	
	printf("  Trigger Settings:\n");
	if ( list_values->trigger_enable == 0) {
		printf("        Triggers are disabled\n");
	} 
	else if ( list_values->trigger_enable == 1) {
		printf("        Triggers are enabled\n");
		printf("        Trigger Start frequency: %d MHz\n", (list_values->trigger_start_frequency / MHZ));
		printf("        Trigger Stop frequency: %d MHz\n", (list_values->trigger_stop_frequency / MHZ));
		printf("        Trigger amplitude: %d dBm\n", list_values->trigger_amplitude);
	}
}
