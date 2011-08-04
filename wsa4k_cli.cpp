// wsa4000_cli.cpp : Defines the entry point for the console application.

#include "wsa4k_cli.h"

using namespace std;

bool debug_mode = FALSE;
bool test_mode = FALSE;


int main(int argc, char* argv[])
{
	char inCmdStr[200];
	int count_arg = 1;
	int i, mode_argc = 0;
	

	// Check user commands for mode parameters
	if (argc > 1) {
		while (1) {
			// Copy the first command string arg to a constant string
			for (i = 0; i < ((int)strlen(argv[count_arg])); i++) 
				inCmdStr[i] = toupper(argv[count_arg][i]);	
			inCmdStr[i] = '\0';
			if (strncmp("-T",inCmdStr,2) == 0) test_mode = TRUE;
			else if (strncmp("-D",inCmdStr,2) == 0) debug_mode = TRUE;

			// up counter to the next argv positn
			if (count_arg < (argc - 1)) count_arg++;	
			else break;
		};
	}
	if (test_mode || debug_mode) mode_argc = 1;
	
	// Do we have enough command line arguments?
    if ((argc - mode_argc) < 2) {
        cerr << "usage: " << argv[0] << " <server-address> " <<
                "<client-address> [client-port]" << endl << endl;
        //cerr << "\tIf you don't pass client-port, it defaults to " <<
        //        defaultClientPort << "." << endl;
        return 1;
    }

	init_client(argc, argv);

	return 0;
}

