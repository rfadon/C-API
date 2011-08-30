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

#include "stdint.h"
#include "wsa4k_cli.h"
#include "wsa_api.h"
#include "wsa_error.h"


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

	printf("---------------------------\n");
	printf("\nCommand Options Available (case insensitive):\n\n");
	printf(" fp             - List the captured file path.\n");
	printf(" d              - Save data to a file.\n");
	printf(" h              - Show the list of available options.\n");
	printf(" o              - Open the folder of captured file(s).\n\n");

	printf(" get ant        - Show the current antenna port in used.\n");
	printf(" get bpf        - Show the current internal BPF state.\n");
	printf(" get cal        - Show the current calibration mode.\n");
	printf(" get cf         - Show the current running 'C'entre 'F'requency "
							 "(in MHz).\n");
	printf(" get fs         - Show the current 'F'rame 'S'ize per file.\n");
	printf(" get gl         - Show the current 'G'ain 'L'evel.\n");
	printf(" get lpf        - Show the current state of the anti-aliasing LPF.\n");
	printf(" get ss         - Show the current 'S'ample 'S'ize per frame.\n\n");

	printf(" set ant <#>    - Select the antenna switch 1 to 3.\n");
	printf(" set bpf <1/0>  - Turn the internal BPF on (1) or off (0).\n");
	printf(" set cal <1/0>  - Turn the calibration mode on (1) or off (0).\n");
	printf(" set cf <freq>  - Set the 'C'entre 'F'requency in MHz (ex: set "
							 "cf 2441.5).\nRange: %.2f - %.2f MHz inclusively."
							 "Resolution 10 kHz.\n", (double) MIN_FREQ / MHZ, 
							 (double) MAX_FREQ / MHZ);
	printf(" set fs <size>  - Set the 'F'rames 'S'ize per file (ex: set fs "
							 "1000). \nMaximum allows: %d.\n", MAX_FS);
	printf(" set gl <level> - Set 'G'ain 'L'evel.\n");
	printf("                 Options: HIGH, MEDIUM, LOW, ULTRALOW.\n");
	printf(" set lpf <1/0>  - Turn the anti-aliasing LPF on (1) or off (0).\n");
	printf(" set ss <size>  - Set the 'S'ize of 'S'amples to be captured "
							 "per frame\n(ex: set ss 2000).\n");
	printf("                 Maximum allows: %llu; Minimum: 1.\n\n", MAX_SS);
	printf(" Q              - 'Q'uit or exit this console.\n\n");
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
		printf("\n> Enter a command (or ':h'): ");

	// Conver the command to upper case.... <- should do this?
	while (((ch = toupper(getchar())) != EOF) && (ch != '\n'))
		input_opt[cnt_ch++] = (char)ch;
	input_opt[cnt_ch] = '\0';	// Terminate string with a null char

	return input_opt;
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
	uint8_t user_quit = FALSE;	// determine if user exits the CLI tool
	struct wsa_device wsa_dev;		// the wsa device structure
	struct wsa_device *dev;
	char intf_str[30];			// store the interface method string
	int16_t result = 0;			// result returned from a function
	uint64_t freq = 0;

	// Create the TCPIP interface method string
	sprintf(intf_str, "TCPIP::%s::%d", wsa_addr, HISLIP);

	// Start the WSA connection
	dev = &wsa_dev;
	if ((result = wsa_open(dev, intf_str)) < 0) {
		printf("ERROR: Failed to connect to the WSA at %s.\n", wsa_addr);
		doutf(1, "%s", wsa_get_err_msg(WSA_ERR_OPENFAILED));
		return WSA_ERR_OPENFAILED;
	}
	
	printf("Name: %s %llu\n", dev->descr.rfe_name, dev->descr.max_tune_freq);

// remove this section
	//TEST commands
	result = wsa_set_freq(dev, 2440000000);

	// Query the WSA status to make sure it is up & running
	freq = wsa_get_freq(dev);

// remove upto here


	//*****
	// Start the control or data acquisition loop
	//*****
	do {
		print_cli_menu(dev);
		break;
	} while (!user_quit);

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
	char *ip_list[MAX_BUF_SIZE];	// store potential WSA IP addresses
	const char *wsa_addr;			// store the desired WSA IP address
	int16_t result = 0;				// result returned from a function

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA4000 Command Line Interface Tool_____\n\n");

	do {
		//*****
		// Ask user to enter an IP address
		//*****
		printf("\n> Enter the WSA4000's IP (or type ':l'): ");
		strcpy(in_str, get_input_cmd(FALSE));

		// User wants to run away...
		if(strncmp(in_str, ":Q", 2) == 0) 
			break;

		// User asked for help
		if (strncmp(in_str, ":H", 2) == 0) {
			printf("Enter an IP address in the format #.#.#.# or host name ");
			printf("string.\nElse type: :l for a list to select "
				"from, :q to quit.\n");
			continue;
		}

		// User chose List option
		else if (strncmp(in_str, ":L", 2) == 0) {
			result = wsa_list(ip_list);
			printf("> ");
			strcpy(in_str, get_input_cmd(FALSE));
			in_num = atoi(in_str);

			if(in_num <= result && in_num > 0)
				wsa_addr = ip_list[in_num - 1];
			else {
				printf("Option invalid!\n");
				continue;
			}
		}

		// User has enter an address so verify first
		else if (strchr(in_str, '.') != 0) {
			// TODO verify & convert host type address to IP using 
			// wsa_get_host_info() -> needed?
			if (wsa_check_addr(in_str) > 0)
				wsa_addr = in_str;
			else {
				printf("\nInvalid address. Try again or ':h'.\n");
				continue;
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.# or host name format)\n");
			continue;
		}	

		//*****
		// All are good, start the connection
		//*****
		if(do_wsa(wsa_addr) == 0)
			break;
	} while (!user_quit);

	return 0;
}

