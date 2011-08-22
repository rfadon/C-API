#ifndef __WSA4K_CLI_H__
#define __WSA4K_CLI_H__


//#include "wsa_lib.h"
#include "wsa_api.h"

#define MAX_STR_LEN 200
#define MAX_BUF_SIZE 20

#define FALSE	0
#define TRUE	1

#define HISLIP 4880		/* Connection protocol's port to use with TCPIP */

extern uint8_t debug_mode;
extern uint8_t test_mode;


int32_t start_cli(void);


//void print_scpi_menu();

#endif
