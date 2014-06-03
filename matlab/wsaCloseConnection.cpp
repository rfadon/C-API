#include "PointerConverter.cpp"

extern "C" {
	#include "wsa_debug.h"
}

void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	doutf(2, "In closeWSAConnection\n");
	doutf(3, "Called with %d input parameters and %d output parameters\n", nrhs, nlhs);
	
	if (nrhs != 1)
		mexErrMsgTxt("Need 1 parameter: WSA device pointer");
	
	wsa_device* wsaDevice = convertMatlabToPointer(prhs[0]);
	
	doutf(3, "Closing WSA connection\n");
	
	wsa_close(wsaDevice);
	mxFree(wsaDevice);
	
	doutf(1, "Freed WSA connection memory\n");
}
