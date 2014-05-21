#include "PointerConverter.cpp"

void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	if (nrhs != 2)
		mexErrMsgTxt("Need 2 parameter: WSA device pointer, IP address");
	
	wsa_device* wsaDevice = convertMatlabToPointer(prhs[0]);
	
	char ipAddress[256];
	int errorCode = mxGetString((prhs[1]), ipAddress, 255);
	if (errorCode)
		mexErrMsgTxt("Invalid IP address parameter");

	char interfaceString[263];
	sprintf(interfaceString, "TCPIP::%s", ipAddress);
	int16_t openResult = wsa_open(wsaDevice, interfaceString);

	plhs[0] = mxCreateDoubleScalar(openResult);

	if (openResult < 0)
		plhs[1] = mxCreateString(wsa_get_err_msg(openResult));

	else
		plhs[1] = mxCreateString("OK");
    
    plhs[2] = mxCreateString(wsaDevice->descr.prod_model);
    plhs[3] = mxCreateString(wsaDevice->descr.fw_version);
    plhs[4] = mxCreateString(wsaDevice->descr.mac_addr);
	
    plhs[5] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	uint64_t* minFrequency = (uint64_t*) mxGetData(plhs[5]);

	plhs[6] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	uint64_t* maxFrequency = (uint64_t*) mxGetData(plhs[6]);

	plhs[7] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	uint64_t* instantaneousBandwidth = (uint64_t*) mxGetData(plhs[7]);

	plhs[8] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* minDecimationRate = (int32_t*) mxGetData(plhs[8]);

	plhs[9] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* maxDecimationRate = (int32_t*) mxGetData(plhs[9]);
	
    *minFrequency = wsaDevice->descr.min_tune_freq;
	*maxFrequency = wsaDevice->descr.max_tune_freq;
	*instantaneousBandwidth = wsaDevice->descr.inst_bw;
	*minDecimationRate = wsaDevice->descr.min_decimation;
	*maxDecimationRate = wsaDevice->descr.max_decimation;
}
