// *****************************************************************************
// Created by Jean Richard
// *****************************************************************************

#ifndef __WSA_DEBUG_H__
#define __WSA_DEBUG_H__

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 3
#endif

// Different debug levels
#define DNO 0
#define DHIGH 1
#define DMED 2
#define DLOW 3

#define WSA_API_LOG_FILE "wsa_api.log"
#define ENABLE_LOG_FILE 0

int doutf(int, const char *, ...);

#endif
