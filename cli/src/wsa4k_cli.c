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
#include <sys/timeb.h>
#include <time.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_error.h"
#include "wsa_commons.h"

static int _frame_size = DEFAULT_FS;

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
	uint32_t MAX_SS = dev->descr.max_sample_size;
	int32_t MAX_IF_GAIN = dev->descr.min_if_gain;
	int32_t MIN_IF_GAIN = dev->descr.max_if_gain;
	uint64_t FREQ_RES = dev->descr.freq_resolution;

	printf("\n---------------------------\n");
	printf("\nCommand Options Available (case insensitive, < > required, "
		"[ ] optional):\n\n");
	printf(" h                      Show the list of available options.\n");
	printf(" o                      Open the folder of captured file(s).\n");
	printf(" q                      Quit or exit this console.\n");
	printf(" save [name] [ext:<type>] Save data to a file with optional "
									"prefix string.\n"
		   "                        Output: [name] YYYY-MM-DD_HHMMSSmmm.[ext]\n"
		   "                        - ext type: csv (default), xsl, dat, ...\n"
		   "                        ex: 'sd Test trial ext:xsl' or 'sd'\n");
	printf("\n");

	printf(" get ant                Show the current antenna port in use.\n");
	printf(" get bpf                Show the current RFE's preselect BPF "
									"state.\n");
	printf(" get cal                Show the current RFE calibration mode.\n");
	printf(" get dec [max | min]    Get the decimation rate (0 = off).\n");
	printf(" get freq [max | min]   Show the current running centre frequency "
									"(in MHz).\n");
	printf(" get dir                List the captured file path.\n");
	printf(" get fs                 Show the current frame size per file.\n");
	printf(" get gain <rf | if [max | min]> \n"
		   "                        Show the current RF front end or IF gain "
									"level.\n");
	printf(" get ss [max | min]     Show the current sample size per frame.\n"
									"\n");
	printf("\n");

	printf(" run cmdf <scpi | cli> <file name> \n"
		   "                        Run commands stored in a text file.\n"
		   "                        Note: Process only set or get commands.\n");
	printf("\n");

	printf(" set ant <1 | 2>        Select the antenna port, available 1 to "
									"%d.\n", WSA_RFE0560_MAX_ANT_PORT);
	printf(" set bpf <on | off>     Turn the RFE's preselect BPF stage on "
									"or off.\n");
	printf(" set cal <on | off>     Turn the calibration mode on or off.\n");
	printf(" set dec <rate>	        Set decimation rate.\n"
		   "                        - Range: 0 (for off), %d - %d.\n", 
		   dev->descr.min_decimation, dev->descr.max_decimation);
	printf(" set freq <freq>        Set the centre frequency in MHz (ex: set "
									"freq 441.5).\n"
		   "                        - Range: %.2f - %.2f MHz inclusively.\n"
		   "                        - Resolution %.2f MHz.\n", 
									(float) MIN_FREQ/MHZ, (float) MAX_FREQ/MHZ,
									(float) FREQ_RES/MHZ);
	printf(" set fs <size>          Set the frames size per file (ex: set fs "
									"1000). \n"
		   "                        - Maximum allows: %d.\n", MAX_FS);
	printf(" set gain <rf | if> <val> Set gain level for RF front end or IF\n"
		   "                        (ex: set gain rf HIGH; set gain if -20).\n"
		   "                        - RF options: HIGH, MEDium, LOW, VLOW.\n"
		   "                        - IF range: %d to %d dB, inclusive.\n", 
									MIN_IF_GAIN, MAX_IF_GAIN);
	printf(" set ss <size>          Set the number of samples per frame to be "
									"captured\n"
		   "                        (ex: set ss 2000).\n"
		   "                        - Maximum allows: %d; Minimum: 128.\n\n", 
									MAX_SS);
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
	int16_t result = 0;
	int32_t read_frame_result = 0;
	// for acquisition time
	time_t start_time;	// to capture run time
	uint16_t start_ms, delta_ms;	// the msec of start time
	time_t delta_sec;	// time takes to run the data collection
	int32_t samples;
	int64_t freq;
	time_t time_stamp;
	struct tm *time_struct;
	char time_str[50];
	char file_name[MAX_STRING_LEN];
	FILE *iq_fptr;

	TIME_HOLDER msec_buf;

	// *****
	// Create parameters and buffers to store the raw data
	// *****
	struct wsa_frame_header *header;
	char *d_buf;		// To store raw data bytes 
	int fi = 0, next = 0;	// frame index and next index location
	int frame_size = _frame_size;
	int32_t total_bytes;

	// *****
	// Create buffers to store the decoded I & Q from the raw data
	// *****
	int16_t *i_buf;		// To store the integer I data
	int16_t *q_buf;		// To store the integer Q data
	int i, j;

	// *****
	// Get parameters to stored in the file
	// TODO smarter way of saving header is to allow users to enable
	// which header info to include...
	// *****

	printf("Gathering WSA settings... ");
	// Verify sample size
	result = wsa_get_sample_size(dev, &samples);
	if (result < 0)
		return result;
	
	if (samples < 128 || samples > (int32_t) dev->descr.max_sample_size) {
		printf("Error: bad sample size detected. Please check the "
			"sample size. No data is saved.\n");
		return WSA_ERR_INVSAMPLESIZE;
	}

	// Get the centre frequency
	result = wsa_get_freq(dev, &freq);
	if (result < 0)
		return result;


	// *****
	// Create the file name string
	// *****

	
	
	// create file name in format "[prefix] YYYY-MM-DD_HHMMSSmmm.[ext]" in a 
	// folder called CAPTURES
	time_stamp = time(NULL);	// call time functions
	time_struct = localtime(&time_stamp);
	get_current_time(&msec_buf);

	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M%S", time_struct);
	sprintf(file_name, "CAPTURES\\%s%s%03d.%s", prefix, time_str, 
		msec_buf.millitm, ext);

	
	// create a new file for data capture
	if ((iq_fptr = fopen(file_name, "w")) == NULL) {
		printf("\nError creating the file \"%s\"!\n", file_name);
		return WSA_ERR_FILECREATEFAILED;
	}

	
	total_bytes = 4 * _frame_size * samples;
	

	// Allocate header buffer space
	header = (struct wsa_frame_header *) 
		malloc(sizeof(struct wsa_frame_header) * _frame_size);
	if (header == NULL)
	{
		return WSA_ERR_MALLOCFAILED;
	}

	// Allocate raw data buffer space
	d_buf = (char *) malloc(sizeof(char) * total_bytes);
	if (d_buf == NULL)
	{
		free(header);
		return WSA_ERR_MALLOCFAILED;
	}
	
	printf("done.\nAcquiring data bytes ");
	
	// Get the start time
	get_current_time(&msec_buf);
	start_time = msec_buf.time;	// time in seconds
	start_ms = msec_buf.millitm;// get millisecond
	
	// Collect the samples for all the _frame_size
	while(fi < frame_size) {
		//doutf(DLOW, "frame %d: ", fi + 1);
		next = fi * samples * 4;
		read_frame_result = wsa_read_frame_raw(dev, &header[fi], &d_buf[next], samples);
		if (read_frame_result < 1) {
			result = (int16_t) read_frame_result;
			frame_size = fi;
			printf("\nError detected while trying to get frame #%d.\n", 
				frame_size + 1);
			break;
		}
		printf(".");
		fi++;
	}
	// Determined the total bytes acquired
	total_bytes = 4 * frame_size * samples;

	// if there are no data to save, exit; else, save.
	if (result < 1) {
		if( total_bytes > 1)
			printf("Collected data will be saved... ");
		else {
			fclose(iq_fptr); 
			free(header);
			free(d_buf);
			return result;
		}
	}

	// get the end time & calculate the throughput rate
	get_current_time(&msec_buf);
	delta_sec = msec_buf.time - start_time;
	delta_ms = msec_buf.millitm - start_ms;
	// re-adjust the 1 second carry over when start_ms > end_ms
	if (start_ms > msec_buf.millitm) {
		delta_sec -= 1;
		delta_ms += 1000;
	}
	//printf("done.\n\t(Run time: %I64d sec %hu msec; Rate: %.03lf bytes/sec).\n", 
	printf("done.\n\t(Run time: %s sec %hu msec; Rate: %.03lf bytes/sec).\n", 
		ctime(&delta_sec), delta_ms, 
		total_bytes / (delta_sec + (delta_ms / 1000.0)));

	
	

	// Allocate i buffer space
	i_buf = (int16_t *) malloc(sizeof(int16_t) * _frame_size * samples);
	if (i_buf == NULL)
	{
		free(header);
		free(d_buf);
		return WSA_ERR_MALLOCFAILED;
	}
	
	// Allocate q buffer space
	q_buf = (int16_t *) malloc(sizeof(int16_t) * _frame_size * samples);
	if (q_buf == NULL)
	{
		free(header);
		free(d_buf);
		free(i_buf);
		return WSA_ERR_MALLOCFAILED;
	}

	// Decode all the samples
	printf("Decoding into I & Q... ");
	wsa_frame_decode(dev, d_buf, i_buf, q_buf, frame_size * samples);
	
	// *****
	// Save data to the file
	// *****
	// Loop to save data into the file
	printf("done.\nSaving data to: %s ... ", file_name);
	for (j = 0; j < frame_size; j++) {
		// Save each header information into the file 
		fprintf(iq_fptr, "#%d, cf:%lld, ss:%d, sec:%d, pico:%lld\n", j + 1, freq, 
			samples, header[j].time_stamp.sec, header[j].time_stamp.psec);

		// Save decoded samples to the file	
		for (i = 0; i < samples; i++) {
			next = j * samples + i;
			fprintf(iq_fptr, "%d,%d\n", i_buf[next], q_buf[next]);
		// For testing purpose only
		//	if ((i % 4) == 0) printf("\n");
		//	printf("%04x,%04x ", i_buf[next], q_buf[next]);
		}
	}
	printf("done.\n");

	fclose(iq_fptr); 
	free(header);
	free(d_buf);
	free(i_buf);
	free(q_buf);

	return 0;
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
	int int_result = 0;
	int8_t user_quit = FALSE;	// determine if user has entered 'q' command
	char msg[100];
	int i;
	int32_t rate;
	int32_t sample_size;
	int32_t if_gain_value;
	//DIR *temp;

	strcpy(msg,"");
	


	//*****
	// Handle GET commands
	//*****
	if (strcmp(cmd_words[0], "GET") == 0) {
		if (strcmp(cmd_words[1], "ANT") == 0) {
			result = wsa_get_antenna(dev, &int_result);
			if (result >= 0)
				printf("Currently using antenna port: %d\n", int_result);
		} // end get ANT 

		else if (strcmp(cmd_words[1], "BPF") == 0) {
			result = wsa_get_bpf_mode(dev, &int_result);
			if (result >= 0) {
				printf("RFE's preselect BPF state: ");
				if (int_result) printf("On\n");
				else if (!int_result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get BPF

		else if (strcmp(cmd_words[1], "CAL") == 0) {
			result = wsa_get_cal_mode(dev, &int_result);
			if (result >= 0) {
				printf("RFE's calibration state: ");
				if (int_result) printf("On\n");
				else if (!int_result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get CAL

		else if (strcmp(cmd_words[1], "DEC") == 0) {
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
			else {
				int32_t rate = 0;
				result = wsa_get_decimation(dev, &rate);
				if (result >= 0)
					printf("The current sample size: %d\n", rate);
			}
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
					(float) freq / MHZ);
		} // end get FREQ

		
		else if (strcmp(cmd_words[1], "DIR") == 0) {
			print_captures_directory();
		}

		else if (strcmp(cmd_words[1], "FS") == 0) {
			printf("Current # of frames per file: %d\n", _frame_size);
		} // end get FS

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				enum wsa_gain gain;
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

		else if (strcmp(cmd_words[1], "SS") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum sample size: %d\n", 
						dev->descr.max_sample_size);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum sample size: 128\n");
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else {
				int32_t size = 0;
				result = wsa_get_sample_size(dev, &size);
				if (result >= 0)
					printf("The current sample size: %d\n", size);
			}
		} // end get SS

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
					sprintf(msg, "\n\t- Valid ports: 1 to %d.", WSA_RFE0560_MAX_ANT_PORT);
		} // end set ANT

		else if (strcmp(cmd_words[1], "BPF") == 0) {
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_set_bpf_mode(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_set_bpf_mode(dev, 0);
			else 
				printf("Use 'on' or 'off' mode.\n");
		} // end set BPF

		else if (strcmp(cmd_words[1], "CAL") == 0) {
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_run_cal_mode(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_run_cal_mode(dev, 0);
			else 
				printf("Use 'on' or 'off' mode.\n");
		} // end set CAL

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

		else if (strcmp(cmd_words[1], "FS") == 0) {
			int temp_fs;
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the frame size value. See 'h'.\n");
			else {
				temp_fs = atoi(cmd_words[2]);
				if (temp_fs < 1)
					printf("Invalid number for the frame size."
						"\n\t- Valid range: 1 or larger\n");
				else 
					_frame_size = temp_fs;
			}
		} // end set FS

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				enum wsa_gain gain = (enum wsa_gain) NULL;
				uint8_t valid = TRUE;

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
				else if (!string_to_integer(cmd_words[3], &if_gain_value)) {
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

		else if (strcmp(cmd_words[1], "SS") == 0) {
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the sample size value. See 'h'.\n");
			
			sample_size = (int32_t) atof(cmd_words[2]);

			result = wsa_set_sample_size(dev, sample_size);
			if (result == WSA_ERR_INVSAMPLESIZE)
				sprintf(msg, "\n\t- Valid range: 128 to %d.",
					dev->descr.max_sample_size);
		} // end set SS

		else 
			printf("Invalid 'set'. See 'h'.\n");
	} // end SET


	//*****
	// Handle non-get/set commands
	//*****
	else {
		if (strlen(cmd_words[0]) == 1 && strspn(cmd_words[0], "H?") > 0) {
			print_cli_menu(dev);
		} // end print help

		else if (strcmp(cmd_words[0], "O") == 0) {
			open_captures_directory();
		}  // end Open directory

		else if (strcmp(cmd_words[0], "SAVE") == 0) {
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
			printf("Possibly due to lost of Ethernet connection.\n\n");
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
 * 
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
 *
 * @param argc - Integer number of argument words
 * @param argv - Pointer to pointer of characters
 *
 * @return 0 if success, negative number if failed
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
	} while(w < argc);

	// Free the allocation
	for (i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	// if do_once is never started, print warning.
	if (do_once && has_ipstr)
		printf("No command detected. See {h}.\n");
	else
		// finish so close the connection
		wsa_close(dev);

	return result;
}


/**
 * Print out some statistics of the WSA's current settings
 *
 * @param dev - A pointer to the WSA device structure.
 *
 * @return 0
 */
void print_wsa_stat(struct wsa_device *dev) {
	int16_t result;
	int64_t freq;
	int32_t value;
	enum wsa_gain gain;

	printf("\nCurrent WSA's statistics:\n");
	printf("\t- Firmware version: %s\n", dev->descr.fw_version);
	printf("\t- Current settings: \n");

	// TODO handle the errors
	result = wsa_get_freq(dev, &freq);
	if (result >= 0)
		printf("\t\t- Frequency: %0.3lf MHz\n", (float) freq / MHZ);
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

	result = wsa_get_sample_size(dev, &value);
	if (result >= 0)
		printf("\t\t- Sample size: %d\n", value);
	else
		printf("\t\t- Error: Failed getting the sample size.\n");

	printf("\t\t- Frame size per file: %d\n", _frame_size);
}


/**
 * Convert a gain RF setting to a string, useful for printing
 * 
 * @param gain - the enum'ed gain value to be converted into a string name
 * @param gain_str - a char pointer to store the string name of the given gain
 * 
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
