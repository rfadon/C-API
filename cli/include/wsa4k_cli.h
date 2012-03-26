#ifndef __WSA4K_CLI_H__
#define __WSA4K_CLI_H__

#include "thinkrf_stdint.h"
#include "wsa4k_cli_os_specific.h"

#define MAX_CMD_WORDS 50
#define MAX_STRING_LEN 500
#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"
#define DEFAULT_FS 100	// temp value

#define MAX_BUF_SIZE 20
#define MAX_FS 1000

#define CTRL_PORT 37001	/* Connection protocol's port to use with TCPIP */
#define DATA_PORT 37000

#define FALSE	0
#define TRUE	1


extern uint8_t debug_mode;
extern uint8_t call_mode;


int16_t start_cli(void);
void call_mode_print_help(char* argv);
int16_t process_call_mode(int32_t argc, char **argv);
void get_current_time(TIME_HOLDER* msec_buf);
void print_captures_directory();
void open_captures_directory();


#endif

