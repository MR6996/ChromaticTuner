#include "SoundAnalizer.h"

SoundAnalizer::SoundAnalizer(double* pBuffer, int wSize, int fs) :
	m_pBuffer(pBuffer),
	mWSize(wSize),
	mFs(fs) {

	//Allocate FFTW data
	mfft_len = 2 * mWSize;

	m_pfft_in = new std::complex<double>[mfft_len];
	m_pfft_out = new std::complex<double>[mfft_len];

	mfft_plan = fftw_plan_dft_1d(
		mfft_len, 
		reinterpret_cast<fftw_complex*>(m_pfft_in),
		reinterpret_cast<fftw_complex*>(m_pfft_out), 
		FFTW_FORWARD,
		FFTW_ESTIMATE
	);

	mifft_plan = fftw_plan_dft_1d(
		mfft_len,
		reinterpret_cast<fftw_complex*>(m_pfft_out),
		reinterpret_cast<fftw_complex*>(m_pfft_in),
		FFTW_BACKWARD,
		FFTW_ESTIMATE
	);

	//Allocate support memory
	
	m_pNSDF = new double[mWSize];
	m_pPow = new double[mWSize];
	m_pTmp = new double[mWSize];
}

SoundAnalizer::~SoundAnalizer() {
	//Release FFT data
	delete[] m_pfft_in;
	delete[] m_pfft_out;
	fftw_destroy_plan(mfft_plan);
	fftw_destroy_plan(mifft_plan);

	//Release support memory
	delete[] m_pNSDF;
	delete[] m_pPow;
	delete[] m_pTmp;
}


double SoundAnalizer::MPMethod() {
	CalculateNSDFunction();

	// Find the first 'major' peak.
	int i = 0, maxArg;
	while (i < mWSize && m_pNSDF[i] >= 0.0) i++;
	while (i < mWSize && m_pNSDF[i] <=  0.0) i++;

	maxArg = i;
	while (i < mWSize && m_pNSDF[i] >= 0.0) {
		if (m_pNSDF[maxArg] < m_pNSDF[i] ) 
			maxArg = i;
		i++;
	}

	while (i < mWSize) {
		if (m_pNSDF[maxArg] < m_pNSDF[i]-0.07)
			maxArg = i;
		i++;
	}

	// Find the period interpolating three point near local maximum.
	// For the frequancy divede fs by the period.
	return mFs /
		   FitStationaryPntPoly2(
			   maxArg-1, maxArg, maxArg+1, 
			   m_pNSDF[maxArg - 1], m_pNSDF[maxArg], m_pNSDF[maxArg + 1]
		   );
}

void SoundAnalizer::CalculateNSDFunction() {
	// Calculates the Normalized squared difference function defined in 
	// the paper, using the FFT for autocorrelation function
	// and a smart way for calculating the sum of squares.

	//Calculatic Autocorrelation function. 
	int i = 0;
	for (i = 0; i < mWSize; i++)
		m_pfft_in[i] = m_pBuffer[i];
	for (; i < mfft_len; i++)			//Pad the second half od the array with zero.
		m_pfft_in[i] = 0.0;

	fftw_execute(mfft_plan);
	for (i = 0; i < mfft_len; i++) 
		m_pfft_out[i] *= std::conj(m_pfft_out[i]);
	fftw_execute(mifft_plan);

	// Calculatic squared difference function. 
	m_pTmp[0] = 0;
	for (i = 0; i < mWSize; i++) {
		m_pPow[i] = m_pBuffer[i] * m_pBuffer[i];	//Pre-calculates the squares
		m_pTmp[0] += m_pPow[i] + m_pPow[i];			//Calculate the sum of squares at deley 0
	}

	int l = 0, r = mWSize - 1;
	for (i = 1; i < mWSize; i++) {
		// The successive sums of squares are obtained from the precedend value.
		m_pTmp[i] = m_pTmp[i - 1] - m_pPow[l] - m_pPow[r];	
		// Divide by mWSize because fftw don't use the normalization factor 1/N.
		m_pNSDF[i] = std::real(m_pfft_in[i]) / mWSize / m_pTmp[i];
		l++; r--;
	}

	m_pNSDF[0] = std::real(m_pfft_in[0]) / mWSize / m_pTmp[0];
}

double SoundAnalizer::FitStationaryPntPoly2(int x_0, int x_1, int x_2, double y_0, double y_1, double y_2) {
	// Use the Newton interpolation polynomial for calculating 
	// the stationary point(the derivative null) of the polinomyal 
	// that interpolate the points passed by parameter. 

	//Calculate the divided differences.
	double f_01 = (y_1 - y_0) / (x_1 - x_0),
		   f_12 = (y_2 - y_1) / (x_2 - x_1),
		   f_02 = (f_12 - f_01) / (x_2 - x_0);

	//Calculate the stationary point.
	return ((x_0 + x_1) - f_01 / f_02) / 2;
}
