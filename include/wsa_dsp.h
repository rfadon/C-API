
#include "kiss_fft.h"
#include "thinkrf_stdint.h"

kiss_fft_scalar normalize_scalar(kiss_fft_scalar value, kiss_fft_scalar maxval);
void normalize_scalar_array(int16_t *values, kiss_fft_scalar *results, int len, int16_t maxval);
void window_hanning_scalar_array(kiss_fft_scalar *values, int len);
void window_hanning_cpx(kiss_fft_cpx *value, int len, int index);
void reverse_cpx(kiss_fft_cpx *value, int len);
int rfft(kiss_fft_scalar *idata, kiss_fft_cpx *fftdata, int len);
kiss_fft_scalar cpx_to_power(kiss_fft_cpx value);
kiss_fft_scalar power_to_logpower(kiss_fft_scalar value);
