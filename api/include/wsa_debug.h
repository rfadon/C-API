// *****************************************************************************
// Created by Jean Richard
// *****************************************************************************
  
#ifndef __WSA_DEBUG_H__
#define __WSA_DEBUG_H__

// Different debug levels
// DHIGH shows fewest messages; DLOW shows DHIGH, DMED and DLOW messages.
//
// DHIGH - Show Error messages
// DMED  - Show Errors and Commands
// DLOW  - Show Errors and Commands and low level communications.

#define DNO   0
#define DHIGH 1
#define DMED  2
#define DLOW  3

#ifndef DEBUGLEVEL
#define DEBUGLEVEL DMED
#endif

#define ENABLE_PRINTING 0

#define WSA_API_LOG_FILE "wsa_api.log"
#define ENABLE_LOG_FILE 1

int  wsa_doutf(int, const char *, ...);

#define doutf wsa_doutf


#endif
