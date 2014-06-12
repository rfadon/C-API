#include <stdio.h>
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

//
//  This file is only for testing the matlab api outside of matlab
//  with code analyze tools
//
//  Define matlab constants to dummy values.
//


#define mxINT8_CLASS   0x12340001
#define mxINT16_CLASS  0x12340002
#define mxINT32_CLASS  0x12340003
#define mxINT64_CLASS  0x12340004

#define mxUINT8_CLASS  0x12350001
#define mxUINT16_CLASS 0x12350002
#define mxUINT32_CLASS 0x12350003
#define mxUINT64_CLASS 0x12350004

#define mxREAL         0x23450000

typedef void * mxArray;

inline mxArray * mxCreateNumericMatrix(int a, int b, int c, int d) { return 0; }

inline void * mxGetData(mxArray const * parray) { return 0; }

inline void mexErrMsgTxt(char const *) { }

inline char * mxArrayToString(mxArray const *) { return 0; }

inline mxArray * mxCreateDoubleScalar(int)     { return 0; }
inline mxArray * mxCreateString(char const *) { return 0; }
inline int mxGetString(mxArray const *, char *, int) { return 0; }

inline int    mxFree(void * ptr) { free(ptr); return 0;}
inline void * mxMalloc(int size) { return malloc(size); }
inline void mexMakeMemoryPersistent(void*) { }


#define mexFunction inline mexFunction
