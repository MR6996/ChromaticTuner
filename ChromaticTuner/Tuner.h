#pragma once
#pragma comment(lib, "portaudio/lib/portaudio_x86.lib")

#include <Windows.h>
#include "SoundAnalizer.h"
#include "portaudio/portaudio.h"

typedef float SAMPLE;

/**
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
	double GetMaxFreq();

	/*
	 *
	 * */
	float GetSample(int i);

	/**
	 *
	 * */
	static double freq2key(double freq, double freq_ref = 440.0);

	/**
	 *
	 * */
	static double key2freq(int key, double freq_ref = 440.0);

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

