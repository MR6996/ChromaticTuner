#pragma once
#pragma comment(lib, "portaudio/lib/portaudio_x86.lib")

#include <Windows.h>
#include "SoundAnalizer.h"
#include "portaudio/portaudio.h"
#include <vector>

typedef float SAMPLE;

/**
 *	The class Tuner rappresents a tuner object that use 
 *	a SoundAnlizer for realtime pitch detection of microphone
 *	input. 
 *	The procedure run in realtime recording audio signal for a 
 *	small delta of time and applaying the algorithm. The delta 
 *  of time is specified by the number of the frame (a frame is 
 *	a collection of samples, one for each channel) and depends 
 *	by the sample rate.
 *
 *	@author Mario Randazzo
 *
 * */
class Tuner {

public:
	
	/**
	 *	Build the tuner, specifying the sample rate, and the number of 
	 *	frames. 
	 *
	 *	@param fs - sample rate
	 *	@param framePerBuffer - the number of frames recorded. 
	 *
	 * */
	Tuner(int fs, int nChannels, int framePerBuffer);

	~Tuner();

	/**
	 *	Starts a non-blocking procedure for recording and analyzing of audio. 
	 *
	 * */
	void StartTune();

	/**
	 *	Stops the procedure of recording and analyzing.
	 *
	 * */
	void EndTune();

	/**
	 *	Change the audio input device, and reboots the procedure of recording and analyzing. 	
	 *
	 *	@param index the index of audio input device.
	 *
	 * */
	void ChangeInputDevice(PaDeviceIndex index);

	/**
	 *	Return the index of the current audio input device.
	 *
	 *	@reurn the index of the current audio input device.
	 *	
	 * */
	PaDeviceIndex GetDeviceIndex();

	/**
	 *	Return the frequency detected on last samples recorded.
	 *
	 *	@return the frequency recorded
	 *
	 * */
	double GetMaxFreq();

	/*
	 *	Return the i-th sample recorded on last samples recorded.
	 *
	 *	@param i - the index of the sample, mast be between 0 and  mFramePerBuffer -1
	 *	@return the sample i-th recorded.
	 *
	 * */
	float GetSample(int i);

	/**
	 *	Return a list of the audio device available.
	 *
	 *	@reutnr a list of the audio device available.
	 *
	 * */
	std::vector<const PaDeviceInfo*> EnumerateAudioInputDevice();

	/**
	 *	Map the frequency specified in a 88-keys standard piano.
	 *
	 *	@param freq - the frequency in Hz
	 *	@param freq_ref - the frequency of a4
	 *	@return the key correspond to frequency specified.
	 *
	 * */
	static double Freq2key(double freq, double freq_ref = 440.0);

	/**
	 *	Map the 88-keys standard piano in a frequencys.
	 *
	 *	@param key - the number of a key .
	 *	@param freq_ref - the frequency of a4
	 *	@return the frequency of the key specified.
	 *
	 * */
	static double Key2freq(int key, double freq_ref = 440.0);


private:

	// Applay the algorithm and update the frequency detected.
	void Analize();

	//	Callback function, is called when the buffer is full.
	//	outbuffer will contains the last framePerBuffer frame recorded.
	//  The data in outbuffer is copied in m_pAudioBuffer and 
	//	Analize function is called.
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

	SoundAnalizer*		mAnalizer;

	double*				m_pAudioBuffer;
	int					mFs;
	int					mNChannels;
	int					mFramePerBuffer;
	double				mMaxFreq;

	HANDLE				mUpdateMutex;
};


/**
 *
 * */
class pa_error : std::exception {

public:
	pa_error (const char *m) {}

	const char* what() {
		return "asd";
	}
};