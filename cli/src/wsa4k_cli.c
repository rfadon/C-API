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
#include <conio.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_lib.h"
#include "wsa_client.h"
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
int16_t save_data_to_bin_file(struct wsa_device *dev, char* prefix, char *ext);
int16_t print_sweep_entry_template(struct wsa_device *dev);
int16_t print_sweep_entry_information(struct wsa_device *dev, int32_t id);

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

	//printf("\n---------------------------\n");
	printf("\nCommand Options Available (case insensitive, < > required, "
		"[ ] optional):\n\n");
	printf("  dir\n"
		"\t- List the captured file path.\n");
	printf("  h\n"
		"\t- Show the list of available options.\n");
	printf("  o\n"
		"\t- Open the folder of captured file(s).\n");
	printf("  q\n"
		"\t- Quit or exit this console.\n");
	printf("  run cmdf <scpi | cli> <file name>\n"
		"\t- Run set or get commands stored in a text file.\n");
	printf("  save [name] [ext:<type>]\n"
		"\t- Save data to a file with optional prefix string.\n"
		"\t  Output: [name] YYYY-MM-DD_HHMMSSmmm.[ext]\n"
		"\t  'ext' type: csv (default), xsl, bin (raw VRT packets)\n"
		"\t  Ex: save Test trial ext:xsl\n"
		"\t      save\n");
	printf("\n");

	printf("////////////////////////////////////////////////////////////////\n");
	printf("//                    Manual Set-up Commands                  //\n");
	printf("////////////////////////////////////////////////////////////////\n");

	printf("\n"); 

	printf("  get acq access\n"
		"\t- Obtain WSA4000 acquisition access to save data.\n");
	printf("  get ant\n"
		"\t- Get the current antenna port in use.\n");
	printf("  get bpf\n"
		"\t- Get the current RFE's preselect BPF state.\n");
	printf("  get dec [max | min]\n"
		"\t- Get the decimation rate (0 = off).\n");
	printf("  get freq [max | min]\n"
		"\t- Get the current running centre frequency (in MHz).\n");	
	printf("  get fshift [max | min]\n"
		"\t- Get the frequency shift value (in MHz).\n");  
	printf("  get gain <rf | if> [max | min]\n"
		"\t- Get the current RF or IF gain level.\n");
	printf("  get ppb\n"
		"\t- Get the current packets per block.\n");
	printf("  get spp [max | min]\n"
		"\t- Get the current samples per packet.\n");
	printf("  get trigger level\n"
		"\t- Get the current level trigger settings\n");
	printf("  get trigger mode\n"
		"\t- Check the current trigger mode\n");
	printf("\n");

	printf("  set ant <1 | 2>\n"
		"\t- Select the antenna port, available 1 to %d.\n", WSA_RFE0560_MAX_ANT_PORT);
	printf("  set bpf <on | off>\n"
		"\t- Turn the RFE's preselect BPF stage on or off.\n");
	printf("  set dec <rate>\n"
		"\t- Set the decimation rate\n"
		"\t  Range: 0 (for off), %d - %d.\n", 
		dev->descr.min_decimation, dev->descr.max_decimation);
	printf("  set freq <freq>\n"
		"\t- Set the centre frequency in MHz\n"
		"\t  Range: %.2f - %.2f MHz inclusively, excluding 40.1 - 89.9 MHz.\n"
		"\t  Resolution %.2f MHz.\n"
		"\t  ex: set freq 441.5\n",
		(float) MIN_FREQ/MHZ, (float) MAX_FREQ/MHZ, (float) FREQ_RES/MHZ);
	printf("  set fshift <freq>\n"
		"\t- Set the frequency shift in MHz\n"
		"\t  Range: %f - %f MHz\n"
		"\t  ex: set fshift 10\n", 
		(float) dev->descr.inst_bw/MHZ * -1, (float) dev->descr.inst_bw/MHZ);
	printf("  set gain <rf | if> <val>\n"
		"\t- Set gain level for RF front end or IF\n"
		"\t  RF options: HIGH, MEDium, LOW, VLOW.\n"
		"\t  IF range: %d to %d dBm, inclusive.\n"
		"\t  ex: set gain rf high;\n"
		"\t      set gain if -20.\n", MIN_IF_GAIN, MAX_IF_GAIN);
	printf("  set ppb <packets>\n"
		"\t- Set the number of packets per block to be captured\n"
		"\t  The maximum value will depend on the \"samples per packet\" setting\n"
		"\t  ex: set ppb 100\n");
	printf("  set spp <samples>\n"
		"\t- Set the number of samples per packet to be captured\n"
		"\t  Range: %hu - %hu, inclusive.\n"
		"\t  ex: set spp 2000\n",
		WSA4000_MIN_SAMPLES_PER_PACKET, WSA4000_MAX_SAMPLES_PER_PACKET);
	printf("  set trigger mode <level | none>\n"
		"\t- Set trigger mode. When set to none, WSA will be in freerun.\n"
		"\t  ex: set trigger mode level\n");
	printf("  set trigger level <start,stop,amplitude>\n"
		"\t- Configure the level trigger with values:\n"
		"\t\t1) Start frequency (in MHz)\n"
		"\t\t2) Stop frequency (in MHz)\n"
		"\t\t3) Amplitude (in dBm)\n"
		"\t  ex: set trigger level 2410,2450,-50\n");
	printf("\n");

	printf("//////////////////////////////////////////////////////////////////\n");
	printf("//                Sweep Capture Commands                        //\n");
	printf("//////////////////////////////////////////////////////////////////\n");

	printf("\n"); 
	
	printf("Commands for Sweep Execution:\n");
	printf("  sweep resume\n"
		"\t- Resume the current sweep list from where it was last stopped\n");
	printf("  sweep start [ID]\n"
		"\t- Execute the current sweep list from the first entry\n");
	printf("  sweep stop\n"
		"\t- Stop the sweep process\n");
	printf("  sweep flush\n"
		"\t- Clear the WSA4000 internal buffer of remaining sweep data\n");
	printf("  sweep status\n"
		"\t- Get the current sweep status, running or stopped\n");
	printf("\n");

	printf("Commands for Sweep Editing:\n");
	printf("NOTE: No setting changes will take effect until the save command is issued\n");
	printf("\n");

	printf("  sweep entry new\n"
		"\t- Reset the editing entry settings to default values\n");
	printf("  sweep entry copy <ID>\n"
		"\t- Copy the settings of the specified entry <ID> into the entry\n"
		"\t  template for quick editing\n");	
	printf("  sweep entry delete <ID | ALL>\n"
		"\t- Delete the specified entry <ID> or all entries\n");
	printf("  sweep entry save [ID]\n"
		"\t- Save the entry template settings to the specified <ID> location\n"
		"\t  or to the end of the list if no ID (or ID 0) is specified\n");
	printf("  sweep entry read [ID]\n"
		"\t- Get the settings of a sweep entry.  When no ID is specified,\n"
		"\t  the current settings of the entry template will be returned\n");
	printf("\n");

	printf("  get sweep entry size\n"
		"\t- Get the number of entries available in the current list\n");
	printf("\n");

	printf("These sweep set commands are to edit the settings of the entry template.\n");
	printf("See the manual mode above for the value ranges and definition.\n");
	printf("  set sweep entry ant <1 | 2>\n");
	printf("  set sweep entry dec <rate>\n");
	printf("  set sweep entry gain <rf | if>\n");
	printf("  set sweep entry freq <start,stop>\n");
	printf("  set sweep entry fshift <freq>\n");
	printf("  set sweep entry ppb <packets>\n");
	printf("  set sweep entry spp <samples>\n");
	printf("  set sweep entry trigger mode <none | level>\n");
	printf("  set sweep entry trigger level <start,stop,amplitude>\n");
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
	if (input_opt == NULL) 
	{
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

	if((cmd_fptr = fopen(file_name, "r")) == NULL) 
	{
		printf("Error opening '%s'.\n", file_name);
		return WSA_ERR_FILEOPENFAILED;
	}

	// Allocate memory
	for (i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STRING_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	fclose(cmd_fptr);

	if (result < 0) 
	{
		// free memory
		for (i = 0; i < MAX_FILE_LINES; i++)
			free(cmd_strs[i]);
		return WSA_ERR_FILEREADFAILED;
	}

	// Send each command line to WSA
	lines = result;
	for (i = 0; i < lines; i++) 
	{
		result = process_cmd_string(dev, cmd_strs[i]);		
		// If a bad command is detected, continue? Prefer not.
		if (result < 0) 
		{
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
	char temp_str[40];
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
	while (temp_ptr != NULL) 
	{
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
	int16_t result = 0;
	int32_t samples_per_packet;
	int32_t packets_per_block = 0;
	char fw_ver[40];
	int16_t acq_status;
	int32_t exit_loop;
	int32_t title_printed = FALSE;
	int32_t dec = 0;
	
	// to store receiver data
	int32_t reference_point = 0;
	int32_t if_gain = 0;
	int32_t rf_gain = 0;
	int64_t frequency = 0;

	// to store digitizer data
	int16_t reference_level = 0;
	int64_t bandwidth = 0;
	float rf_frequency_offset = 0;

	char file_name[40];
	char sweep_status[40];
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
	struct wsa_extension_packet* extension;
	
	// *****
	// Create buffers to store the decoded I & Q from the raw data
	// *****
	int16_t *i_buffer;		// To store the integer I data
	int16_t *q_buffer;		// To store the integer Q data
	
	// *****
	// Initialize packet order indicators of all the packet types
	// *****
	uint8_t expected_if_packet_order_indicator = UNASSIGNED_PACKET_ORDER_INDICATOR;
	uint8_t expected_digitizer_packet_order_indicator = UNASSIGNED_PACKET_ORDER_INDICATOR;
	uint8_t expected_receiver_packet_order_indicator =  UNASSIGNED_PACKET_ORDER_INDICATOR;
	uint8_t expected_extension_packet_order_indicator =  UNASSIGNED_PACKET_ORDER_INDICATOR;

	int32_t iq_pkt_count = 1;
	int32_t i;
	int j;

	// *****
	// Get parameters to stored in the file
	// TODO smarter way of saving header is to allow users to enable
	// which header info to include...
	// *****

	printf("Gathering WSA settings...");	
	

	// determine if the another user is capturing data
	result = wsa_system_acq_status(dev, &acq_status);
	if (result < 0)
		return result;
	else if (acq_status == 0) 
		return WSA_ERR_DATAACCESSDENIED;

	// Get sweep status
	result = wsa_get_sweep_status(dev, sweep_status);
	if (result < 0)
		return result;
	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) != 0 && strcmp(sweep_status, WSA4000_SWEEP_STATE_RUNNING) != 0)
	{
		return WSA_ERR_SWEEPMODEUNDEF;
	}

	result = wsa_get_fw_ver(dev, fw_ver);
	if (result < 0)
		return result;
	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0) 
	{	

		// Get samples per packet
		result = wsa_get_samples_per_packet(dev, &samples_per_packet);
		doutf(DMED, "In save_data_to_file: wsa_get_samples_per_packet returned %hd\n", result);
		if (result < 0)
			return result;

		// Get packets per block
		result = wsa_get_packets_per_block(dev, &packets_per_block);
		doutf(DMED, "In save_data_to_file: wsa_get_packets_per_block returned %hd\n", result);	
		if (result < 0)
			return result;	
	printf(" done.\n");

	}

	// if sweep mode is enabled, samples per packet is set to the 
	// maximum value to hold iq packets with variant sizes
	else
	{
		printf(" done.\n");
		printf("Sweep Mode is enabled. \n Press 'ESC' key to stop data capture.\n");
		samples_per_packet = dev->descr.max_sample_size;
	}
	
	
	// create file name in format "[prefix] YYYY-MM-DD_HHMMSSmmm.[ext]" in a 
	// folder called CAPTURES
	generate_file_name(file_name, prefix, ext);
	
	// create a new file for data capture
	if ((iq_fptr = fopen(file_name, "a+")) == NULL) {
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

	// allocate receiver packet buffer space
	receiver = (struct wsa_receiver_packet*) malloc(sizeof(struct wsa_receiver_packet));
	if (receiver == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate receiver\n");
		fclose(iq_fptr); 
		free(trailer);
		free(header);
		
		return WSA_ERR_MALLOCFAILED;
	}

	// allocate digitizer packet buffer space
	digitizer = (struct wsa_digitizer_packet*) malloc(sizeof(struct wsa_digitizer_packet));
	if (digitizer == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate digitizer\n");
		fclose(iq_fptr); 
		free(receiver);
		free(trailer);
		free(header);
		
		return WSA_ERR_MALLOCFAILED;
	}

	// allocate extension packet buffer space
	extension = (struct wsa_extension_packet*) malloc(sizeof(struct wsa_extension_packet));
	if (extension == NULL)
	{
		doutf(DHIGH, "In save_data_to_file: failed to allocate sweep info\n");
		fclose(iq_fptr); 
		free(receiver);
		free(trailer);
		free(header);
		free(digitizer);
		
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
		free(extension);

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
		free(extension);

		return WSA_ERR_MALLOCFAILED;
	}

	// set capture block if not doing sweep
	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0) 
	{
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
			free(extension);
			return result;
		}
	}

	// Get the start time
	get_current_time(&run_start_time);

	// Initialize counter i
	i = 0;

	// loop to save data in file
	exit_loop = 0;
	while (exit_loop != 1)
	{
		i++;
		// Get the start time
		get_current_time(&capture_start_time);

		result = wsa_read_vrt_packet(dev, header, trailer, receiver, digitizer, extension, 
					i_buffer, q_buffer, samples_per_packet);
		if (result < 0)
			break;
	
		
		// get the end time of each data capture
		get_current_time(&capture_end_time);
		// sum it up
		capture_time_ms += get_time_difference(&capture_start_time, &capture_end_time);

		// TODO handle the packet order indicator for rec'r & dig'r as well		

		// handle extension packets
		if (header->packet_type == EXTENSION_PACKET_TYPE) 
		{
			// initialize extension packet order indicator
			if (expected_extension_packet_order_indicator == UNASSIGNED_PACKET_ORDER_INDICATOR)
				expected_extension_packet_order_indicator = extension->packet_order_indicator;			
				
			// verify the extension packet order indicator
			if (extension->packet_order_indicator != expected_extension_packet_order_indicator)
			{
				result = WSA_ERR_PACKETOUTOFORDER;
				break;
			}
				
			expected_extension_packet_order_indicator++;

			// reset  extension packet order indicator to original value
			if (expected_extension_packet_order_indicator > MAX_PACKET_ORDER_INDICATOR)
			expected_extension_packet_order_indicator = MIN_PACKET_ORDER_INDICATOR;
			
			// TODO find meaningful way to convey the sweep_start_id to the user
			//if (header->stream_id == EXTENSION_STREAM_ID)

			i--;
			continue;
		}
		// handle context packets
		if (header->packet_type == CONTEXT_PACKET_TYPE) 
		{
			// handle receiver packet
			if (header->stream_id == RECEIVER_STREAM_ID)
			{
				
				// initialize receiver packet order indicator
				if (expected_receiver_packet_order_indicator == UNASSIGNED_PACKET_ORDER_INDICATOR)
					expected_receiver_packet_order_indicator = receiver->packet_order_indicator;
				
				// verify the receiver packet order indicator
				if (receiver->packet_order_indicator != expected_receiver_packet_order_indicator)
				{
					result = WSA_ERR_PACKETOUTOFORDER;
					break;
				}
				expected_receiver_packet_order_indicator++;
				// reset packet order indicator to original value
				if (expected_receiver_packet_order_indicator > MAX_PACKET_ORDER_INDICATOR)
				expected_receiver_packet_order_indicator = MIN_PACKET_ORDER_INDICATOR;

				// handle change in reference point
				if ((receiver->indicator_field & REFERENCE_POINT_FIELD_INDICATOR_MASK) == REFERENCE_POINT_FIELD_INDICATOR_MASK)
					reference_point = (int32_t) receiver->reference_point;		

				// handle change in frequency
				else if ((receiver->indicator_field  & FREQ_FIELD_INDICATOR_MASK) == FREQ_FIELD_INDICATOR_MASK)
					frequency = (int64_t) receiver->freq;		

				// handle change in gain rf/if 
				else if ((receiver->indicator_field & GAIN_FIELD_INDICATOR_MASK) == GAIN_FIELD_INDICATOR_MASK) 
				{
					if_gain = (int32_t) receiver->gain_if;
					rf_gain = (int32_t) receiver->gain_rf;
				}
			}

			// handle possible digitizer changes
			else if (header->stream_id == DIGITIZER_STREAM_ID)
			{
				
				// initialize digitizer packet order indicator
				if (expected_digitizer_packet_order_indicator == UNASSIGNED_PACKET_ORDER_INDICATOR)
					expected_digitizer_packet_order_indicator = digitizer->packet_order_indicator;			
				
				// verify the digitizer packet order indicator
				if (digitizer->packet_order_indicator != expected_digitizer_packet_order_indicator)
				{
					result = WSA_ERR_PACKETOUTOFORDER;
					break;
				}
				
				expected_digitizer_packet_order_indicator++;

				// reset  digitizer packet order indicator to original value
				if (expected_digitizer_packet_order_indicator > MAX_PACKET_ORDER_INDICATOR)
				expected_digitizer_packet_order_indicator = MIN_PACKET_ORDER_INDICATOR;

				// handle change in bandwidth
				if ((digitizer->indicator_field & BANDWIDTH_FIELD_INDICATOR_MASK) == BANDWIDTH_FIELD_INDICATOR_MASK) 
					bandwidth = (int64_t) digitizer->bandwidth;
		
				// handle change in frequency offset
				else if ((digitizer->indicator_field & RF_FREQUENCY_OFFSET_INDICATOR_MASK) == RF_FREQUENCY_OFFSET_INDICATOR_MASK)
					rf_frequency_offset = (float)  digitizer->rf_frequency_offset;

				// handle change in reference level
				else if ((digitizer->indicator_field & REFERENCE_LEVEL_FIELD_INDICATOR_MASK) == REFERENCE_LEVEL_FIELD_INDICATOR_MASK)
					reference_level = (int16_t) digitizer->reference_level;			
			}

			i--;
			continue;
		} 

		// handle if packet types
		else if (header->packet_type == IF_PACKET_TYPE)
		{
			if (header->stream_id == IF_DATA_STREAM_ID)
			{
			
				if ( (strcmp(sweep_status, WSA4000_SWEEP_STATE_RUNNING) == 0 || (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0 && title_printed == FALSE))) 
				{
					title_printed = TRUE;
					fprintf(iq_fptr, "FwVersion,SampleSize,Seconds,Picoseconds,CentreFreq,Bandwidth,OffsetFreq,GainIF,GainRF,RefPoint,RefLevel\n");	
					fprintf(iq_fptr, "%s,%d,%d,%u,%0.3f,%0.3f,%lf,%lf,%lf,%d,%lf\n",fw_ver,
																						header->samples_per_packet, 
																						header->time_stamp.sec,
																						header->time_stamp.psec,
																						(float) frequency,
																						(float) bandwidth,
																						(float) rf_frequency_offset,
																						(float) if_gain,
																						(float) rf_gain,
																						(int32_t) reference_point,
																						(float) reference_level);
				}

				if (expected_if_packet_order_indicator == UNASSIGNED_PACKET_ORDER_INDICATOR)		
					expected_if_packet_order_indicator = header->packet_order_indicator;

				if (header->packet_order_indicator != expected_if_packet_order_indicator)
				{

					result = WSA_ERR_PACKETOUTOFORDER;
					break;
				}

				expected_if_packet_order_indicator++;	
			
				if (expected_if_packet_order_indicator > MAX_PACKET_ORDER_INDICATOR)
					expected_if_packet_order_indicator = MIN_PACKET_ORDER_INDICATOR;

				for (j = 0; j < header->samples_per_packet; j++)
					fprintf(iq_fptr, "%d,%d\n", i_buffer[j], q_buffer[j]);
		
				if (!(iq_pkt_count % 10))
				{
					if (iq_pkt_count != packets_per_block)
						printf(" \n");
				}
				else 
					printf(".");

				iq_pkt_count++;

				// if capture mode is enabled, save the number of specified packets
				if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0)
				{
					if (i >= packets_per_block) 
					{
						exit_loop = TRUE;
					}
				}		
		 
				// if sweep mode is enabled, capture data until the 'ESC' key is pressed
				else 
				{
					if (_kbhit() != FALSE)
					{
						if (_getch() == 0x1b) {    // esc key
							 
							printf("\nEscape key pressed, data capture stopped...\n");
							exit_loop = TRUE;
						}
					}
				}
			}
		}
	}

	if (result >= 0) 
	{
		// get the total run end time
		get_current_time(&run_end_time);
		run_time_ms = get_time_difference(&run_start_time, &run_end_time);
		
		printf("done.\n\n");
		
		printf("(Data capture time: %.3f sec; Total run time: %.3f sec)\n", 
			capture_time_ms,
			run_time_ms);
	}
		
	fclose(iq_fptr);
	free(digitizer);
	free(receiver);
	free(trailer);
	free(header);
	free(i_buffer);
	free(q_buffer);
	free(extension);

	return result;
}

/**
 * Save data to a bin file with the file name format as:
 * [prefix] YYYY-MM-DD_HHMMSSmmm.bin
 *
 * @param dev - A pointer to the WSA device structure.
 * @param prefix - A char pointer to a prefix string.
 *
 * @return 0 if successful, else a negative value.
 */
int16_t save_data_to_bin_file(struct wsa_device *dev, char* prefix, char* ext)
{
	
	int32_t packets_per_block = 0;
	int32_t exit_loop;
	int16_t result = 0;
	int16_t acq_status;
	int32_t iq_pkt_count = 0;
	char file_name[40];
	char sweep_status[40];
	FILE *iq_fptr;

	// to calculate run time
	TIME_HOLDER run_start_time;
	TIME_HOLDER run_end_time;
	double run_time_ms = 0;

	int32_t bytes_received;
	int16_t socket_receive_result = 0;

	// to calculate data capture time
	TIME_HOLDER capture_start_time;
	double capture_time_ms = 0;

	uint16_t packet_size = 0;
	// *****
	// Create buffer to store raw data
	// *****
	uint8_t* vrt_header_buffer;	
	int32_t vrt_header_bytes;

	uint8_t* vrt_packet_buffer;
	int32_t vrt_packet_bytes;

	int32_t i;

	
	// create file name in format "[prefix] YYYY-MM-DD_HHMMSSmmm.[ext]" in a 
	// folder called CAPTURES
	generate_file_name(file_name, prefix, ext);

	// create a new file for the binary data capture
	if ((iq_fptr = fopen(file_name, "ab+")) == NULL) {
		printf("\nError creating the binary filefile \"%s\"!\n", prefix);
		return WSA_ERR_FILECREATEFAILED;
	}


	// determine if the another user is capturing data
	result = wsa_system_acq_status(dev, &acq_status);
	if (result < 0)
		return result;
	else if (acq_status == 0) 
		return WSA_ERR_DATAACCESSDENIED;

	// Get sweep status
	result = wsa_get_sweep_status(dev, sweep_status);
	if (result < 0)
		return result;
	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) != 0 && strcmp(sweep_status, WSA4000_SWEEP_STATE_RUNNING) != 0)
	{
		return WSA_ERR_SWEEPMODEUNDEF;
	}	

	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_RUNNING) == 0)
	{
		printf("Sweep Mode is enabled. \n Press 'ESC' key to stop data capture.\n");
	} else
		
		// get the ppb value
		result = wsa_get_packets_per_block(dev, &packets_per_block);	
		if (result < 0)
			return result;

	// set capture block if not doing sweep
	if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0) 
	{
		result = wsa_capture_block(dev);
		if (result < 0)
		{
			doutf(DHIGH, "In save_data_to_bin_file: wsa_capture_block returned %d\n", result);
			return result;
		}
	}

	// Get the start time
	get_current_time(&run_start_time);

	// Initialize counter i
	i = 0;
	printf("\n");

	// loop to save data in bin file
	exit_loop = 0;
	while (exit_loop != 1)
	{
		
		i++;
		// Get the start time
		get_current_time(&capture_start_time);

		// Set to get the first 2 words of the header to extract 
		// packet size and packet type
		vrt_header_bytes = 2 * BYTES_PER_VRT_WORD;
		
		// allocate space for the header buffer
		vrt_header_buffer = (uint8_t*) malloc(vrt_header_bytes * sizeof(uint8_t));
		if (vrt_header_buffer == NULL)
		{
			fclose(iq_fptr);
			return WSA_ERR_MALLOCFAILED;
		}
		
		
		socket_receive_result = wsa_sock_recv_data(dev->sock.data, vrt_header_buffer, vrt_header_bytes, TIMEOUT, &bytes_received);
		if (socket_receive_result < 0)
		{
			doutf(DHIGH, "Error in save_data_to_bin_file:  %s\n", wsa_get_error_msg(socket_receive_result));
			fclose(iq_fptr);
			free(vrt_header_buffer);
			return socket_receive_result;
		}
		// save the header data to the .bin file
		fwrite (vrt_header_buffer , 1 , vrt_header_bytes, iq_fptr);

		// retrieve the VRT packet size
		packet_size = (((uint16_t) vrt_header_buffer[2]) << 8) + (uint16_t) vrt_header_buffer[3];
		
		// allocate memory for the vrt packet without the first two words
		vrt_packet_bytes = BYTES_PER_VRT_WORD * (packet_size - 2);
		vrt_packet_buffer = (uint8_t*) malloc(vrt_packet_bytes * sizeof(uint8_t));
		if (vrt_packet_buffer == NULL)
		{
			fclose(iq_fptr);
			return WSA_ERR_MALLOCFAILED;
		}
		
		socket_receive_result = wsa_sock_recv_data(dev->sock.data, vrt_packet_buffer, vrt_packet_bytes, TIMEOUT, &bytes_received);
		doutf(DMED, "In save_data_to_bin_file: wsa_sock_recv_data returned %hd\n", socket_receive_result);
		
		if (socket_receive_result < 0)
			{
				doutf(DHIGH, "Error in save_data_to_bin_file:  %s\n", wsa_get_error_msg(socket_receive_result));
				fclose(iq_fptr);
				free(vrt_packet_buffer);
				free(vrt_header_buffer);
				return socket_receive_result;
			}
		
		// save packet data to the bin file
		fwrite (vrt_packet_buffer , 1 , vrt_packet_bytes, iq_fptr);

		// if capture mode is enabled, save the number of specified packets
		if (strcmp(sweep_status, WSA4000_SWEEP_STATE_STOPPED) == 0)
		{
			if (i >= packets_per_block) 
			{
				exit_loop = 1;
			}
		}		
		 
		// if sweep mode is enabled, capture data until the 'ESC' key is pressed
		else 
		{
			if (_kbhit() != 0)
			{
				if (_getch() == 0x1b) {    // esc key
					if (result < 0)
					{
						fclose(iq_fptr);
						return result;
					}
 
					printf("\nEscape key pressed, data capture stopped...\n");
					exit_loop = 1;
				}
			}
		}
		if (!(iq_pkt_count % 10))
			{
				if (iq_pkt_count != packets_per_block)
					printf(" \n");
			}
			else 
			{
				printf(".");
			}

		iq_pkt_count++;
		free(vrt_packet_buffer);
		free(vrt_header_buffer);
	}

	if (result >= 0) 
	{
		// get the total run end time
		get_current_time(&run_end_time);
		run_time_ms = get_time_difference(&run_start_time, &run_end_time);
		
		printf("done.\n\n");
		
		printf("(Data capture time: %.3f sec; Total run time: %.3f sec)\n", 
			capture_time_ms,
			run_time_ms);
	}
		
	fclose(iq_fptr);
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
	int16_t input_veri_result = 0; // result returned from varifying input cmd param
	int8_t user_quit = FALSE;	// determine if user has entered 'q' command
	char msg[MAX_STRING_LEN];
	int i;
	char* strtok_result;
	char char_result[MAX_STRING_LEN];
	
	int16_t temp_short;
	int32_t temp_int;
	long temp_long;
	double temp_double;

	float fshift = 0;
	int64_t freq_value;
	int64_t start_freq;
	int64_t stop_freq;
	int32_t amplitude;
	int32_t dwell_seconds;
	
	strcpy(msg,"");

	//*****
	// Handle GET commands
	//*****

	if (strcmp(cmd_words[0], "GET") == 0) 
	{
		if (strcmp(cmd_words[1], "ANT") == 0) 
		{
			if (num_words > 2)
			{
				if (strcmp(cmd_words[2], "MAX") == 0)
					printf("Maximum number of antenna ports: 2\n");
				else if (strcmp(cmd_words[2], "MIN") == 0)
					printf("Minimum number of antenna ports: 1\n");
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else
			{
				result = wsa_get_antenna(dev, &temp_int);
				if (result >= 0)
					printf("Currently using antenna port: %d\n", temp_int);
			}
		} // end get ANT 

		else if (strcmp(cmd_words[1], "BPF") == 0) 
		{			
			if(num_words > 2)
				printf("Extra parameters ignored (max/min not available)!\n");
			
			result = wsa_get_bpf_mode(dev, &temp_int);
			if (result >= 0)
				printf("RFE's preselect BPF state: %s.\n", (temp_int) ? "On" : "Off");
		} // end get BPF

		else if (strcmp(cmd_words[1], "DEC") == 0) 
		{
			if (num_words > 2)
			{
				if (strcmp(cmd_words[2], "MAX") == 0)
					printf("Maximum decimation rate: %d\n", 
						dev->descr.max_decimation);
				else if (strcmp(cmd_words[2], "MIN") == 0)
					printf("Minimum decimation rate: %d\n", 
						dev->descr.min_decimation);
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else
			{
				result = wsa_get_decimation(dev, &temp_int);
				if (result >= 0)
					printf("Current decimation rate: %d\n", temp_int);
			}
		} // end get decimation rate

		else if (strcmp(cmd_words[1], "FREQ") == 0) 
		{
			if (num_words > 2)
			{
				if (strcmp(cmd_words[2], "MAX") == 0) 
					printf("Maximum frequency: %0.2f MHz\n", 
						(float) dev->descr.max_tune_freq / MHZ);
				else if (strcmp(cmd_words[2], "MIN") == 0)
					printf("Minimum frequency: %0.2f MHz\n", 
						(float) dev->descr.min_tune_freq / MHZ);
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else
			{
				result = wsa_get_freq(dev, &freq_value);
				if (result >= 0)
					printf("Current centre frequency: %0.3f MHz\n", 
						(float) freq_value/MHZ);
			}
		} // end get FREQ

		else if (strcmp(cmd_words[1], "FSHIFT") == 0) 
		{
			if (num_words > 2)
			{
				if (strcmp(cmd_words[2], "MAX") == 0)
					printf("Maximum shift: %0.3f MHz\n", 
						(float) dev->descr.inst_bw / MHZ);
				else if (strcmp(cmd_words[2], "MIN") == 0)
					printf("Minimum frequency: %0.3f MHz\n", 
						(float) dev->descr.inst_bw / MHZ * (-1.0));
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else
			{
				result = wsa_get_freq_shift(dev, &fshift);
				if (result >= 0)
					printf("Current frequency shift: %0.6f MHz\n", fshift / MHZ);
			}
		} // end get FSHIFT

		else if (strcmp(cmd_words[1], "GAIN") == 0) 
		{
			if (strcmp(cmd_words[2], "RF") == 0) 
			{
				if (num_words > 3) 
				{
					if (strcmp(cmd_words[3], "MAX") == 0)
						printf("Maximum RF gain: HIGH\n");
					else if (strcmp(cmd_words[3], "MIN") == 0)
						printf("Minimum RF gain: VLOW\n");
					else
						printf("Did you mean \"min\" or \"max\"?\n");
				}
				else
				{
					result = wsa_get_gain_rf(dev, char_result);
					if (result >= 0) 
						printf("Current RF gain: %s\n", char_result);
					
				}
			}  // end get GAIN RF

			else if (strcmp(cmd_words[2], "IF") == 0) 
			{
				temp_int = -1000;
				if (num_words > 3) 
				{
					if (strcmp(cmd_words[3], "MAX") == 0)
						printf("Maximum IF gain: %d dB\n", 
							dev->descr.max_if_gain);
					else if (strcmp(cmd_words[3], "MIN") == 0)
						printf("Minimum IF gain: %d dB\n", 
							dev->descr.min_if_gain);
					else
						printf("Did you mean \"min\" or \"max\"?\n");
				}
				else
				{
					result = wsa_get_gain_if (dev, &temp_int);
					if (result >= 0)
						printf("Current IF gain: %d dB\n", temp_int);
				}
			} // end get GAIN IF

			else
				printf("Incorrect get GAIN. Specify RF or IF or see 'h'.\n");

		} // end get GAIN

		else if (strcmp(cmd_words[1], "PPB") == 0) 
		{
			if (num_words > 2)
				printf("Extra parameters ignored (max/min not available)!\n");

			result = wsa_get_packets_per_block(dev, &temp_int);
			if (result >= 0)
				printf("Current packets per block: %d\n", temp_int);
		} // end get PPB

		else if (strcmp(cmd_words[1], "SPP") == 0) 
		{
			if (num_words > 2) 
			{
				if (strcmp(cmd_words[2], "MAX") == 0)
					printf("Maximum samples per packet: %hu\n", 
						WSA4000_MAX_SAMPLES_PER_PACKET);
				else if (strcmp(cmd_words[2], "MIN") == 0)
					printf("Minimum samples per packet: %hu\n", 
						WSA4000_MIN_SAMPLES_PER_PACKET);
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else 
			{
				result = wsa_get_samples_per_packet(dev, &temp_int);
				if (result >= 0)
					printf("Current samples per packet: %d\n", temp_int);
			}
		} // end get SPP

		else if (strcmp(cmd_words[1], "ACQ") == 0) 
		{
			if (strcmp(cmd_words[2], "ACCESS") == 0)
			{
				result = wsa_system_request_acquisition_access(dev, &temp_short);
				if (result >= 0)
					printf("Data acquisition access %s.\n", (temp_short) ? "obtained" : "denied");
			}
			else
				printf("Did you mean \"access\"?\n");

		} //end get READ ACCESS

		else if (strcmp(cmd_words[1], "REF") == 0)
		{
			if (strcmp(cmd_words[2], "PLL") == 0)
			{
				if(num_words > 3)
					printf("Extra parameters ignored for 'get ref pll'.\n");

				result = wsa_get_reference_pll(dev, char_result);
				if (result >= 0)
					printf("Current reference PLL source: %s\n", char_result);
			}
		} //end get REF PLL


		else if (strcmp(cmd_words[1], "RF") == 0)
		{	
			if (strcmp(cmd_words[2], "LOCK") == 0)
			{
				if (strcmp(cmd_words[3], "STATUS") == 0)
				{
					if(num_words > 4)
						printf("Extra parameters ignored for 'get rf lock status'.\n");

					result = wsa_get_lock_rf(dev, &temp_int);
					if (result >= 0) 
						printf("RFE's PLL is %s.\n", (temp_int) ? "locked" : "not locked");
				}
			}
		} //end get RF LOCK STATUS
		
		else if (strcmp(cmd_words[1], "TRIGGER") == 0) 
		{
			if (strcmp(cmd_words[2], "MODE") == 0) 
			{
				result = wsa_get_trigger_type(dev, char_result);
				if (result >= 0)
					printf("Trigger mode: %s\n", char_result);
			} // end get TRIGGER MODE

			else if (strcmp(cmd_words[2], "LEVEL") == 0) 
			{
				result = wsa_get_trigger_level(dev, &start_freq, &stop_freq, &amplitude);
				if (result >= 0) 
				{
					printf("Trigger configuration:\n");
					printf("   Start frequency: %f MHz\n", (float) (start_freq / MHZ));
					printf("   Stop frequency: %f MHz\n", (float) (stop_freq / MHZ));
					printf("   Amplitude: %ld dBm\n", amplitude);
				}
			}
			else 
				printf("Usage: 'get trigger <level | mode>'");
			// end get TRIGGER LEVEL

		} // end get TRIGGER
		
		else if (strcmp(cmd_words[1], "SWEEP") == 0) 
		{
			if (strcmp(cmd_words[2], "ENTRY") == 0) 
			{
				if (strcmp(cmd_words[3], "SIZE") == 0) 
				{
					result = wsa_get_sweep_entry_size(dev, &temp_int);
					if (result >= 0) 
						printf("The sweep entry size is %d \n", temp_int);
				}
				else
					printf("Need a 4th parameter (\"size\"?). See 'h'.\n");

			}
			else 
				printf("Invalid 'get sweep' command. Try 'h'.\n");

		} // end get SWEEP

		else 
			printf("Invalid 'get' command. Try 'h'.\n");
	} // end GET
	
	else if (strcmp(cmd_words[0], "RUN") == 0) 
	{
		if (strcmp(cmd_words[1], "CMDF") == 0) 
		{
			if (strcmp(cmd_words[2], "") == 0) 
			{
				printf("Missing the syntax type and file name.\n");
			}
			else 
			{
				if (strcmp(cmd_words[3], "") == 0) 
				{
					printf("Missing the file name.\n");
				}
				else 
				{
					// TODO fix this declaration
					char *file_name = cmd_words[3];
					if (num_words > 3) 
					{
						for (i = 4; i < num_words; i++) 
						{
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

		else 
			printf("Invalid 'run' command, Try 'h'\n");
	} // end RUN


	//*****
	// Handle SET commands
	//*****
	else if (strcmp(cmd_words[0], "SET") == 0) 
	{
		if (strcmp(cmd_words[1], "ANT") == 0) 
		{
			if (strcmp(cmd_words[2], "") == 0) 
			{
				printf("Missing the antenna port value. See 'h'.\n");
			}
			else
			{
				input_veri_result = determine_if_int(cmd_words[2]);
				if (input_veri_result != 0)
					printf("Invalid input, antenna port must be a number.\n");
				else
				{
					result = wsa_set_antenna(dev, atoi(cmd_words[2]));
					if (result == WSA_ERR_INVANTENNAPORT)
						sprintf(msg, "\n\t- Valid ports: 1 to %d.", 
							WSA_RFE0560_MAX_ANT_PORT);
				}
			}
		} // end set ANT

		else if (strcmp(cmd_words[1], "BPF") == 0) 
		{
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_set_bpf_mode(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_set_bpf_mode(dev, 0);
			else 
				printf("Use 'on' or 'off' mode.\n");
		} // end set BPF

		else if (strcmp(cmd_words[1], "DEC") == 0) 
		{
			if (strcmp(cmd_words[2], "") == 0) 
			{
				printf("Missing the decimation rate. See 'h'.\n");
			}
			else
			{
				input_veri_result = determine_if_int(cmd_words[2]);
				if (input_veri_result != 0)
					printf("Invalid input, decimation value must be a number.\n");
				else
				{
					result = wsa_set_decimation(dev, (int32_t) atof(cmd_words[2]));
					if (result == WSA_ERR_INVDECIMATIONRATE)
						sprintf(msg, "\n\t- Valid range: %d to %d.",	// TODO #s
						dev->descr.min_decimation, dev->descr.max_decimation);
				}
			}
		} // end set decimation rate

		else if (strcmp(cmd_words[1], "FREQ") == 0) 
		{
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the frequency value. See 'h'.\n");

			else 
			{
				input_veri_result = determine_if_int(cmd_words[2]);
				if (input_veri_result != 0)
					printf("Invalid input, frequency value must be a number.\n");
				else
				{
					freq_value = (int64_t) (atof(cmd_words[2]) * MHZ);
					result = wsa_set_freq(dev, freq_value);
					if (result == WSA_ERR_FREQOUTOFBOUND)
						sprintf(msg, "\n\t- Valid range: %0.2lf to %0.2lf MHz.",
							(double) dev->descr.min_tune_freq / MHZ, 
							(double) dev->descr.max_tune_freq / MHZ);
				}
			}
		} // end set FREQ

		else if (strcmp(cmd_words[1], "FSHIFT") == 0) 
		{
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the frequency value. See 'h'.\n");
			else 
			{
				input_veri_result = determine_if_int(cmd_words[2]);
				if (input_veri_result != 0)
					printf("Invalid input, frequency shift value must be a number.\n");
				else
				{
					fshift = (float) (atof(cmd_words[2]) * MHZ);
					result = wsa_set_freq_shift(dev, fshift);
					if (result == WSA_ERR_FREQOUTOFBOUND)
						sprintf(msg, "\n\t- Valid range: %0.2f to %0.2f MHz.",
							0.0, (float) dev->descr.inst_bw / MHZ);
				}
			}
		} // end set FSHIFT

		else if (strcmp(cmd_words[1], "GAIN") == 0) 
		{
			if (strcmp(cmd_words[2], "RF") == 0) 
			{
				if (strcmp(cmd_words[3], "") == 0)
					printf("Missing the gain parameter, See 'h'.\n");

				else
				{
					// user can use medium as well as med
					if (strcmp(cmd_words[3], "MEDIUM") == 0)	
						sprintf(cmd_words[3], "MED");

					result = wsa_set_gain_rf(dev, cmd_words[3]);
				}

			} // end set GAIN RF

			else if (strcmp(cmd_words[2], "IF") == 0) 
			{
				if (strcmp(cmd_words[3], "") == 0) 
					printf("Missing the gain in dB value. See 'h'.\n");
				else if (!to_int(cmd_words[3], &temp_long)) 
				{

					result = wsa_set_gain_if(dev, (int32_t) temp_long);
					if (result == WSA_ERR_INVIFGAIN)
						sprintf(msg, "\n\t- Valid range: %d to %d dB.", 
							dev->descr.min_if_gain, dev->descr.max_if_gain);
				}
				else 
					printf("The IF gain value must be an integer. See 'h'.\n");
			} // end set GAIN IF
			
			else 
				printf("Incorrect set GAIN. Specify RF or IF. See 'h'.\n");
		} // end set GAIN

		else if (strcmp(cmd_words[1], "PPB") == 0) 
		{
			if (num_words < 3) 
				printf("Missing the packets per block value. See 'h'.\n");
			else if (!to_int(cmd_words[2], &temp_long))
				result = wsa_set_packets_per_block(dev,	(int32_t) temp_long);
			else
				printf("Invalid integer value. See 'h'.\n");
		} // end set PPB

		else if (strcmp(cmd_words[1], "SPP") == 0) 
		{
			if (num_words < 3) 
			{
				printf("Missing the samples per packet value. See 'h'.\n");
			}
			else 
			{
				if (!to_int(cmd_words[2], &temp_long)) 
				{
					result = wsa_set_samples_per_packet(dev, (int32_t) temp_long);
					if (result == WSA_ERR_INVSAMPLESIZE)
						sprintf(msg, "\n\t- Valid range: %hu to %hu.",
							WSA4000_MIN_SAMPLES_PER_PACKET,
							WSA4000_MAX_SAMPLES_PER_PACKET);
				}
				else
					result = WSA_ERR_INVNUMBER;
			}
		} // end set SPP

		else if (strcmp(cmd_words[1], "REF") == 0)
		{
			if (strcmp(cmd_words[2], "PLL") == 0) 
			{
				if (strcmp(cmd_words[3], "INT") == 0 || strcmp(cmd_words[3], "EXT") == 0)
				{
					if(num_words > 4)
						printf("Extra-parameters ignored in 'set ref pll'.\n");
					
					result = wsa_set_reference_pll(dev, cmd_words[3]);
				}
				else
					printf("Usage: 'set ref pll <ext | int>'\n");
			}

		}// end set REF PLL		
		
		else if (strcmp(cmd_words[1], "TRIGGER") == 0) 
		{
			if (strcmp(cmd_words[2], "MODE") == 0) 
			{
				if (num_words > 4) 
					printf("Extra parameters ignored in 'set trigger mode'.\n");
				else if (num_words == 4)
					result = wsa_set_trigger_type(dev, cmd_words[3]);
				else
					printf("Missing trigger type. See See 'h'. \n");				
			}

			else if (strcmp(cmd_words[2], "LEVEL") == 0) 
			{
				if (num_words < 4) 
					printf("Usage: 'set trigger level <start>,<stop>,<amplitude>'\n");
				else 
				{
					// Get the start freq
					strtok_result = strtok(cmd_words[3], ",");
					if (to_double(strtok_result, &temp_double) < 0) 
						printf("Start frequency must be a valid number\n");
					else 
					{
						start_freq = (int64_t) (temp_double * MHZ);						
						
						// Get the top frequency
						strtok_result = strtok(NULL, ",");
						if (to_double(strtok_result, &temp_double) < 0) 
							printf("Stop frequency must be a valid number\n");
						else 
						{
							stop_freq = (int64_t) (temp_double * MHZ);
							// Get the amplitude value
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0)
								printf("Amplitude must be a valid number\n");
							else
								result = wsa_set_trigger_level(dev, start_freq, stop_freq, (int32_t) temp_double);
						}
					}
				}
			}
			else
				printf("Usage: 'set trigger level <start,stop,amplitude>'\n"
				       "    or 'set trigger mode <level | none>'\n");
		}// end set TRIGGER
		
		else if (strcmp(cmd_words[1], "SWEEP") == 0) 
		{
			if (strcmp(cmd_words[2], "ENTRY") == 0) 
			{
				if (strcmp(cmd_words[3], "DWELL") == 0) 
				{
					if (num_words < 5)
						printf("Usage: 'set sweep entry dwell <seconds>,<microseconds>'\n");
					else 
					{
						strtok_result = strtok(cmd_words[4], ",");
						if (to_double(strtok_result, &temp_double) < 0) 
							printf("Dwell seconds value must be a valid number\n");
						else
						{
							dwell_seconds = (int32_t) temp_double;
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0) 
								printf("Dwell microseconds value must be a valid number\n");
							else
								result = wsa_set_sweep_dwell(dev, dwell_seconds, (int32_t) temp_double);
						}
					}
				}//end set SWEEP ENTRY DWELL

				else if (strcmp(cmd_words[3], "ANT") == 0) {
					if (strcmp(cmd_words[4], "") == 0) 
						printf("Missing the antenna port value. See 'h'.\n");
					else
					{
						input_veri_result = determine_if_int(cmd_words[4]);
						
						if (input_veri_result != 0)
							printf("Invalid input, antenna port must be a number\n");
						
						else
						{
							result = wsa_set_sweep_antenna(dev, atoi(cmd_words[4]));
					
							if (result == WSA_ERR_INVANTENNAPORT)
								sprintf(msg, "\n\t- Valid ports: 1 to %d.", 
								WSA_RFE0560_MAX_ANT_PORT);
						}
					}
				}
				//end set SWEEP ENTRY ANT
				else if (strcmp(cmd_words[3], "GAIN") == 0) {
					if (strcmp(cmd_words[4], "RF") == 0) {
						if (strcmp(cmd_words[5], "") == 0)
							printf("Missing the gain rf value. See 'h'.\n");
						else
						{
							// user can use medium as well as med
							if (strcmp(cmd_words[5], "MEDIUM") == 0)	
							sprintf(cmd_words[5], "MED");
							
							result = wsa_set_sweep_gain_rf(dev, cmd_words[5]);
						}
					} // end set SWEEP ENTRY GAIN RF

					else if (strcmp(cmd_words[4], "IF") == 0) 
					{
						if (strcmp(cmd_words[5], "") == 0) 
							printf("Missing the gain in dB value. See 'h'.\n");
						else if (!to_int(cmd_words[5], &temp_long)) 
						{
							result = wsa_set_sweep_gain_if(dev, (int32_t) temp_long);
							if (result == WSA_ERR_INVIFGAIN) 
								sprintf(msg, "\n\t- Valid range: %d to %d dB.", 
									dev->descr.min_if_gain, dev->descr.max_if_gain);
						}
						else 
							printf("The IF gain value must be an integer. See 'h'.\n");
					} // end set SWEEP ENTRY GAIN IF
				
					else
						printf("Incorrect set GAIN. Specify RF or IF. See 'h'.\n");
				} // end set SWEEP ENTRY GAIN
			
				else if (strcmp(cmd_words[3], "DEC") == 0) {
					if (strcmp(cmd_words[4], "") == 0) 
						printf("Missing the decimation rate. See 'h'.\n");
					
					input_veri_result = determine_if_int(cmd_words[4]);
						
					if (input_veri_result != 0)
						printf("Invalid input, decimation value must be a number.\n");
						
					else
					{
						result = wsa_set_sweep_decimation(dev,(int32_t) atof(cmd_words[4]));
						if (result == WSA_ERR_INVDECIMATIONRATE)
							sprintf(msg, "\n\t- Valid range: %d to %d.",	// TODO #s
									dev->descr.min_decimation, dev->descr.max_decimation);
					}
				} // end set SWEEP ENTRY DEC

				else if (strcmp(cmd_words[3], "TRIGGER") == 0) 
				{
					if (strcmp(cmd_words[4], "MODE") == 0) 
					{
						if (num_words == 6) 
							result = wsa_set_sweep_trigger_type(dev, cmd_words[5]);
						else
							printf("Missing trigger type. See See 'h'. \n");
					
					} // end set SWEEP ENTRY TRIGGER MODE 

					else if (strcmp(cmd_words[4], "LEVEL") == 0) 
					{
						if (num_words == 6) 
						{
							strtok_result = strtok(cmd_words[5], ",");
							if (to_double(strtok_result, &temp_double) < 0) 
							{
								printf("Error: Start frequency must be a valid number\n");
								return 0;
							}
							start_freq = (int64_t) (temp_double);
							
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0) 
							{
								printf("Error: Stop frequency must be a valid number\n");
								return 0;
							}
							stop_freq = (int64_t) (temp_double);
									
							strtok_result = strtok(NULL, ",");
							if (to_double(strtok_result, &temp_double) < 0)
							{
								printf("Error: Amplitude must be a valid number\n");
								return 0;
							}

							result = wsa_set_sweep_trigger_level(dev, start_freq, stop_freq, (int32_t) temp_double);
							
						}
						else 
							printf("Usage: 'set sweep trigger level <start>,<stop>,<amplitude>'\n");
					} 
					else 
						printf("Invalid 'set sweep entry trigger'. See 'h'.\n");
					
				// end set TRIGGER LEVEL
				}// end set trigger

				else if (strcmp(cmd_words[3], "FREQ") == 0) 
				{
					if (strcmp(cmd_words[4], "")  == 0) 
						printf("Missing frequency value. See 'h'.\n");
					else if (num_words != 5) 
						printf("Invalid 'set sweep entry freq'. See 'h'. \n");
					else 
					{
						strtok_result = strtok(cmd_words[4], ",");
						if (to_double(strtok_result, &temp_double) < 0) 
						{
							printf("Error: Start frequency must be a valid number\n");
							return 0;
						}
						start_freq = (int64_t) (temp_double);
						
						strtok_result = strtok(NULL, ",");
						if (to_double(strtok_result, &temp_double) < 0) 
						{
							printf("Error: Stop frequency must be a valid number\n");
							return 0;
						}
					stop_freq = (int64_t) (temp_double);
					result = wsa_set_sweep_freq(dev, start_freq * MHZ, stop_freq * MHZ);

					}
				} // end set SWEEP FREQ

				else if (strcmp(cmd_words[3], "FSHIFT") == 0) 
				{
					if (strcmp(cmd_words[4], "") == 0) 
						printf("Missing the frequency value. See 'h'.\n");
					else 
					{
						input_veri_result = determine_if_int(cmd_words[4]);
						
						if (input_veri_result != 0)
							printf("Invalid input, frequency shift value must be a number.\n");
						
						else
						{
							fshift = (float) (atof(cmd_words[4]) * MHZ);
							result = wsa_set_sweep_freq_shift(dev, fshift);
							if (result == WSA_ERR_FREQOUTOFBOUND)
								sprintf(msg, "\n\t- Valid range: %0.2f to %0.2f MHz.\n",
									0.0, (float) dev->descr.inst_bw / MHZ);
						}
					}

				} // end set FSHIFT

				else if (strcmp(cmd_words[3], "FSTEP") == 0) 
				{
					if (strcmp(cmd_words[4], "") == 0) 
					{
						printf("Missing the frequency step value. See 'h'.\n");
					}
					else 
					{
						input_veri_result = determine_if_int(cmd_words[4]);
						
						if (input_veri_result != 0)
							printf("Invalid input, frequency step must be a number.\n");
						
						else
						{
							freq_value = (int64_t) (atof(cmd_words[4]) * MHZ);
							result = wsa_set_sweep_freq_step(dev, freq_value);
							if (result == WSA_ERR_FREQOUTOFBOUND)
								sprintf(msg, "\n\t- Valid range: %0.2lf to %0.2lf MHz.\n",
									(double) dev->descr.min_tune_freq / MHZ, 
									(double) dev->descr.max_tune_freq / MHZ);
						}
					}

				} // end set FSTEP
				
				else if (strcmp(cmd_words[3], "PPB") == 0) 
				{
					if (num_words < 4)
						printf("Missing the packets per block value. See 'h'.\n");
					else if (!to_int(cmd_words[4], &temp_long)) 
						result = wsa_set_sweep_packets_per_block(dev,	
										(int32_t) temp_long);
					else
						printf("Invalid packets per block input.\n");
				} // end set PPB

				else if (strcmp(cmd_words[3], "SPP") == 0) 
				{
					if (num_words < 4) 
					{
						printf("Missing the samples per packet value. See 'h'.\n");
					}
					else if (!to_int(cmd_words[4], &temp_long)) 
					{
						result = wsa_set_sweep_samples_per_packet(dev, 
								(int32_t) temp_long);
						if (result == WSA_ERR_INVSAMPLESIZE)
							sprintf(msg, "\n\t- Valid range: %hu to %hu.\n",
								WSA4000_MIN_SAMPLES_PER_PACKET,
								WSA4000_MAX_SAMPLES_PER_PACKET);
					}
				} // end set SPP
			} // end set sweep ENTRY
			else 
			{
				printf("Invalid 'set sweep entry'. See 'h'.\n");
			}
		}	// end set SWEEP
		
		else 
		{
			printf("Invalid 'set'. See 'h'.\n");
		}
	} // end SET

	//*****
	// Handle SWEEP commands
	//*****
	else if  (strcmp(cmd_words[0], "SWEEP") == 0) 
	{
		if  (strcmp(cmd_words[1], "START") == 0) 
		{
			if (num_words == 2)			
			result = wsa_sweep_start(dev);

			else if (num_words == 3)
			{
				// make sure given sweep id is a positive integer
				if ((determine_if_int(cmd_words[2]) < 0) || (determine_if_unsigned(cmd_words[2]) < 0))
				{
					printf("Invalid input, sweep id must be a positive number.\n");	
					return 0;
				}
				else
					result =  wsa_sweep_start_id(dev,(int64_t) (atof(cmd_words[2])));
			}
			else
				printf("Invalid 'sweep start'. See 'h'.\n");
		} // end SWEEP START
		
		else if (strcmp(cmd_words[1], "STOP") == 0)
		{
			result = wsa_get_sweep_status(dev, char_result);
			if (strcmp(char_result, WSA4000_SWEEP_STATE_STOPPED) == 0)
				printf("Sweep mode is already disabled.\n");
			else
				result =  wsa_sweep_stop(dev);
		} // end SWEEP STOP

		else if (strcmp(cmd_words[1], "RESUME") == 0)
		{
			result =  wsa_sweep_resume(dev);
		} // end SWEEP RESUME
		
		else if (strcmp(cmd_words[1], "FLUSH") == 0) 
		{
			result = wsa_flush_data(dev);
		} // end SWEEP FLUSH
		
		else if (strcmp(cmd_words[1], "STATUS") == 0) 
		{
			result = wsa_get_sweep_status(dev, char_result);
			if (result >= 0) 
				printf("Sweep mode is %s.\n", char_result);
		} // end SWEEP STATUS  

		else if (strcmp(cmd_words[1], "ENTRY") == 0) 
		{
			if(strcmp(cmd_words[2], "COPY") == 0) 
			{
				if (num_words < 4)
					printf("Missing the ID of the entry. See 'h'.\n");
				else if (!to_int(cmd_words[3], &temp_long))
					result = wsa_sweep_entry_copy(dev, (uint32_t) temp_long);
				else
					printf("ID value must be an integer number.\n");

				
			} // end SWEEP ENTRY COPY

			else if (strcmp(cmd_words[2], "DELETE") == 0) 
			{
				if (num_words < 4)
					printf("Missing the entry ID or 'ALL'. See 'h'.\n");
				else if (strcmp(cmd_words[3], "ALL") == 0)
					result = wsa_sweep_entry_delete_all(dev);
				else if (!to_int(cmd_words[3], &temp_long))
					result = wsa_sweep_entry_delete(dev, (uint32_t) temp_long);
				else
					printf("ID value must be an integer number.\n");
			} // end SWEEP ENTRY DELETE

			else if (strcmp(cmd_words[2], "NEW") == 0) 
				result = wsa_sweep_entry_new(dev);    
			
			// end SWEEP ENTRY NEW

			else if (strcmp(cmd_words[2], "READ") == 0) 
			{
				if (num_words < 4)
					result = print_sweep_entry_template(dev);
				else if (!to_int(cmd_words[3], &temp_long))
					result = print_sweep_entry_information(dev, (int32_t) temp_long);
				else
					printf("ID value must be an integer number.\n");
			}
			else if (strcmp(cmd_words[2], "SAVE") == 0) 
			{
				if (num_words < 4)
					result = wsa_sweep_entry_save(dev, 0);
				else if (!to_int(cmd_words[3], &temp_long))
					result = wsa_sweep_entry_save(dev, (uint32_t) temp_long);
				else
					printf("ID value must be an integer number.\n");
					
			}
			else 
			{
				printf("Need a 3rd parameter. See 'h'.\n");
			}
		} // end ENTRY 
		
		else 
		{
			printf("Invalid 'sweep' command. See 'h'.\n");
		}
	} // end SWEEP

	//*****
	// Handle non-get/set commands
	//*****
	else {	
		if (strcmp(cmd_words[0], "DIR") == 0) 
		{
			if(num_words > 1)
				printf("Extra parameters ignored!\n");

			print_captures_directory();
		}
		
		else if ((strlen(cmd_words[0]) && (strspn(cmd_words[0], "H?") > 0)) ||
				(strstr(cmd_words[0], "HELP") != NULL)) 
		{
			print_cli_menu(dev);
		} // end print help

		else if (strcmp(cmd_words[0], "O") == 0) 
		{
			open_captures_directory();
		}  // end Open directory

		else if (strcmp(cmd_words[0], "SAVE") == 0) 
		{

			// fix the declaration here?
			char prefix[200];
			char ext[10];
			int n = 1;
			char *temp;

			strcpy(prefix, "");
			strcpy(ext, "csv");

			// Get the [name] &/or [ext:<type>] string if there exists one
			while (strcmp(cmd_words[n], "") != 0) 
			{
				unsigned int j;
				if (strstr(cmd_words[n], "EXT:") != NULL) 
				{
					temp = strchr(cmd_words[n], ':');
					strcpy(ext, strtok(temp, ":"));
					
					// convert to lower case
					for (j = 0; j < strlen(ext); j++)
						ext[j] = (char) tolower(ext[j]);		
				}
				else 
				{
					// convert to lower case after the first letter
					for (j = 1; j < strlen(cmd_words[n]); j++)
						cmd_words[n][j] = (char) tolower(cmd_words[n][j]);
					strcat(prefix, cmd_words[n]);
					strcat(prefix, " ");
				}

				n++;
			}

			if (strcmp(ext, "bin") == 0)
				result = save_data_to_bin_file(dev, prefix, ext);
			else
				result = save_data_to_file(dev, prefix, ext);
		} // end save data

		// User wants to run away...
		else if ((strcmp(cmd_words[0], "Q") == 0) || 
				(strstr(cmd_words[0], "QUIT") != NULL))
		{
			user_quit = TRUE;
		} // end quit

		// Keep going if nothing is entered
		else if (strcmp(cmd_words[0], "") == 0) 
		{
			// Do nothing
		}

		else
		{
			user_quit = -1;
		}
	} // End handling non get/set cmds.

	// Print out the errors
	if (result < 0) {
		
		if (result == WSA_WARNING_TRIGGER_CONFLICT)
			printf("WARNING %d: %s. %s\n", result, wsa_get_err_msg(result), msg);
		else
		{
			printf("ERROR %d: %s. %s\n", result, wsa_get_err_msg(result), msg);
			if (result == WSA_ERR_QUERYNORESP) {
				printf("Possibly due to loss of Ethernet connection.\n\n");
				user_quit = TRUE;
			}
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
	char temp_str[40];


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
	char in_str[40];	// store user's input string
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
		if ((strcmp(in_str, "Q") == 0) || 
			(strstr(in_str, "QUIT") != NULL)) 
			return 0;

		// User asked for help
		if ((strcmp(in_str, "H") == 0) || 
			(strcmp(in_str, "?") == 0) ||
			(strstr(in_str, "HELP") != NULL)) 
		{
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
    int64_t start_freq = 0;
	int64_t stop_freq = 0;
	int32_t amplitude = 0;
	int32_t enable = 0;
	char fw_ver[40];
	char trigger_mode[40];
	char gain[10];

	printf("\nCurrent WSA's statistics:\n");
	printf("\t- Current settings: \n");

	// TODO handle the errors
	result = wsa_get_fw_ver(dev, fw_ver);
	if (result < 0)
		printf("\t\t- Error: Failed getting the firmware version.\n");
	else
		printf("\t\t- Firmware Version: %s \n", fw_ver);

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

	result = wsa_get_gain_rf(dev, gain);
	
	if (result >= 0)
		printf("\t\t- Gain RF: %s\n",  gain);
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
	result = wsa_get_trigger_type(dev, trigger_mode);
	if (result >= 0)
			printf("\t\t\t- Trigger Mode: %s\n", trigger_mode);	
	else 
		printf("\t\t- Error: Failed reading the trigger mode.\n");

	
	result = wsa_get_trigger_level(dev, &start_freq, &stop_freq, &amplitude);
	if (result >= 0) {
		printf("\t\t\t- Start Frequency: %u\n", start_freq/MHZ);
		printf("\t\t\t- Stop Frequency: %u\n", stop_freq/MHZ);
		printf("\t\t\t- Amplitude: %ld dBm\n", amplitude);
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
 * @return 0 on success, or a negative number on error.
 * 
 */
int16_t print_sweep_entry_template(struct wsa_device *dev) 
{	
	int16_t result = 0;			// result returned from a function
	float fshift = 0;
	int32_t temp_int = 0;
	int32_t dwell_seconds;
	int32_t dwell_microseconds;
	int64_t freq = 0;
	int64_t start_freq;
	int64_t stop_freq;
	int32_t amplitude;
	char trigger_type[40];
	char temp[10];

	printf("Current Sweep Entry Template Settings:\n");

	// print frequency sweep value
	result = wsa_get_sweep_freq(dev, &start_freq, &stop_freq);
	if (result < 0)
		return result;
	printf("   Sweep range (MHz): %0.3f - %0.3f, ", (float) start_freq/MHZ, (float) stop_freq/MHZ);

	// print fstep sweep value
	result = wsa_get_sweep_freq_step(dev, &freq);
	if (result < 0)
		return result;
	printf("Step: %0.3f \n", (float) freq/MHZ);

	// print antenna sweep value
	result = wsa_get_sweep_antenna(dev, &temp_int);
	if (result < 0)
		return result;
	printf("   Antenna port: %d \n", temp_int);

	// print samples per packets sweep value
	result = wsa_get_sweep_samples_per_packet(dev, &temp_int);
	if (result < 0)
		return result;
	printf("   Capture block: %d * ", temp_int);

	// print packets per block
	result = wsa_get_sweep_packets_per_block(dev, &temp_int);
	if (result < 0)
		return result;
	printf("%d \n", temp_int);

	// print decimation sweep value
	result = wsa_get_sweep_decimation(dev, &temp_int);
	if (result < 0)
		return result;
	printf("   Decimation rate: %d \n", temp_int);

	// print fstep sweep value	
	result = wsa_get_sweep_freq_shift(dev, &fshift);
	if (result < 0)
		return result;
	printf("   Frequency shift: %0.3f MHz \n", (float) fshift/MHZ);

	// print gain if sweep value		
	result = wsa_get_sweep_gain_if(dev, &temp_int);
	if (result < 0)
		return result;
	printf("   Gain IF: %d dBm \n", temp_int);

	// print gain rf sweep value
	result = wsa_get_sweep_gain_rf(dev, temp);
	if (result < 0)
		return result;
	printf("   Gain RF: %s\n", temp);

	// print trigger status sweep value
	printf("   Trigger settings:\n");
	result = wsa_get_sweep_trigger_type(dev, trigger_type);
	if (result < 0)
		return result;
	printf("      Trigger mode: %s\n", trigger_type);

	// print trigger level sweep value
	result = wsa_get_sweep_trigger_level(dev, &start_freq, &stop_freq, &amplitude);
	if (result < 0)
		return result;
	printf("      Range (MHz): %0.3f - %0.3f\n", (float) start_freq, (float) stop_freq);
	printf("      Amplitude: %ld dBm\n", amplitude);

	// print dwell sweep value
	result = wsa_get_sweep_dwell(dev, &dwell_seconds, &dwell_microseconds);
	if (result < 0)
		return result;
	printf("   Dwell time: %u.%lu seconds\n", dwell_seconds, dwell_microseconds);

	return 0;
}

/**
 * Print the settings of the sweep entry in the wsa of the specified id
 * @param dev - a pointer to the wsa device
 * @param id - position of sweep entry
 * @return 0 on success, or a negative number on error
 */
int16_t print_sweep_entry_information(struct wsa_device *dev, int32_t id) 
{	
	int16_t result;
	struct wsa_sweep_list* list_values;
	
	list_values = (struct wsa_sweep_list*) malloc(sizeof(struct wsa_sweep_list));

	result = wsa_sweep_entry_read(dev, id, list_values);
	if (result < 0) 
	{
		free(list_values);
		return result;
	}
	printf("Sweep Entry %d Settings:\n", id);
	printf("  Sweep range (MHz): %0.3f - %0.3f, Step: %0.3f \n", ((float) list_values->start_freq / MHZ),(float)  (list_values->stop_freq / MHZ), (float) list_values->fstep / MHZ);
	printf("  Antenna port: %u \n", list_values->ant_port);
	printf("  Capture block: %d * %d \n", list_values->samples_per_packet, list_values->packets_per_block);
	printf("  Decimation rate: %d \n", list_values->decimation_rate);
	printf("  Frequency shift: %0.3f MHz\n", (float) list_values->fshift);
	printf("  Gain IF: %u \n", list_values->gain_if);
	printf("  Gain RF: %s\n", list_values->gain_rf);
	printf("  Trigger settings:\n");
    if (strcmp(list_values->trigger_type, "NONE") == 0) 
	{
		printf("    Trigger mode: %s\n",list_values->trigger_type);
	} 
	else
	{
		printf("    Trigger mode: %s\n",list_values->trigger_type);
		printf("       Range (MHz): %0.3f %0.3f\n", (float) (list_values->trigger_start_freq / MHZ),
													(float)  (list_values->trigger_stop_freq / MHZ));
		printf("       Amplitude: %ld dBm\n", list_values->trigger_amplitude);
	}
	printf("  Dwell time: %u.%lu seconds\n", list_values->dwell_seconds, list_values->dwell_microseconds);

	free(list_values);
	
	return 0;
}



