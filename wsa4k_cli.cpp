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


char* get_input_cmd(uint8_t pretext)
{
	const int str_size = MAX_STR_LEN;	// Maximum characters in an option string
	char input_opt[str_size], ch;	// store user's option
	int	cnt_ch=0;				// count # of chars entered	

	// Initialized the option
	ZeroMemory(input_opt, str_size);

	//* Get command loop for string input terminated by "enter"
	if(pretext) printf("\n>> Enter a command (or 'h'): ");
	while (((ch = toupper(getchar())) != EOF) && (ch != '\n'))
		input_opt[cnt_ch++] = (char)ch;
	input_opt[cnt_ch] = '\0';	// Terminate string with a null char

	return input_opt;
}

int32_t start_cli(void) 
{
	time_t dateStamp = time(NULL);	// use for display
	char in_str[MAX_STR_LEN];
	uint8_t user_quit = FALSE;
	int32_t result;
	const char *wsa_addr;

	// Print some opening screen start messages:
	printf("%s\n",	asctime(localtime(&dateStamp)));
	printf("\t\t_____ThinkRF - WSA4000 Command Line Interface Tool_____\n\n");


	do {
		printf("\n>> Enter the WSA4000's IP (or type ':l'): ");
		strcpy(in_str, get_input_cmd(FALSE));

		if(strncmp(in_str, ":Q", 2) == 0) 
			break;

		if (strncmp(in_str, ":H", 2) == 0) {
			//TODO do help option printing
		}
		else if (strncmp(in_str, ":L", 2) == 0) {
			// TODO: detect all the IPs available to the PC...
			// Better yet... get only IP of WSA.
			//get_avail_ips();
			printf("\t1. 192.168.215.107\n");
			printf(">> ");
			strcpy(in_str, get_input_cmd(FALSE));
			if(atoi(in_str) == 1)
				wsa_addr = "192.168.215.107";
			else {
				printf("Option invalid!\n");
				continue;
			}
			get_host_by_name("192.168.215.107");
		}
		else if (strchr(in_str, '.') != 0) {
			wsa_addr = in_str;
			if ((result = lookup_addr(wsa_addr)) == INADDR_NONE) {
				printf("\nInvalid address. Try again or ':q' to exit.\n");
				continue;
			}
		}
		else {
			printf("Invalid IP address (Use: #.#.#.#)\n");
			continue;
		}
		
		result = init_client(wsa_addr);
	} while (!user_quit);


	return 0;
}