#include "fft.h"
#include <math.h>

#ifdef fftDBL_PRECISION
#	define fftPrecise double
# define fftSin sin
#else
#	define fftPrecise float
#	define fftSin sinf
#endif

static const float c_pi = 3.141592653589793f;
static const float c_2pi = 6.283185307179586f;

// Replaces data[1..2*len] by its discrete Fourier transform, if direction is fftDirect; or replaces
// data[1..2*len] by nn times its inverse discrete Fourier transform, if direction is fftInverse.
// data is a complex array of length nn or, equivalently, a real array of length 2*len. len MUST
// be an integer power of 2 (this is not checked for!).
void fft1d(float* data, unsigned long len, fftDirection direction)
{
	unsigned long n = len << 1, mmax, m, j = 1, istep, i;
	fftPrecise wtemp, wr, wpr, wpi, wi, theta;
	float tempr, tempi;

	for(i = 1; i < n; i += 2) 
	{
		if(j > i) 
		{
			tempr = data[j]; 
			data[j] = data[i]; 
			data[i] = tempr;
			tempr = data[j+1]; 
			data[j+1] = data[i+1]; 
			data[i+1] = tempr;
		}

		m = len;
		while(m >= 2 && j > m) 
		{
			j -= m;
			m >>= 1;
		}

		j += m;
	}

	mmax = 2;
	while(n > mmax) 
	{
		istep = mmax << 1;
		theta = (fftPrecise)c_2pi/(fftPrecise)((int)direction*mmax);
		wtemp = fftSin((fftPrecise)0.5f*theta);
		wpr = ((fftPrecise)-2.0f)*wtemp*wtemp;
		wpi = fftSin(theta);
		wr = 1.0f;
		wi = 0.0f;

		for(m = 1; m < mmax; m += 2) 
		{
			for(i = m; i <= n; i += istep) 
			{
				j = i + mmax;
				tempr = (float)(wr*(fftPrecise)data[j] - wi*(fftPrecise)data[j+1]);
				tempi = (float)(wr*(fftPrecise)data[j+1] + wi*(fftPrecise)data[j]);
				data[j] = data[i] - tempr;
				data[j+1] = data[i+1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wtemp = wr;
			wr = wtemp*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}
}

//Calculates the Fourier transform of a set of n real-valued data points. Replaces this data (which
//is stored in array data[1..n]) by the positive frequency half of its complex Fourier transform.
//The real-valued ?rst and last components of the complex transform are returned as elements
//data[1] and data[2], respectively. n must be a power of 2. This routine also calculates the
//inverse transform of a complex data array if it is the transform of real data. (Result in this case
//must be multiplied by 2/n.)
void fft1dReal(float* data, unsigned long len, fftDirection direction)
{
	unsigned long i, i1, i2, i3, i4, np3;
	float c1=0.5f, c2, h1r, h1i, h2r, h2i;
	fftPrecise wr, wi, wpr, wpi, wtemp, theta = (fftPrecise)c_pi/(fftPrecise)(len >> 1);

	if(fftDirect == direction) 
	{
		c2 = -0.5f;
		fft1d(data, len >> 1, direction);
	} 
	else 
	{
		c2 = 0.5f;
		theta = -theta;
	}
	wtemp = fftSin((fftPrecise)0.5f*theta);
	wpr = (fftPrecise)-2.0f*wtemp*wtemp;
	wpi = fftSin(theta);
	wr = (fftPrecise)1.0f + wpr;
	wi = wpi;
	np3 = len+3;
	for(i=2; i <= (len >> 2); ++i)
	{
		i1 = i+i-1;
		i2 = 1+i1;
		i3 = np3-i2;
		i4 = 1+i3;
		h1r = c1*(data[i1]+data[i3]);
		h1i = c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i = c2*(data[i1]-data[i3]);
		data[i1] = h1r+wr*h2r-wi*h2i;
		data[i2] = h1i+wr*h2i+wi*h2r;
		data[i3] = h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wtemp = wr;
		wr = wtemp*wpr-wi*wpi+wr;
		wi = wi*wpr+wtemp*wpi+wi;
	}
	
	if(fftDirect == direction) 
	{
		h1r = data[1];
		data[1] = h1r+data[2];
		data[2] = h1r-data[2];
	} 
	else 
	{
		h1r=data[1];
		data[1]=c1*(h1r+data[2]);
		data[2]=c1*(h1r-data[2]);
		fft1d(data, len >> 1, direction); 
	}
}

// calculate spectral magnitude inplace
// magnitude vector will have len/2 length
void fft1dMagnitude(float* data, unsigned long len)
{
	float r, i, tmp;
	float *dest, *pos, *end;

	*data /= (float)len;			// calc the first magnitude member
	dest = data+1;
	tmp = (*dest)/(float)len; // save end real magnitude
	pos = data+2;
	end = data+len;
	while(pos < end)
	{
		r = *pos++;
		i = *pos++;
		*dest++ = sqrt(r*r + i*i)/(float)len;
	}
	// write ending magnitude value
	*dest = tmp;
}
