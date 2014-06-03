#include "PointerConverter.cpp"
#include "wsa_api.h"
#include "wsa_lib.h"
void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{

	if (nrhs != 2)
		mexErrMsgTxt("Need 2 parameter: WSA device pointer, number of samples per VRT packet");

    wsa_device* wsaDevice = convertMatlabToPointer(prhs[0]);
 
    int32_t* sampleSize = (int32_t*) mxGetData(prhs[1]);
    
    plhs[2] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
    uint8_t* headerPacketCount = (uint8_t*) mxGetData(plhs[2]);
    
    plhs[3] = mxCreateNumericMatrix(1, 1, mxUINT16_CLASS, mxREAL);
	uint16_t* headerSamplesPerPacket = (uint16_t*) mxGetData(plhs[3]);
    
    plhs[4] = mxCreateNumericMatrix(1, 1, mxINT8_CLASS, mxREAL);
    uint8_t* headerPacketType = (uint8_t*) mxGetData(plhs[4]);
	
    plhs[5] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* headerStreamId = (int32_t*) mxGetData(plhs[5]);
	
    plhs[6] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
	uint32_t* headerTimeStampSeconds = (uint32_t*) mxGetData(plhs[6]);
	
    plhs[7] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	uint64_t* headerTimeStampPicoSeconds = (uint64_t*) mxGetData(plhs[7]);
    
    plhs[8] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* receiverIndicatorField = (int32_t*) mxGetData(plhs[8]);
  
    plhs[9] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
    int32_t* receiverReferencePoint = (int32_t*) mxGetData(plhs[9]);

	plhs[10] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
	int64_t* receiverFrequency = (int64_t*) mxGetData(plhs[10]);

	plhs[11] = mxCreateNumericMatrix(1, 1, mxINT16_CLASS, mxREAL);
	int16_t* receiverGainIf = (int16_t*) mxGetData(plhs[11]);
	
    plhs[12] = mxCreateNumericMatrix(1, 1, mxINT16_CLASS, mxREAL);
	int16_t* receiverGainRf = (int16_t*) mxGetData(plhs[12]);
	
    plhs[13] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* receiverTemperature = (int32_t*) mxGetData(plhs[13]);
     
    plhs[14] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* digitizerIndicatorField = (int32_t*) mxGetData(plhs[14]);

	plhs[15] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
	int64_t* digitizerBandwidth = (int64_t*) mxGetData(plhs[15]);

	plhs[16] = mxCreateNumericMatrix(1, 1, mxINT16_CLASS, mxREAL);
	int16_t* digitizerReferenceLevel = (int16_t*) mxGetData(plhs[16]);
    
    plhs[17] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
	int64_t* digitizerRfFrequencyOffset = (int64_t*) mxGetData(plhs[17]);
    
    plhs[18] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
	int32_t* extensionIndicatorField = (int32_t*) mxGetData(plhs[18]);
    
    plhs[19] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
	uint32_t* extensionSweepId = (uint32_t*) mxGetData(plhs[19]);
    
    plhs[20] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
	uint32_t* extensionStreamId = (uint32_t*) mxGetData(plhs[20]);
       
    plhs[21] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
	uint8_t* trailerValidDataIndicator = (uint8_t*) mxGetData(plhs[21]);
    
    plhs[22] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
	uint8_t* trailerRefLockIndicator = (uint8_t*) mxGetData(plhs[22]);
    
    plhs[23] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
	uint8_t* trailerSpectralInversionIndicator = (uint8_t*) mxGetData(plhs[23]);

    plhs[24] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
	uint8_t* trailerOverRangeIndicator = (uint8_t*) mxGetData(plhs[24]);
    
    plhs[25] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
	uint8_t* trailerSampleLossIndicator = (uint8_t*) mxGetData(plhs[25]);
    
    plhs[26] = mxCreateNumericMatrix(*sampleSize, 2, mxINT16_CLASS, mxREAL);
	void* iq_data = mxGetData(plhs[26]);
 
    plhs[27] = mxCreateNumericMatrix(*sampleSize, 1, mxINT32_CLASS, mxREAL);
	void* i32_data = mxGetData(plhs[27]);
	
 	struct wsa_vrt_packet_header* header;
	struct wsa_vrt_packet_trailer* trailer;

   	struct wsa_receiver_packet* receiver;
	struct wsa_digitizer_packet* digitizer;
	struct wsa_extension_packet* extension;
	
	bool receivedData = false;
	int16_t resultStatus = 0;

    digitizer = (struct wsa_digitizer_packet*) malloc(sizeof(struct wsa_digitizer_packet));
    receiver = (struct wsa_receiver_packet*) malloc(sizeof(struct wsa_receiver_packet));
    extension = (struct wsa_extension_packet*) malloc(sizeof(struct wsa_extension_packet));
    header = (struct wsa_vrt_packet_header*) malloc(sizeof(struct wsa_vrt_packet_header));
    trailer = (struct wsa_vrt_packet_trailer*) malloc(sizeof(struct wsa_vrt_packet_trailer));
   
	while (!receivedData)
	{
        resultStatus = wsa_read_vrt_packet(wsaDevice, header, trailer, receiver, digitizer, extension, (int16_t*) iq_data, ((int16_t*) iq_data) +  ((uint16_t) *sampleSize),(int32_t*) i32_data,  *sampleSize);
		
        if (resultStatus != WSA_ERR_QUERYNORESP)
			receivedData = true;
	}
    *headerPacketCount = (uint8_t) header->pkt_count;
	*headerPacketType = (uint8_t) header->packet_type;
    *headerStreamId = (uint32_t)header->stream_id;
    *headerTimeStampSeconds = (uint32_t) header->time_stamp.sec;
    *headerTimeStampPicoSeconds = (uint64_t) header->time_stamp.psec; 
	
    if (*headerPacketType == 4) 
    {   
        *receiverIndicatorField = (int32_t) receiver->indicator_field;
        *receiverReferencePoint = receiver->reference_point;
        *receiverFrequency =  (int64_t) receiver->freq;
        *receiverGainIf = (int16_t) receiver->gain_if;
        *receiverGainRf = (int16_t) receiver->gain_rf;
        *digitizerBandwidth = (int64_t) digitizer->bandwidth;
        *digitizerReferenceLevel = (int16_t) digitizer->reference_level;
        *digitizerRfFrequencyOffset = (int64_t) digitizer->rf_freq_offset;
        *digitizerIndicatorField = (int32_t) digitizer->indicator_field;
        *extensionIndicatorField = (int32_t) extension->indicator_field;
        *extensionStreamId = (uint32_t) extension->stream_start_id;
        *extensionSweepId = (uint32_t) extension->sweep_start_id;
        
    } else if (*headerPacketType == 1) 
    {   
        *headerSamplesPerPacket = (uint16_t) header->samples_per_packet; 
        *trailerValidDataIndicator = (uint8_t) trailer->valid_data_indicator;
        *trailerRefLockIndicator = (uint8_t) trailer->ref_lock_indicator;
        *trailerSpectralInversionIndicator = (uint8_t) trailer->spectral_inversion_indicator;
        *trailerOverRangeIndicator = (uint8_t) trailer->over_range_indicator;
        *trailerSampleLossIndicator = (uint8_t) trailer->sample_loss_indicator;
    }
    free(receiver);
	free(digitizer);
	free(extension);
	free(trailer);
	free(header);

	plhs[0] = mxCreateDoubleScalar(resultStatus);
	
	if (resultStatus < 0)
		plhs[1] = mxCreateString(wsa_get_err_msg(resultStatus));
		
        else
            plhs[1] = mxCreateString("OK");

}

