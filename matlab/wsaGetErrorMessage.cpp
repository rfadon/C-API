#include "PointerConverter.cpp"

void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	if (nrhs != 1)
		mexErrMsgTxt("Need 1 parameter: Error ID");
	
	int16_t* erro_code = (int16_t*) mxGetData(prhs[0]);

   	plhs[0] = mxCreateString(wsa_get_err_msg(*erro_code));
}
