#ifndef _fft_h_
#define _fft_h_

//#define fftDBL_PRECISION

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	fftDirect = 1,
	fftInverse = -1
	
} fftDirection;

// Replaces data[1..2*len] by its discrete Fourier transform, if direction is fftDirect; or replaces
// data[1..2*len] by nn times its inverse discrete Fourier transform, if direction is fftInverse.
// data is a complex array of length nn or, equivalently, a real array of length 2*len. len MUST
// be an integer power of 2 (this is not checked for!).
void fft1d(float* data, unsigned long len, fftDirection direction);
// Calculates the Fourier transform of a set of n real-valued data points. Replaces this data (which
// is stored in array data[1..n]) by the positive frequency half of its complex Fourier transform.
// The real-valued first and last components of the complex transform are returned as elements
// data[1] and data[2], respectively. n must be a power of 2. This routine also calculates the
// inverse transform of a complex data array if it is the transform of real data. (Result in this case
// must be multiplied by 2/n.)
void fft1dReal(float* data, unsigned long len, fftDirection direction);
// calculate spectral magnitude of complex data vector inplace
// magnitude vector will have len/2 length
void fft1dMagnitude(float* data, unsigned long len);

#ifdef __cplusplus
	}
#endif

#endif //_fft_h_
