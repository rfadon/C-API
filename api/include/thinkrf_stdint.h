#ifndef _THINKRF_STDINT_H_
#define _THINKRF_STDINT_H_

/**
 * Exact-width integer types 
 */
#ifdef _WIN32
typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int					int32_t;	// same as for long
typedef unsigned int		uint32_t;	// same as for unsigned long
typedef long long			int64_t;
typedef unsigned long long	uint64_t;
#else
#include <stdint.h>
#include <sys/types.h>
typedef long long           wsa_int64_t;
typedef unsigned long long	wsa_uint64_t;
#define int64_t		        wsa_int64_t
#define uint64_t            wsa_uint64_t
#endif

#endif
	
