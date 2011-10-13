#ifndef __WSA4K_CLI_H__
#define __WSA4K_CLI_H__

#include "stdint.h"

#define MAX_CMD_WORDS 50
#define MAX_STRING_LEN 500
#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"
#define DEFAULT_FS 100	// temp value

#define MAX_BUF_SIZE 20
#define MAX_ANT_PORT 2
#define MAX_FS 1000
#define MHZ 1000000

#define FALSE	0
#define TRUE	1

//#define HISLIP 4880		/* Connection protocol's port to use with TCPIP */

extern uint8_t debug_mode;
extern uint8_t call_mode;


int16_t start_cli(void);
int16_t process_call_mode(int32_t argc, char **argv);


#endif

