#include "mex.h"

extern "C" {
	#include "wsa_api.h"
	#include "wsa_error.h"
}

inline mxArray* convertPointerToMatlab(wsa_device* pointer)
{
    mxArray* matlabScalar = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	uint64_t* matlabScalarData = (uint64_t*) mxGetData(matlabScalar);
    *matlabScalarData = reinterpret_cast<uint64_t>(pointer);
    return matlabScalar;
}

inline wsa_device* convertMatlabToPointer(const mxArray *matlabScalar)
{
	uint64_t* matlabScalarData = (uint64_t*) mxGetData(matlabScalar);
    wsa_device* pointer = reinterpret_cast<wsa_device*>(*matlabScalarData);
    return pointer;
}

inline wsa_receiver_packet* convertMatlabreceiverToPointer(const mxArray *matlabScalar)
{
	uint64_t* matlabScalarData = (uint64_t*) mxGetData(matlabScalar);
    wsa_receiver_packet* pointer = reinterpret_cast<wsa_receiver_packet*>(*matlabScalarData);
    return pointer;
}

inline wsa_digitizer_packet* convertMatlabDigitizerToPointer(const mxArray *matlabScalar)
{
	uint64_t* matlabScalarData = (uint64_t*) mxGetData(matlabScalar);
    wsa_digitizer_packet* pointer = reinterpret_cast<wsa_digitizer_packet*>(*matlabScalarData);
    return pointer;
}
