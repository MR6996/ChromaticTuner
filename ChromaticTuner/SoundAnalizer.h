#pragma once
#pragma comment(lib, "fftw/fftw3.lib")

#include "fftw/fftw3.h"
#include <complex>

/**
 *	Provides an implementation of McLeod Pitch Method,
 *	for detect the continuous pitch in monophonic musical sounds.
 *	This implementation act on a buffer, specified in the 
 *	constructor.
 *
 *	@author Mario Randazzo
 *
 * */
class SoundAnalizer {

public:

	/**
	 *	@param pBuffer - a pointer to the audio buffer
	 *	@param wSize   - the size of the audio buffer
	 *	@param fs	   - sample frequency
	 *
	 * */
	SoundAnalizer(double* pBuffer, int wSize, int fs);

	~SoundAnalizer();

	/**
	 *	Detect the frequency of the monophonic musical sound.
	 *
	 *	@return the frequancy detected by MPM.
	 *
	 * */
	double MPMethod();


private:

	// Calcualte the Normalized Squared Difference Function defined by 
	// the McLeod pitch method. 
	void CalculateNSDFunction();

	// Calculate the stationary point of the polinomyal that 
	// interpolate the point (x_0,y_0), (x_1,y_1), (x_2,y_2).
	double FitStationaryPntPoly2(int x_0, int x_1, int x_2,
								 double y_0, double y_1, double y_2);

	//	MEBER VARIABLE
	int						mFs;
	int						mWSize;
	double*					m_pBuffer;
	double*					m_pNSDF;
	double*					m_pPow;
	double*					m_pTmp;

	std::complex<double>*	m_pfft_in;
	std::complex<double>*	m_pfft_out;
	fftw_plan				mfft_plan;
	fftw_plan				mifft_plan;
	int						mfft_len;
};

