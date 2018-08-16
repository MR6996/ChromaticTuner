#pragma once
#pragma comment(lib, "portaudio/lib/portaudio_x86.lib")

#include <Windows.h>
#include "SoundAnalizer.h"
#include "portaudio/portaudio.h"
#include <vector>

typedef float SAMPLE;

/**
 *	
 *
 *	@author Mario Randazzo
 *
 * */
class Tuner {

public:
	
	/**
	 *
	 * */
	Tuner(int fs, int nChannels, int framePerBuffer);

	~Tuner();

	/**
	 *	
	 * */
	void StartTune();

	/**
	 *
	 * */
	void EndTune();

	/**
	 *
	 * */
	void ChangeInputDevice(PaDeviceIndex index);

	/**
	 *
	 * */
	PaDeviceIndex GetDeviceIndex();

	/**
	 *
	 * */
	double GetMaxFreq();

	/*
	 *
	 * */
	float GetSample(int i);

	/**
	 *
	 * */
	std::vector<const PaDeviceInfo*> EnumerateAudioInputDevice();

	/**
	 *
	 * */
	static double Freq2key(double freq, double freq_ref = 440.0);

	/**
	 *
	 * */
	static double Key2freq(int key, double freq_ref = 440.0);


private:

	//
	void Analize();

	//
	static int RecordCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData
	);


	//MEBER VARIABLE
	PaStreamParameters	mInputParameters;
	PaStream*			m_pStream;
	PaError				mErr;

	SoundAnalizer*		mAnalizer;

	double*				m_pAudioBuffer;
	int					mFs;
	int					mNChannels;
	int					mFramePerBuffer;
	double				mMaxFreq;

	HANDLE				mUpdateMutex;
};

