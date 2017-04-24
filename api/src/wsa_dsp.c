#include "kiss_fft.h"
#include "thinkrf_stdint.h"
#include "wsa_lib.h"
#include "wsa_dsp.h"
#include "wsa_error.h"
#define _USE_MATH_DEFINES
#include "math.h"
#define ENOMEM 4
// ////////////////////////////////////////////////////////////////////////////
// Local Functions Section                                                   //
// ////////////////////////////////////////////////////////////////////////////
void find_average(int32_t array_size, kiss_fft_scalar * data_array, kiss_fft_scalar * average);
void find_average(int32_t array_size, kiss_fft_scalar * data_array, kiss_fft_scalar * average)
{
	kiss_fft_scalar sum = 0;
	int i = 0;

	for (i=0; i < array_size; i++)
	{
		sum = sum + data_array[i];
	}
	*average = sum / array_size;

}

kiss_fft_scalar normalize_scalar(kiss_fft_scalar value, kiss_fft_scalar maxval);
kiss_fft_scalar normalize_scalar(kiss_fft_scalar value, kiss_fft_scalar maxval)
{
	return value / maxval;
}

void get_normalization_factor(uint32_t const stream_id, kiss_fft_scalar  * normalization_factor);
void get_normalization_factor(uint32_t const stream_id, kiss_fft_scalar  * normalization_factor)
{

		if (stream_id == I32_DATA_STREAM_ID)
			*normalization_factor = 8388608;
		else
			*normalization_factor = 8192;
}

// ////////////////////////////////////////////////////////////////////////////
// Normalize Section                                                         //
// ////////////////////////////////////////////////////////////////////////////
/**
 * Normalize I or IQ data
 *
 * @samples_per_packet - the number of samples
 * @stream_id - the stream id which identifies the data format
 * @i16_buffer - buffer containing the 16-bit i data
 * @q16_buffer - buffer containing the 16-bit q data
 * @i32_buffer - buffer containing the 32-bit i data
 * @idata - buffer containing normalized i data
 * @qdata - buffer containing normalized q data
 */
void normalize_iq_data(int32_t samples_per_packet,
					uint32_t stream_id,
					int16_t * i16_buffer,
					int16_t * q16_buffer,
					int32_t * i32_buffer,
					kiss_fft_scalar * idata,
					kiss_fft_scalar * qdata)
{
	int i = 0;
	// retrieve the normalization factor based on the number of bits
	kiss_fft_scalar normalization_factor = 0;
	get_normalization_factor(stream_id, &normalization_factor);
	
	// normalize data, depending on whether data is I-only or IQ
	if (stream_id == I16Q16_DATA_STREAM_ID)
	{
		for (i=0; i<samples_per_packet; i++)
		{
			idata[i] = normalize_scalar( (kiss_fft_scalar) i16_buffer[i], normalization_factor);
			qdata[i] = normalize_scalar( (kiss_fft_scalar) q16_buffer[i], normalization_factor);
		}
	}
	else if (stream_id == I16_DATA_STREAM_ID)
	{
		for (i=0; i<samples_per_packet; i++)
		{
			
			idata[i] = ((float) i16_buffer[i]) / normalization_factor;
		}
	}
	else
	{
		for (i=0; i<samples_per_packet; i++)
			idata[i] = normalize_scalar( (kiss_fft_scalar) i32_buffer[i], normalization_factor);
	}

}
/**
 * Correct the DC offset
 *
 * @samples_per_packet - the number of samples
 * @idata - buffer containing normalized i data
 * @qdata - buffer containing normalized q data
 */
void correct_dc_offset(int32_t samples_per_packet,
					kiss_fft_scalar * idata,
					kiss_fft_scalar * qdata)
{
	kiss_fft_scalar i_average = 0;
	kiss_fft_scalar q_average = 0;
	int i = 0;
	find_average(samples_per_packet, idata, &i_average);
	find_average(samples_per_packet, qdata, &q_average);

	for (i=0; i < samples_per_packet; i++)
	{
		idata[i] = idata[i] / i_average;
		qdata[i] = qdata[i] / i_average;
	}


}
// ////////////////////////////////////////////////////////////////////////////
// Windowing Section                                                         //
// ////////////////////////////////////////////////////////////////////////////
/**
 * performs a hanning window on a scalar value
 *
 * @values - a pointer to the array of scalar values
 * @len - the length of the array that is being windowed
 * @index - the index of this item in the window
 * @returns the windowed value
 */
kiss_fft_scalar window_hanning_scalar(kiss_fft_scalar value, int len, int index)
{
	return (float) (value * 0.5 * (1 - cosf(2 * M_PI * index / (len - 1))));
}


/**
 * performs a hanning window on a list of scalar values, windowing is done in place
 *
 * @param values - a pointer to the array of scalar values
 * @param len - the length of the array
 */
void window_hanning_scalar_array(kiss_fft_scalar *values, int len)
{
	int i;

	for(i=0; i<len; i++) {
		values[i] = window_hanning_scalar(values[i], len, i);
	}
}


/**
 * performs a hanning window on a complex value in place
 *
 * @values - a pointer to the array of complex values
 * @len - the length of the array
 * @index - the index in the array we are windowing
 */
void window_hanning_cpx(kiss_fft_cpx *value, int len, int index)
{
	float mult =(float) (0.5 * (1 - cosf(2 * M_PI * index / (len - 1))));

	value->r = value->r * mult;
	value->i = value->i * mult;
}


/**
 * performs a spectral inversion on fft data
 *
 * @value - a pointer to the array of complex values
 * @len - the length of the array
 */
void reverse_cpx(kiss_fft_cpx *value, int len)
{
	int i;
	kiss_fft_cpx tmpval;

	for (i=0; i<(len/2); i++) {
		// copy current to tmp
		tmpval.r = value[i].r;
		tmpval.i = value[i].i;

		// copy opposite to current
		value[i].r = value[len - i - 1].r;
		value[i].i = value[len - i - 1].i;

		// copy tmp to opposite
		value[len - i - 1].r = tmpval.r;
		value[len - i - 1].i = tmpval.i;
	}
}

/**
 * performs a real fft on some scalar data
 *
 * @param idata - the real values to perform the FFT on
 * @param fftdata - the pointer to put the resulting fft data in
 * @param len - the length of the array
 * @returns negative on error, 0 on success
 */
int rfft(kiss_fft_scalar *idata, kiss_fft_cpx *fftdata, int len)
{
	int i, n;
	kiss_fft_cfg fftcfg;
	kiss_fft_cpx *iq;
	kiss_fft_cpx tmpval;

	iq = malloc(sizeof(kiss_fft_cpx) * len);
	if (iq == NULL) {
		fprintf(stderr, "error: out of memory during rfft alloc\n");
		return -ENOMEM;
	}

	// copy the real data into an complex iq array
	for (i=0; i<len; i++) {
		iq[i].r = idata[i];
		iq[i].i = 0;
	}

	fftcfg = kiss_fft_alloc(len, 0, 0, 0);
	kiss_fft(fftcfg, iq, fftdata);
	free(fftcfg);
	free(iq);

	// perform fft shift
	n = len >> 1;
	for (i=0; i<n; i++) {
		tmpval.r = fftdata[i].r;
		tmpval.i = fftdata[i].i;
		fftdata[i].r = fftdata[i + n].r;
		fftdata[i].i = fftdata[i + n].i;
		fftdata[i + n].r = tmpval.r;
		fftdata[i + n].i = tmpval.i;
	}

	// because we did a real fft, half the data is just an image.. get rid of it
	for (i=0; i<n; i++) {
		fftdata[i].r = fftdata[i+n].r;
		fftdata[i].i = fftdata[i+n].i;
	}

	return 0;
}

/**
 * converts a complex value to a power value
 *
 * @param value - the complex value to convert
 * @returns - the power value
 */
kiss_fft_scalar cpx_to_power(kiss_fft_cpx value)
{
	return (float) (sqrt((value.r * value.r) + (value.i * value.i)));
}


kiss_fft_scalar power_to_logpower(kiss_fft_scalar value) {
	return  (float) (10 * log10(value));
}

// ////////////////////////////////////////////////////////////////////////////
// Utility Functions                                                         //
// ////////////////////////////////////////////////////////////////////////////
/**
 * Find the peak value within the given spectral data
 * Note you must set the sample size, and acquire read status before 
 * calling this function
 * @param dev - A pointer to the WSA device structure.
 * @fstart- An unsigned 64-bit integer containing the start frequency
 * @fstop - An unsigned 64-bit integer containing the stop frequency
 * @rbw - A 64-bit integer containing the RBW value of the captured data (in Hz)
 * @data_size - The number of samples inside the spectral data array
 * @spectral_data - A floating point array containing the spectral data(in dBm)
 * @peak_freq - An unsigned 64-bit integer to store the frequency of the peak(in Hz)
 * @peak_power - A flloating point pointer to store the power level of the peak (in dBm)
 *
 * @return 0 on success or a negative value on error
 */
int16_t psd_peak_find(uint64_t fstart, 
				uint64_t fstop, 
				uint32_t rbw, 
				uint32_t data_size,
				float *spectra_data,
				uint64_t *peak_freq,
				float *peak_power)
{
	uint32_t i = 0;
	uint32_t random_holder = rbw;
	uint64_t current_freq = fstart;
	uint64_t rbw_cal = (fstop - fstart) / (uint64_t) data_size;
	*peak_power = spectra_data[i];
	*peak_freq = fstart;
	for (i = 0; i < data_size; i++){
		if (spectra_data[i] > *peak_power){
			*peak_power = spectra_data[i];
			
			*peak_freq = current_freq;
		}
		current_freq = current_freq + (uint64_t) rbw_cal;
		
	}
	i =  random_holder * 2;
	return 0;
}

/**
 * Calculate the channel power of a range of spectral data
 * @start_bin - The first bin where the channel power calculation should take place
 * @stop_bin - The last bin where the channel power calculation should take place
 * @data_size - The number of samples inside the spectral data array
 * @spectral_data - A floating point array containing the spectral data(in dBm)
 * @channel_power - A flloating point pointer to store the channel power(in dBm)
 *
 * @return 0 on success or a negative value on error
 */
int16_t psd_calculate_channel_power(uint32_t start_bin,
								uint32_t stop_bin,
								float *spectral_data,
								uint32_t data_size,
								float *channel_power)
{

	float linear_sum = 0;
	float tmp_float = 0;
	uint32_t i = 0;  

	// make sure that the stop bin is larger than the start bin
	if (start_bin >= stop_bin)
		return WSA_ERR_INVCHPOWERRANGE;
	
	// make sure that the stop bin is lower than the data size
	if (stop_bin > data_size)
		return WSA_ERR_INVCHPOWERRANGE;

	// find the linear sum of the squares
	for (i = 0; i < data_size; i++){
		if (i >= start_bin && i <= stop_bin){
			tmp_float = (float) (pow(10,  (spectral_data[i] / 20)));
			linear_sum = linear_sum + (tmp_float * tmp_float);
		}
	}
	*channel_power = (float) (10 * log10(linear_sum));
	return 0;
}

/**
 * Calculate the absolute power of spectral data
 * @start_bin - The first bin where the channel power calculation should take place
 * @stop_bin - The last bin where the channel power calculation should take place
 * @data_size - The number of samples inside the spectral data array
 * @spectral_data - A floating point array containing the spectral data(in dBm)
 * @absolute_power - A flloating point pointer to store the channel power(in dBm)
 *
 * @return 0 on success or a negative value on error
 */
int16_t psd_calculate_absolute_power(uint32_t start_bin,
								uint32_t stop_bin,
								float *spectral_data,
								uint32_t data_size,
								float *absolute_power)
{

	float linear_sum = 0;
	float tmp_float = 0;
	uint32_t i = 0;  

	// make sure that the stop bin is larger than the start bin
	if (start_bin >= stop_bin)
		return WSA_ERR_INVCHPOWERRANGE;
	
	// make sure that the stop bin is lower than the data size
	if (stop_bin > data_size)
		return WSA_ERR_INVCHPOWERRANGE;

	// find the linear sum of the squares
	for (i = 0; i < data_size; i++){
		if (i >= start_bin && i <= stop_bin){
			tmp_float = (float) (pow(10,  (spectral_data[i] / 20)));
			linear_sum = linear_sum + (tmp_float * tmp_float);
		}
	}
	*absolute_power = linear_sum;
	return 0;
}