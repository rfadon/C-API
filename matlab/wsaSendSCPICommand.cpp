#include "PointerConverter.cpp"
#include "wsa_lib.h"


void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	if (nrhs != 2)
		mexErrMsgTxt("Need 2 parameter: WSA device pointer, and SCPI command");

	wsa_device* wsaDevice = convertMatlabToPointer(prhs[0]);
	char *command;

	command = mxArrayToString(prhs[1]);

    int16_t resultStatus =  wsa_send_scpi(wsaDevice, command);
    
    plhs[0] = mxCreateDoubleScalar(resultStatus);

	if (resultStatus < 0)
		plhs[1] = mxCreateString(wsa_get_err_msg(resultStatus));
	else
		plhs[1] = mxCreateString("OK");
}
