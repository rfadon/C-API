#include <stdlib.h>

#include "PointerConverter.cpp"
#include "matrix.h"

void mexFunction(int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])
{
    if (nlhs != 1) {
        mexErrMsgTxt("This function must return 1 output");
    }
    
    wsa_device* wsaDevice = (wsa_device*) mxMalloc(sizeof(wsa_device));
    mexMakeMemoryPersistent(wsaDevice);
    plhs[0] = convertPointerToMatlab(wsaDevice);
}
