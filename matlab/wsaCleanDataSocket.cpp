#include "PointerConverter.cpp"
#include "wsa_lib.h"


void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	if (nrhs != 1)
		mexErrMsgTxt("Need 1 parameter: WSA device pointer");

	wsa_device* wsaDevice = convertMatlabToPointer(prhs[0]);

    int16_t resultStatus =  wsa_clean_data_socket(wsaDevice);
    
    plhs[0] = mxCreateDoubleScalar(resultStatus);

	if (resultStatus < 0)
		plhs[1] = mxCreateString(wsa_get_err_msg(resultStatus));
	else
		plhs[1] = mxCreateString("OK");
}
