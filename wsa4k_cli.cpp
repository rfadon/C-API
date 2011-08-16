/**
 * @mainpage Introduction
 *
 * This documentation, compiled using Doxygen, shows in details the code
 * structure of the CLI (Command Line Interface) tool. \n \n
 * The following diagram illustrates the different layers involved in 
 * interfacing with a WSA on the PC side.  
 *
 * @image html wsa4000_cli_2.PNG
 * @image latex wsa4000_cli_2.PNG "Interface Layers to WSA on PC Side" width=15cm
 *
 * The wsa_lib is the main gateway to a WSA box in the application/
 * presentation layer, in which the CLI tool would belong.  The wsa_lib has, 
 * in brief, functions to open, close, send/receive commands, querry the WSA 
 * box status, and gets data.  In this CLI version, wsa_lib calls the 
 * wsa_client's functions in the transport layer to establish TCP/IP specific 
 * connections.  Other connection methods such as USB could be added to the 
 * transport layer.  The wsa_lib, then, abstracts away the interface 
 * method from any application/presentation level calling it.
 * The CLI, hence, is a direct example of how the wsa_lib library could be 
 * used.
 * 
 * The WSA4000 CLI is designed using mixed C/C++ languages.
 * The CLI when executed will run in a Windows command promt console.
 * 
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
	ZeroMemory(input_opt, str_size);

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
	if ((result = wsa_connect(&wsa_dev, SCPI, intf_str)) < 0) {
		printf("ERROR: Failed to connect to the WSA at %s.\n", wsa_addr);
		return -1;
	}

	//TEST send command
	result = wsa_send_command(&wsa_dev, "Start Message\0");

	// Query the WSA status to make sure it is up & running
	query = wsa_send_query(&wsa_dev, "QUERY:Test query\n");
	if (query.status > 0)
		result = query.status;
	printf("Query: received %d bytes.\n", result);

	//*****
	// Start the control or data acquisition loop
	//*****
	//do {

		//TODO do help option printing
		//print_cli_menu();

	//} while (!user_quit);

	// send stop flag to server
	result = wsa_send_command(&wsa_dev, "STOPALL");//, strlen("STOPDATA"));

	wsa_close(&wsa_dev);

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
		//*****
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
			result = wsa_list_ips(ip_list);
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
			// TODO verify & convert www type address to IP using wsa_get_host_info()
			if ((result = wsa_verify_addr(wsa_addr)) == INADDR_NONE) {
				printf("\nInvalid address. Try again or ':q' to exit.\n");
				continue;
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.#)\n");
			continue;
		}	

		//*****
		// All are good, start the connection
		//*****
		result = do_wsa(wsa_addr);
	} while (!user_quit);


	return 0;
}

