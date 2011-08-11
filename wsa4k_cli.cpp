/**
 * @mainpage WSA4000 CLI Documentation
 * @par Overview:
 * Defines the entry point for the WSA4000 CLI console application.
 * @par
 * The WSA4000 CLI is designed using mixed C/C++ languages.
 *
 * @author QS.
 * @version v1.0
 * @date Aug. 2, 2011
 */

#include "wsa4k_cli.h"


/**
 * Print out the CLI options menu
 *
 * @return None
 */
void print_cli_menu(void)
{
	// Call to display SCPI menu or any other customized CLI menu
	// ex:
	//print_scpi_menu();
}


/**
 * Get input characters/string from the console and return the string when
 * the return key is pressed.
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
	int	cnt_ch=0;					// count # of chars entered	

	// Initialized the option
	ZeroMemory(input_opt, str_size);

	// Get command loop for string input terminated by "enter"
	if(pretext) printf("\n> Enter a command (or ':h'): ");
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
int32_t do_wsa(const char *wsa_addr)
{	
	uint8_t user_quit = FALSE;	// determine if user exits the CLI tool
	struct wsa_device wsa_dev;	// the wsa device structure
	struct wsa_resp query;		// store query results
	char intf_str[30];			// store the interface method string
	int result = 0;				// result returned from a function

	// Create the TCPIP interface method string
	sprintf(intf_str, "TCPIP::%s::%d", wsa_addr, HISLIP);
	
	// Start the WSA connection
	if (wsa_connect(&wsa_dev, SCPI, intf_str) < 0) {
		printf("ERROR: Failed to connect to the WSA at %s.\n", wsa_addr);
		return -1;
	}

	// Query the WSA status to make sure it is up & running
	//query = wsa_send_query(wsa_dev, char *command);

	//*****
	// Start the control or data acquisition loop
	//*****
	//do {

		//TODO do help option printing
		//print_cli_menu();

	//} while (!user_quit);

	return result;
}


/**
 * Start the CLI tool. First get a valid IP address from users, verify 
 * and start the WSA connection.
 * 
 * @return 0 if su
 */
int32_t start_cli(void) 
{
	uint8_t user_quit = FALSE;		// determine if user exits the CLI tool
	time_t dateStamp = time(NULL);	// use for display in the start of CLI
	char in_str[MAX_STR_LEN];		// store user's input string
	int16_t in_num = 0;				// store user's entered number
	char *ip_list[MAX_BUF_SIZE];	// store potential WSA IP addresses
	const char *wsa_addr;			// store the desired WSA IP address
	int32_t result = 0;				// result returned from a function

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA4000 Command Line Interface Tool_____\n\n");

	do {
	//*****
	// Ask user to enter an IP address
	//****
		printf("\n> Enter the WSA4000's IP (or type ':l'): ");
		strcpy(in_str, get_input_cmd(FALSE));

		// User wants to run away...
		if(strncmp(in_str, ":Q", 2) == 0) 
			break;

		// User asked for help
		if (strncmp(in_str, ":H", 2) == 0) {
			printf("Enter an IP address in the format #.#.#.# or a WWW ");
			printf("address string.\nElse type :l for a list to select from");
			continue;
		}

		// User chose List option
		else if (strncmp(in_str, ":L", 2) == 0) {
			result = list_ips(ip_list);
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
			wsa_addr = in_str;
			// TODO verify & convert www type address to IP using get_host_info()
			if ((result = verify_addr(wsa_addr)) == INADDR_NONE) {
				printf("\nInvalid address. Try again or ':q' to exit.\n");
				continue;
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.#)\n");
			continue;
		}

	} while (!user_quit);
	
	//*****
	// All are good, start the connection
	//*****
	result = do_wsa(wsa_addr);

	return 0;
}

