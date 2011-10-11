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
 * The CLI interfaces to a WSA through the wsa_api library, which provides
 * functions to set/get particular settings or data from the WSA.  The wsa_api
 * encodes the commands into SCPI syntax scripts, which are sent to a WSA 
 * through the wsa_lib library.  Subsequently decodes any responses or packets
 * coming back from the WSA through the wsa_lib. \n \n
 *
 * The wsa_lib, thus, is the main gateway to a WSA box from a PC.  The 
 * wsa_lib has functions to open, close, send/receive commands, 
 * querry the WSA box status, and get data.  In this CLI version, wsa_lib 
 * calls the wsa_client's functions in the transport layer to establish TCP/IP 
 * specific connections.  Other connection methods such as USB could be 
 * added to the transport layer later on.  The wsa_lib, thus, abstracts 
 * away the interface method from any application/presentation program 
 * calling it.
 *
 * The CLI, hence, is a direct example of how the wsa_api library could be 
 * used.  VRT data packet will be decoded before saving into a file.
 *  
 * The WSA4000 CLI is designed using mixed C/C++ languages.
 * The CLI when executed will run in a Windows command prompt console. List 
 * of commands available with the CLI is listed in the print_cli_menu() 
 * function. \n \n
 *
 * @section limitation Limitations in v1.0
 * The following features are not yet supported with the CLI:
 *  - DC correction.  Need Nikhil to clarify on that.
 *  - IQ correction.  Same as above.
 *  - Automatic finding of a WSA box(s) on a network.
 *  - Set sample sizes. 1024 size for now.
 *  - Triggers.
 *  - Gain calibrarion. TBD with triggers.
 *  - USB interface method - might never be available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <malloc.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <direct.h>

#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_error.h"
#include "wsa_commons.h"

static int _frame_size = DEFAULT_FS;

//*****
// Local functions
//*****
int16_t process_cmd_string(struct wsa_device *dev, char *cmd_str);
int8_t process_cmd_words(struct wsa_device *dev, char *cmd_words[], 
					int16_t num_words);
void print_cli_menu(struct wsa_device *dev);
char* get_input_cmd(uint8_t pretext);
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
	float MAX_IF_GAIN = 0;	//TODO use this wsa_get_max_if_gain() ?
	float MIN_IF_GAIN = -39.0;		//TODO wsa_get_min_if_gain()
	uint32_t FREQ_RES = 10000;	//TODO

	printf("\n---------------------------\n");
	printf("\nCommand Options Available (case insensitive, < > required, "
		"[ ] optional):\n\n");
	printf(" h                      Show the list of available options.\n");
	printf(" o                      Open the folder of captured file(s).\n");
	printf(" q                      Quit or exit this console.\n");
	printf(" sd [name] [ext:<type>] Save data to a file with optional "
									"prefix string.\n"
		   "                        Output: [name] YYYY-MM-DD_HHMMSSmmm.[ext]\n"
		   "                        - ext type: csv (default), xsl, dat, ...\n"
		   "                        ex: 'sd Test trial ext:xsl' or 'sd'\n");
	printf("\n");

	printf(" get ant                Show the current antenna port in use.\n");
	printf(" get bpf                Show the current RFE's preselect BPF "
									"state.\n");
	printf(" get cal                Show the current RFE calibration mode.\n");
	printf(" get freq [max | min]   Show the current running centre frequency "
									"(in MHz).\n");
	printf(" get dir                List the captured file path.\n");
	printf(" get fs                 Show the current frame size per file.\n");
	printf(" get gain <rf | if [max | min]> \n"
		   "                        Show the current RF front end or IF gain "
									"level.\n");
	//printf(" get lpf                Show the current RFE's anti-aliasing"
	//								" LPF state.\n");
	printf(" get ss [max | min]     Show the current sample size per frame.\n"
									"\n");
	printf("\n");

	printf(" run cmdf <scpi | cli> <file name> \n"
		   "                        Run commands stored in a text file.\n"
		   "                        Note: Process only set or get commands.\n");
	printf("\n");

	printf(" set ant <1 | 2>        Select the antenna port, available 1 to "
									"%d.\n", MAX_ANT_PORT);
	printf(" set bpf <on | off>     Turn the RFE's preselect BPF stage on "
									"or off.\n");
	printf(" set cal <on | off>     Turn the calibration mode on or off.\n");
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
		   "                        (ex: set gain rf HIGH, set gain if -20.0).\n"
		   "                        - RF options: HIGH, MEDIUM, LOW, VLOW.\n"
		   "                        - IF range: %0.2lf to %0.2lf dB, inclusive."
									"\n", MIN_IF_GAIN, MAX_IF_GAIN);
	//printf(" set lpf <on | off>     Turn the RFE's anti-aliasing LPF stage "
	//								"on or off.\n");
	printf(" set ss <size>          Set the number of samples per frame to be "
									"captured\n"
		   "                        (ex: set ss 2000).\n"
		   "                        - Maximum allows: %d; Minimum: 1.\n\n", 
									MAX_SS);
}
// NOTE TO SELF: I can get & set all the values from Jean's lib!!! YAY!!!
// maybe except for fs & ss????


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
	while (((ch = toupper(getchar())) != EOF) && (ch != '\n'))
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
// TODO fix error code here
int16_t wsa_set_cli_command_file(struct wsa_device *dev, char *file_name) 
{
	int16_t result = 0;
	int16_t lines = 0;
	char *cmd_strs[MAX_FILE_LINES]; // store user's input words
	FILE *cmd_fptr;

	if((cmd_fptr = fopen(file_name, "r")) == NULL) {
		printf("Error opening '%s'.\n", file_name);
		return -1;
	}

	// Allocate memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		cmd_strs[i] = (char*) malloc(sizeof(char) * MAX_STRING_LEN);

	result = wsa_tokenize_file(cmd_fptr, cmd_strs);
	
	fclose(cmd_fptr);

	if (result < 0) {
		// free memory
		for (int i = 0; i < MAX_FILE_LINES; i++)
			free(cmd_strs[i]);
		return -1;
	}

	// Send each command line to WSA
	lines = result;
	for (int i = 0; i < lines; i++) {
		result = process_cmd_string(dev, cmd_strs[i]);
		Sleep(20); // delay the send a little bit
		
		// If a bad command is detected, continue? Prefer not.
		if (result < 0) {
			printf("Error at line %d: '%s'.\n", i + 1, cmd_strs[i]);
			break;
		}
	}
	result = lines;

	// Free memory
	for (int i = 0; i < MAX_FILE_LINES; i++)
		free(cmd_strs[i]);

	return result;
}


/**
 * Process any command (only) string.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param cmd_str - A char pointer for a command string.
 *
 * @return 1 for quit, 0 for no error or negative number if failed
 */
//TODO must catch/return some errors...
int16_t process_cmd_string(struct wsa_device *dev, char *cmd_str) 
{
	int16_t result = 0;
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words
	char temp_str[MAX_STRING_LEN];
	char *temp_ptr;
	uint8_t w = 0;	// an index

	// Allocate memory
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		cmd_words[i] = (char*) malloc(MAX_STRING_LEN * sizeof(char));

	// clear up the words first
	for (int i = 0; i < MAX_CMD_WORDS; i++)
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
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	return result;
}

/**
 * Save data to a file with the file name format as:
 * [prefix] YYYY-MM-DD_HH:MM:SS:mmm.[ext] if the prefix string and extension is
 * given.
 *
 * @param dev - A pointer to the WSA device structure.
 * @param prefix - A char pointer to a prefix string.
 * @param ext - A char pointer an extension string.
 *
 * @return 0 if successful, else a negative value.
 */
int16_t save_data_to_file(struct wsa_device *dev, char *prefix, char *ext)
{
	int16_t result;
	// for acquisition time
	time_t start_time;	// to capture run time
	int start_ms;		// the msec of start time
	int32_t delta_sec, delta_ms;	// time takes to run the data collection

	// *****
	// Get parameters to stored in the file
	// TODO smarter way of saving header is to allow users to enable
	// which header info to include...
	// *****

	printf("Gathering WSA settings... ");
	// Verify sample size
	int32_t samples = wsa_get_sample_size(dev);
	// TODO change this when set various ss is allowed
	if (samples < 1) {
		printf("Warning: bad sample size detected. Defaulting it to "
			"1024.\n");
		samples = 1024;
		result = wsa_set_sample_size(dev, samples);
		if (result < 0)
			return result;
	}

	// Get the centre frequency
	int64_t freq = wsa_get_freq(dev);
	if (freq < 0)
		return (int16_t) freq;


	// *****
	// Create the file name string
	// *****
	time_t time_stamp;
	struct tm *time_struct;
	char time_str[50];
	struct _timeb msec_buf;
	char file_name[MAX_STRING_LEN];
	FILE *iq_fptr;
	
	// create file name in format "[prefix] YYYY-MM-DD_HHMMSSmmm.[ext]" in a 
	// folder called CAPTURES
	time_stamp = time(NULL);	// call time functions
	time_struct = localtime(&time_stamp);
	_ftime_s(&msec_buf);		 
	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M%S", time_struct);
	sprintf(file_name, "CAPTURES\\%s%s%03d.%s", prefix, time_str, 
		msec_buf.millitm, ext);

	
	// create a new file for data capture
	if ((iq_fptr = fopen(file_name, "w")) == NULL) {
		printf("\nError creating the file \"%s\"!\n", file_name);
		return WSA_ERR_FILECREATEFAILED;
	}


	// *****
	// Create parameters and buffers to store the data
	// *****
	struct wsa_frame_header *header; 
	int fi = 0, next = 0;	// frame index and next index location
	char *d_buf;		// To store raw data bytes
	int16_t *i_buf;		// To store the integer I data
	int16_t *q_buf;		// To store the integer Q data
	int64_t total_bytes = 4 * _frame_size * samples;

	// Allocate buffer space
	header = (struct wsa_frame_header *) 
		malloc(sizeof(struct wsa_frame_header) * _frame_size);
	d_buf = (char *) malloc(sizeof(char) * total_bytes);
	i_buf = (int16_t *) malloc(sizeof(int16_t) * _frame_size * samples);
	q_buf = (int16_t *) malloc(sizeof(int16_t) * _frame_size * samples);

	
	// Collect the samples for all the _frame_size
	printf("done.\nAcquiring data bytes... ");
	start_time = time(0);			// time in seconds
	_ftime(&msec_buf);			// call time function
	start_ms = msec_buf.millitm;	// get millisecond
	while(fi < _frame_size) {
		next = fi * samples * 4;
		wsa_read_frame_raw(dev, &header[fi], &d_buf[next], samples);
		fi++;
	}
	delta_sec = (time(0) - start_time);
	_ftime_s(&msec_buf);			// call time function
	delta_ms = msec_buf.millitm - start_ms;
	printf("done. \n\t(Run time: %d sec %d msec;  Rate: %.03lf bytes/sec).\n", 
		delta_sec, delta_ms, 
		total_bytes / (delta_sec + (delta_ms / 1000.0)));

	// Decode all the samples
	printf("Decoding into I & Q... ");
	wsa_frame_decode(d_buf, i_buf, q_buf, _frame_size * samples);
	
	// *****
	// Save data to the file
	// *****
	// Loop to save data into the file
	printf("done.\nSaving data to: %s ... ", file_name);
	for (int j = 0; j < _frame_size; j++) {
		// Save each header information into the file 
		fprintf(iq_fptr, "#%d, cf:%ld, ss:%d, sec:%d, pico:%d\n", j + 1, freq, 
			samples, header[j].time_stamp.sec, header[j].time_stamp.psec);

		// Save decoded samples to the file	
		for (int i = 0; i < samples; i++) {
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
	float fl_result = 0;
	uint8_t user_quit = FALSE;	// determine if user has entered 'q' command


	//*****
	// Handle GET commands
	//*****
	if (strcmp(cmd_words[0], "GET") == 0) {
		if (strcmp(cmd_words[1], "ANT") == 0) {
			result = wsa_get_antenna(dev);
			if (result > 0)
				printf("Currently using antenna port: %d\n", result);
			else
				printf("Unknown port. Check setup for any error.\n");
		} // end get ANT 

		else if (strcmp(cmd_words[1], "BPF") == 0) {
			result = wsa_get_bpf(dev);
			if (result >= 0) {
				printf("RFE's preselect BPF state: ");
				if (result) printf("On\n");
				else if (!result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get BPF

		else if (strcmp(cmd_words[1], "CAL") == 0) {
			result = wsa_query_cal_mode(dev);
			if (result >= 0) {
				printf("RFE's calibration state: ");
				if (result) printf("On\n");
				else if (!result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get CAL

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

			freq = wsa_get_freq(dev);

			if (freq < 0)
					result = (int16_t) freq;
			else
				printf("Current centre frequency: %0.2f MHz\n", 
					(float) freq / MHZ);
		} // end get FREQ

		
		else if (strcmp(cmd_words[1], "DIR") == 0) {
			printf("File directory is: \"%s\\CAPTURES\\\"\n", 
				_getcwd(NULL, 0));
		}

		else if (strcmp(cmd_words[1], "FS") == 0) {
			printf("Current # of frames per file: %d\n", _frame_size);
		} // end get FS

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				result = wsa_get_gain_rf(dev);
				if (result >= 0) {
					printf("Current RF gain: ");
					switch(result) {
						case(WSA_GAIN_HIGH):	printf("HIGH"); break;
						case(WSA_GAIN_MEDIUM):	printf("MEDIUM"); break;
						case(WSA_GAIN_LOW):		printf("LOW"); break;
						case(WSA_GAIN_VLOW):	printf("VLOW"); break;
						default: printf("Unknown"); break;
					}
					printf("\n");
				}
			}  // end get GAIN RF

			else if (strcmp(cmd_words[2], "IF") == 0) {
				if (strcmp(cmd_words[3], "") != 0) {
					if (strcmp(cmd_words[3], "MAX") == 0) {
						printf("Maximum IF gain: %0.2f dB\n", 
							dev->descr.max_if_gain);
						return 0;
					}
					else if (strcmp(cmd_words[3], "MIN") == 0) {
						printf("Minimum IF gain: %0.2f dB\n", 
							dev->descr.min_if_gain);
						return 0;
					}
					else
						printf("Did you mean \"min\" or \"max\"?\n");
				}
				fl_result = wsa_get_gain_if (dev);

				// Here assume that there will be no gain less than -200 dB
				if (fl_result == WSA_ERR_QUERYNORESP || fl_result < -200)
					result = (int16_t) fl_result;
				else
					printf("Current IF gain: %0.2f dB\n", fl_result);
			} // end get GAIN IF

			else 
				printf("Incorrect get GAIN. Specify RF or IF or see 'h'.\n");
		} // end get GAIN

		//else if (strcmp(cmd_words[1], "LPF") == 0) {
		//	result = wsa_get_lpf(dev);
		//	if (result >= 0) {
		//		printf("RFE's anti-aliasing LPF state: ");
		//		if (result) printf("On\n");
		//		else if (!result) printf("Off\n");
		//		else printf("Unknown state\n");
		//	}
		//} // end get LPF

		else if (strcmp(cmd_words[1], "SS") == 0) {
			if (strcmp(cmd_words[2], "") != 0) {
				if (strcmp(cmd_words[2], "MAX") == 0) {
					printf("Maximum sample size: %lld\n", 
						dev->descr.max_sample_size);
					return 0;
				}
				else if (strcmp(cmd_words[2], "MIN") == 0) {
					printf("Minimum sample size: 1\n");
					return 0;
				}
				else
					printf("Did you mean \"min\" or \"max\"?\n");
			}
			else {
				int32_t size = 0;
				size = wsa_get_sample_size(dev);
				if (size < 1)
					result = (int16_t) size;
				else
					printf("The current sample size: %lld\n", size);
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
						for (int i = 4; i < num_words; i++) {
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
		} // end set ANT

		else if (strcmp(cmd_words[1], "BPF") == 0) {
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_set_bpf(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_set_bpf(dev, 0);
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

		else if (strcmp(cmd_words[1], "FREQ") == 0) {
			if (strcmp(cmd_words[2], "") == 0) {
				printf("Missing the frequency value. See 'h'.\n");
			}
			else {
				freq = (int64_t) (atof(cmd_words[2]) * MHZ);
				result = wsa_set_freq(dev, freq);
			}
		} // end set FREQ

		else if (strcmp(cmd_words[1], "FS") == 0) {
			int temp_fs;
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the frame size value. See 'h'.\n");
			else {
				temp_fs = atoi(cmd_words[2]);
				if (temp_fs < 1)
					printf("Invalid number for the frame size.\n");
				else 
					_frame_size = temp_fs;
			}
		} // end set FS

		else if (strcmp(cmd_words[1], "GAIN") == 0) {
			if (strcmp(cmd_words[2], "RF") == 0) {
				wsa_gain gain = (wsa_gain) NULL;
				uint8_t valid = TRUE;

				// Convert to wsa_gain type
				if (strstr(cmd_words[3], "HIGH") != NULL)
					gain = WSA_GAIN_HIGH;
				else if (strstr(cmd_words[3], "MEDIUM") != NULL)
					gain = WSA_GAIN_MEDIUM;
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
					printf("Missing the gain dB value. See 'h'.\n");
				}
				else
					result = wsa_set_gain_if (dev, (float) atof(cmd_words[3]));
			} // end set GAIN IF
			
			else {
				printf("Incorrect set GAIN. Specify RF or IF. See 'h'.\n");
			}
		} // end set GAIN

		//else if (strcmp(cmd_words[1], "LPF") == 0) {
		//	if (strcmp(cmd_words[2], "ON") == 0)
		//		result = wsa_set_lpf(dev, 1);
		//	else if (strcmp(cmd_words[2], "OFF") == 0)
		//		result = wsa_set_lpf(dev, 0);
		//	else 
		//		printf("Use 'on' or 'off' mode.\n");
		//} // end set LPF

		else if (strcmp(cmd_words[1], "SS") == 0) {
			// TODO HERE
			if (strcmp(cmd_words[2], "") == 0) 
				printf("Missing the sample size value. See 'h'.\n");
			
			int32_t sample_size = (int32_t) atof(cmd_words[2]);

			if (sample_size != 1024) {
				printf("Not supporting various sample sizes yet! "
					"Default to 1024.\n");
				sample_size = 1024;
			}
			result = wsa_set_sample_size(dev, sample_size);
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
			char dir[500];	// be generous b/c overflow will kill ur program.
			sprintf(dir, "explorer %s\\CAPTURES", _getcwd(NULL, 0));
			
			if (system(dir)!= NULL)
				printf("Open the folder of captured file(s)...\n");
			else 
				printf("Open failed!\n");
		}  // end Open directory

		else if (strcmp(cmd_words[0], "SD") == 0) {
			char prefix[200];
			char ext[10];
			int i = 1;
			char *temp;

			strcpy(prefix, "");
			strcpy(ext, "csv");

			// Get the [name] &/or [ext:<type>] string if there exists one
			while (strcmp(cmd_words[i], "") != 0) {
				unsigned int j;
				if (strstr(cmd_words[i], "EXT:") != NULL) {
					temp = strchr(cmd_words[i], ':');
					strcpy(ext, strtok(temp, ":"));
					
					// convert to lower case
					for (j = 0; j < strlen(ext); j++)
						ext[j] = tolower(ext[j]);
					//break; // break when reached the ext: line?			
				}
				else {
					// convert to lower case after the first letter
					for (j = 1; j < strlen(cmd_words[i]); j++)
						cmd_words[i][j] = tolower(cmd_words[i][j]);
					strcat(prefix, cmd_words[i]);
					strcat(prefix, " ");
				}

				i++;
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
	if (result < 0)
		printf("ERROR: %s.\n", wsa_get_err_msg(result));

	return user_quit;
}

/**
 * Setup WSA device variables, start the WSA connection and 
 *
 * @param wsa_addr - A char pointer to the IP address of the WSA
 *
 * @return
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
	sprintf(intf_str, "TCPIP::%s::%d", wsa_addr, HISLIP);

	// Start the WSA connection
	dev = &wsa_dev;
	if ((result = wsa_open(dev, intf_str)) < 0) {
		doutf(1, "Error WSA_ERR_OPENFAILED: %s.\n", 
			wsa_get_err_msg(WSA_ERR_OPENFAILED));
		return WSA_ERR_OPENFAILED;
	}


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
	char in_str[MAX_STRING_LEN];		// store user's input string
	char *in_buf;
	int16_t in_num = 0;				// store user's entered number
	//char *ip_list[MAX_BUF_SIZE];	// store potential WSA IP addresses
	const char *wsa_addr;			// store the desired WSA IP address
	int16_t result = 0;				// result returned from a function

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA Command Line Interface Tool_____\n\n");

	do {
		//*****
		// Ask user to enter an IP address
		//*****
		printf("\n> Enter the WSA4000's IP (or type 'l'): ");
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
			return 0; // break;

		// User asked for help
		//if (strspn(in_str, "H?") > 0) {
		if (strcmp(in_str, "H") == 0 || strcmp(in_str, "?") == 0) {
			printf("Enter an IP address in the format #.#.#.# or host name ");
			printf("string.\nElse type: 'l' for a list to select "
				"from, 'q' to quit.\n");
			continue;
		}

		// User chose List option
		else if (strcmp(in_str, "L") == 0) {
			printf("No list option for now. Pls enter IP address instead.\n");
			continue;
			/*
			result = wsa_list(ip_list);
			printf("> ");
			strcpy(in_str, get_input_cmd(FALSE));
			in_num = atoi(in_str);

			if (in_num <= result && in_num > 0)
				wsa_addr = ip_list[in_num - 1];
			else {
				printf("Option invalid!\n");
				continue;
			}*/
		}

		// User has enter an address so verify first
		else if (strchr(in_str, '.') != 0) {
			if (wsa_check_addr(in_str) > 0)
				wsa_addr = in_str;
			else {
				printf("\nInvalid address. Try again or 'h'.\n");
				continue;
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.# or host name format)\n");
			continue;
		}	

		//*****
		// All are good, start the connection & command part
		//*****
		if (do_wsa(wsa_addr) == 0)
			break;

	} while (!user_quit);
	
	free(in_buf);

	return 0;
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
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words
	int16_t w = 1;				// skip the executable at index 0
	int16_t c = 0;				// keep track of the words processed
	char temp_str[200];
	char intf_str[200];			// store the interface method string
	uint8_t cmd_end = FALSE, is_cmd = FALSE;	// some cmd words related flags
	uint8_t do_once = TRUE;

	int16_t result = 0;
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_device *dev;
	
	dev = &wsa_dev;

	// Allocate memory
	for (int i = 0; i < MAX_CMD_WORDS; i++) {
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
					result = WSA_ERR_INVIPADDRESS;
					break;
				}

				strcpy(temp_str, strchr(argv[w], '='));
				strcpy(temp_str, strtok(temp_str, "="));

				// Create the TCPIP interface method string
				sprintf(intf_str, "TCPIP::%s::%d", temp_str, HISLIP);

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
			for (int i = 0; i < MAX_CMD_WORDS; i++)
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
			printf("Warning: Omit '%s' not contained within { }.\n", argv[w]);
			w++;
		}

	//*****
	// Send the cmd words to be processed if there are any & tx
	//*****
		if (cmd_end) {
			strcpy(temp_str, "");
			for (int i = 0; i <= c; i++) {
				// Copy the words into one complete cmd string for display
				if (strcmp(cmd_words[i], "") != 0)
					strcat(temp_str, cmd_words[i]);
				if (i < c)
					strcat(temp_str, " ");

				// capitalized the words
				for (int j = 0; j < (int) strlen(cmd_words[i]); j++)
					cmd_words[i][j] = toupper(cmd_words[i][j]);
			}

			if (strlen(temp_str) > 0) {
				// there are cmds, so establish the connection but only once
				if (do_once) {
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
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	// if do_once is never started, print warning.
	if (do_once)
		printf("No command detected. See {h}.\n");
	else
		// finish so close the connection
		wsa_close(dev);

	return result;
}

