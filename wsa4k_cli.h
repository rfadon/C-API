#pragma once

#include "targetver.h"
#include <fstream>
#include <iostream>
#include <string.h>
//#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>	// to be used when passing unknown # or args
#include "ws-util.h"

#define FALSE	0
#define TRUE	1

extern bool debug_mode;
extern bool test_mode;


void print_scpi_menu();

int init_client(int argc, char* argv[]);


