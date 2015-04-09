#ifndef __COLOG_h__
#define __COLOG_h__


/*
 * based on https://github.com/thlorenz/log.h/blob/master/log.h
 * colour info taken from https://wiki.archlinux.org/index.php/Color_Bash_Prompt
 */


// these are font modifies that can be used
#define CF_NORMAL 0
#define CF_BOLD 1
#define CF_UNDERLINE 4
#define CF_INVERSE 0x8000

// codes to represent bright and dim colours
#define CA_NORMAL 30
#define CA_HIGHINT 90
#define CA_BACKGROUND 10

// internal colour codes
#define CC_BLACK  0
#define CC_RED    1
#define CC_GREEN  2
#define CC_YELLOW 3
#define CC_BLUE   4
#define CC_PURPLE 5
#define CC_CYAN   6
#define CC_WHITE  7

// colour codes for the user to use
#define C_BLACK  (CA_NORMAL + CC_BLACK)
#define C_BLACK  (CA_NORMAL + CC_BLACK)
#define C_DARKRED  (CA_NORMAL + CC_RED)
#define C_DARKGREEN  (CA_NORMAL + CC_GREEN)
#define C_DARKYELLOW  (CA_NORMAL + CC_YELLOW)
#define C_DARKBLUE  (CA_NORMAL + CC_BLUE)
#define C_DARKPURPLE  (CA_NORMAL + CC_PURPLE)
#define C_DARKCYAN  (CA_NORMAL + CC_CYAN)
#define C_DARKWHITE  (CA_NORMAL + CC_WHITE)
#define C_LIGHTRED  (CA_HIGHINT + CC_RED)
#define C_LIGHTGREEN  (CA_HIGHINT + CC_GREEN)
#define C_LIGHTYELLOW  (CA_HIGHINT + CC_YELLOW)
#define C_LIGHTBLUE  (CA_HIGHINT + CC_BLUE)
#define C_LIGHTPURPLE  (CA_HIGHINT + CC_PURPLE)
#define C_LIGHTCYAN  (CA_HIGHINT + CC_CYAN)
#define C_LIGHTWHITE  (CA_HIGHINT + CC_WHITE)

void colog(unsigned int, unsigned int, const char *, ...);

#endif
