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
 * through the wsa_lib library.  Subsequently decodes any responses or packet
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

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <direct.h>

#include "stdint.h"
#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_error.h"

//*****
// Local functions
//
int8_t process_cmds(struct wsa_device *dev, char *cmd_words[], 
					int16_t num_words);
void print_cli_menu(struct wsa_device *dev);
char* get_input_cmd(uint8_t pretext);

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
	uint64_t MAX_SS = dev->descr.max_sample_size;
	float MAX_IF_GAIN = 0;	//TODO use this wsa_get_max_if_gain() ?
	float MIN_IF_GAIN = -39.0;		//TODO wsa_get_min_if_gain()
	uint32_t FREQ_RES = 10000;	//TODO

	printf("\n---------------------------\n");
	printf("\nCommand Options Available (case insensitive):\n\n");
	printf(" d                     Save data to a file.\n");
	printf(" fp                    List the captured file path.\n");
	printf(" h                     Show the list of available options.\n");
	printf(" o                     Open the folder of captured file(s).\n");
	printf(" q                     Quit or exit this console.\n\n");

	printf(" get ant               Show the current antenna port in use.\n");
	printf(" get bpf               Show the current RFE's preselect BPF "
									"state.\n");
	printf(" get cal               Show the current RFE calibration mode.\n");
	printf(" get cf                Show the current running centre frequency "
									"(in MHz).\n");
	printf(" get fs                Show the current frame size per file.\n");
	printf(" get gl <rf/if>        Show the current RF front end or IF gain "
									"level.\n");
	printf(" get lpf               Show the current RFE's anti-aliasing"
									" LPF state.\n");
	printf(" get ss                Show the current sample size per frame."
									"\n\n");

	printf(" set ant <1/2>         Select the antenna switch 1 to %d.\n",
									MAX_ANT_PORT);
	printf(" set bpf <on/off>      Turn the RFE's preselect BPF stage on "
									"or off.\n");
	printf(" set cal <on/off>      Turn the calibration mode on or off.\n");
	printf(" set cf <freq>         Set the centre frequency in MHz (ex: set "
									"cf 2441.5).\n"
		   "                       - Range: %.2f - %.2f MHz inclusively.\n"
		   "                       - Resolution %.2f MHz.\n", 
								   (float) MIN_FREQ/MHZ, (float) MAX_FREQ/MHZ,
								   (float) FREQ_RES/MHZ);
	printf(" set fs <size>         Set the frames size per file (ex: set fs "
									"1000). \n"
		   "                       - Maximum allows: %d.\n", MAX_FS);
	printf(" set gl <rf/if> <val>  Set gain level for RF front end or IF\n"
		   "                       (ex: set gl rf HIGH, set gl if -20.0).\n"
		   "                       - RF options: HIGH, MEDIUM, LOW, VLOW.\n"
		   "                       - IF range: %0.2lf to %0.2lf dB, inclusive."
									"\n", MIN_IF_GAIN, MAX_IF_GAIN);
	printf(" set lpf <on/off>      Turn the RFE's anti-aliasing LPF stage "
									"on or off.\n");
	printf(" set ss <size>         Set the number of samples per frame to be "
									"captured\n"
		   "                       (ex: set ss 2000).\n"
		   "                       - Maximum allows: %llu; Minimum: 1.\n\n", 
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
	const int str_size = MAX_STR_LEN; // Maximum characters in an option string
	char input_opt[str_size], ch;	// store user's option
	int	cnt_ch = 0;					// count # of chars entered	

	// Initialized the option
	//ZeroMemory(input_opt, str_size);
	strcpy(input_opt, "");

	// Get command loop for string input terminated by "enter"
	if (pretext) 
		printf("\n> Enter a command (or 'h'): ");

	// Conver the command to upper case.... <- should do this?
	while (((ch = toupper(getchar())) != EOF) && (ch != '\n'))
		input_opt[cnt_ch++] = (char)ch;
	input_opt[cnt_ch] = '\0';	// Terminate string with a null char

	return input_opt;
}

/**
 * Process any command (only) string.
 *
 * @param dev -
 * @param cmd_str - 
 *
 * @return 1 if 'q'uit is set, 0 for no error.
 */
int8_t process_cmds(struct wsa_device *dev, char **cmd_words, 
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

		else if (strcmp(cmd_words[1], "CF") == 0) {
			freq = wsa_get_freq(dev);
			if (freq < 0)
					result = (int16_t) freq;
			else
				printf("Current centre frequency: %0.2f MHz\n", 
					(float) freq / MHZ);
		} // end get CF

		else if (strcmp(cmd_words[1], "FS") == 0) {
			printf("TO BE IMPLEMENTED!");
		} // end get FS

		else if (strcmp(cmd_words[1], "GL") == 0) {
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
			}  // end get GL RF

			else if (strcmp(cmd_words[2], "IF") == 0) {
				fl_result = wsa_get_gain_if(dev);
				// Here assume that there will be no gain less than -200 dB
				if (fl_result < -200)
					result = (int16_t) fl_result;
				else
					printf("Current IF gain: %0.2f dB\n", fl_result);
			} // end get GL IF

			else 
				printf("Incorrect get GL. Specify RF or IF or see 'h'.\n");
		} // end get GL

		else if (strcmp(cmd_words[1], "LPF") == 0) {
			result = wsa_get_lpf(dev);
			if (result >= 0) {
				printf("RFE's anti-aliasing LPF state: ");
				if (result) printf("On\n");
				else if (!result) printf("Off\n");
				else printf("Unknown state\n");
			}
		} // end get LPF

		else if (strcmp(cmd_words[1], "SS") == 0) {
			printf("Not supporting various sample sizes yet! "
				"Default to 1024.\n");
		} // end get SS

		else {
			printf("Invalid 'get'. Try 'h'.\n");
		}
	} // end GET


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

		else if (strcmp(cmd_words[1], "CF") == 0) {
			if (strcmp(cmd_words[2], "") == 0) {
				printf("Missing the frequency value. See 'h'.\n");
			}
			else {
				freq = (int64_t) (atof(cmd_words[2]) * MHZ);
				result = wsa_set_freq(dev, freq);
			}
		} // end set CF

		else if (strcmp(cmd_words[1], "FS") == 0) {
			printf("TO BE IMPLIMENTED\n");
			//if (strcmp(cmd_words[2], "") == 0) 
			//	printf("Missing the frame size value. See 'h'.\n");
		} // end set FS

		else if (strcmp(cmd_words[1], "GL") == 0) {
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

				if(valid)
					result = wsa_set_gain_rf(dev, gain);
			} // end set GL RF

			else if (strcmp(cmd_words[2], "IF") == 0) {
				if (strcmp(cmd_words[3], "") == 0) {
					printf("Missing the gain dB value. See 'h'.\n");
				}
				else
					result = wsa_set_gain_if(dev, (float) atof(cmd_words[3]));
			} // end set GL IF
			
			else {
				printf("Incorrect set GL. Specify RF or IF. See 'h'.\n");
			}
		} // end set GL

		else if (strcmp(cmd_words[1], "LPF") == 0) {
			if (strcmp(cmd_words[2], "ON") == 0)
				result = wsa_set_lpf(dev, 1);
			else if (strcmp(cmd_words[2], "OFF") == 0)
				result = wsa_set_lpf(dev, 0);
			else 
				printf("Use 'on' or 'off' mode.\n");
		} // end set LPF

		else if (strcmp(cmd_words[1], "SS") == 0) {
			printf("Not supporting various sample sizes yet! "
				"Default to 1024.\n");
			//if (strcmp(cmd_words[2], "") == 0) 
			//	printf("Missing the sample size value. See 'h'.\n");
		} // end set SS

		else 
			printf("Invalid 'set'. See 'h'.\n");
	} // end SET


	//*****
	// Handle non-get/set commands
	//*****
	else {
		if (strcmp(cmd_words[0], "D") == 0) {
			printf("TO BE IMPLEMENTED.\n");
		}

		else if (strcmp(cmd_words[0], "FP") == 0) {
			printf("File directory is: \"%s\\CAPTURES\\\"\n", 
				_getcwd(NULL, 0));
		}

		else if (strlen(cmd_words[0]) == 1 && strspn(cmd_words[0], "H?") > 0) {
			print_cli_menu(dev);
		} // end print help

		else if (strcmp(cmd_words[0], "O") == 0) {
			char dir[200];
			sprintf(dir, "explorer %s\\CAPTURES", _getcwd(NULL, 0));
			
			if (system(dir)!= NULL)
				printf("Open the folder of captured file(s)...\n");
			else 
				printf("Open failed!\n");
		}  // end Open directory

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
		printf("ERROR: %s\n", wsa_get_err_msg(result));

	return user_quit;
}

/**
 * Setup WSA device variables, start the WSA connection and 
 *
 * @param wsa_addr
 *
 * @return
 */
int16_t do_wsa(const char *wsa_addr)
{	
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_device *dev;
	char intf_str[50];			// store the interface method string
	int16_t result = 0;			// result returned from a function
	
	uint8_t w = 0;	// an index
	char temp_str[MAX_STR_LEN];
	char *temp_ptr;
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words

	// Allocate memory
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		cmd_words[i] = (char*) malloc(MAX_STR_LEN * sizeof(char));

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
		// reset variables
		w = 0;

		// clear up the words first
		for (int i = 0; i < MAX_CMD_WORDS; i++)
			strcpy(cmd_words[i], "");
		
		// Get input string command
		strcpy(temp_str, get_input_cmd(TRUE));

		// Tokenized the string into words
		temp_ptr = strtok(temp_str, " \t\r\n");
		while (temp_ptr != NULL) {
			strcpy(cmd_words[w], temp_ptr);
			temp_ptr = strtok(NULL, " \t\r\n");
			w++;
		}
		
		// send cmd words to be processed
		result = process_cmds(dev, cmd_words, w);
		if (result < 0) 
			printf("Command '%s' not recognized.  See 'h'.\n", temp_str);
	} while (result != 1);

	// Free the allocation
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

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
	char in_str[MAX_STR_LEN];		// store user's input string
	int16_t in_num = 0;				// store user's entered number
	//char *ip_list[MAX_BUF_SIZE];	// store potential WSA IP addresses
	const char *wsa_addr;			// store the desired WSA IP address
	int16_t result = 0;				// result returned from a function

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA4000 Command Line Interface Tool_____\n\n");

	do {
		//*****
		// Ask user to enter an IP address
		//*****
		printf("\n> Enter the WSA4000's IP (or type 'l'): ");
		strcpy(in_str, get_input_cmd(FALSE));

		// prevent crashing b/c of strtok in the next line
		//if(strcmp(in_str, "") == 0)	continue;
		if(strtok(in_str, " \t\r\n") == NULL) continue;
		
		// remove spaces or tabs in string
		strcpy(in_str, strtok(in_str, " \t\r\n")); 

		// do nothing if nothing is entered
		if(strcmp(in_str, "") == 0) continue;

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

	return 0;
}

int16_t process_call_mode_words(int32_t argc, char **argv)
{
	char *cmd_words[MAX_CMD_WORDS]; // store user's input words
	int16_t w = 1;				// skip the executable arg, start at next word
	int16_t c = 0;				// keep track of the words processed
	char intf_str[50];			// store the interface method string
	char temp_str[50];
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_device *dev;
	int16_t result = 0;
	uint8_t cmd_end = FALSE, is_cmd = FALSE;	// some cmd words related flags

	// Allocate memory
	for (int i = 0; i < MAX_CMD_WORDS; i++) {
		cmd_words[i] = (char*) malloc(MAX_STR_LEN * sizeof(char));
		strcpy(cmd_words[i], "");
	}

	do {
	//*****
	// Process non-cmd words
	//*****
		if (!is_cmd && !cmd_end) {
			// ignore -c & -h command
			if(strcmp(argv[w], "-c") == 0 || strcmp(argv[w], "-h") == 0) {
				w++;
			}

			// Get the ip address and establish the connection if success
			if(strncmp(argv[w], "-ip=", 4) == 0) {
				if(strlen(argv[w]) <= 4) {
					printf("\nERROR: Invalid IP address or host name.\n");
					result = WSA_ERR_INVIPADDRESS;
					break;
				}

				strcpy(temp_str, strchr(argv[w], '='));
				strcpy(temp_str, strtok(temp_str, "="));

				// Create the TCPIP interface method string
				sprintf(intf_str, "TCPIP::%s::%d", temp_str, HISLIP);

				// Start the WSA connection
				dev = &wsa_dev;
				if ((result = wsa_open(dev, intf_str)) < 0) {
					printf("Error WSA_ERR_OPENFAILED: %s.", 
						wsa_get_err_msg(WSA_ERR_OPENFAILED));
					break;
				}

				w++;
			}
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
			}
			else {//if (argv[w][1] == '}')
				argv[w] = strtok(argv[w], "{");	
				printf("{: %s, cmd: null\n", argv[w]);
			}
		}

		// case {w only
		if (strchr(argv[w], '{') != NULL && strchr(argv[w], '}') == NULL){
			strcpy(cmd_words[c], strtok(argv[w], "{"));
			printf("{w: %s, cmd: %s\n", argv[w], cmd_words[c]);
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
			printf("w}: %s, cmd: %s\n", argv[w], cmd_words[c]);
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
			printf("}{: %s, cmd: %s\n", argv[w], cmd_words[c]);
			is_cmd = FALSE;
			cmd_end = TRUE;
		}

		// case within { w }
		else if (is_cmd) {
			strcpy(cmd_words[c], argv[w]);
			printf("w: %s, cmd: %s\n", argv[w], cmd_words[c]);
			w++;
			c++;
		}
		
		// words not within the bracket
		else {
			printf("Warning: Omits '%s' not contained within { }.\n", argv[w]);
			w++;
		}

	//*****
	// Send the cmd to be processed & tx
	//*****
		if (cmd_end) {
			printf("Command '");
			for (int i = 0; i <= c; i++) {
				if (i == c) {
					if (strlen(cmd_words[i]) == 0)
						printf("'");
					else
						printf("%s'", cmd_words[i]);
				}
				else
					printf("%s ", cmd_words[i]);

				// capitalized the words
				for(int j = 0; j < (int) strlen(cmd_words[i]); j++)
					cmd_words[i][j] = toupper(cmd_words[i][j]);
			}

			result = process_cmds(dev, cmd_words, c + 1);
			if (result < 0)
				printf(" not recognized.  See 'h'.\n");
			else
				printf(" processed.\n");

			// restart the flag
			cmd_end = FALSE;
			c = 0;
		}
	} while(w < argc);

	// Free the allocation
	for (int i = 0; i < MAX_CMD_WORDS; i++)
		free(cmd_words[i]);

	// finish so close the connection
	wsa_close(dev);

	return result;
}

