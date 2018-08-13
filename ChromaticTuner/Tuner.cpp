#include "Tuner.h"
#include <cmath>

Tuner::Tuner(int fs, int nChannels, int framePerBuffer) :
	mFs(fs), 
	mNChannels(nChannels),
	mFramePerBuffer(framePerBuffer),
	mMaxFreq(440.f) {

	//Initialize PortAudio data.
	mErr = paNoError;
	mErr = Pa_Initialize();
	/**
					Error handling
	*/

	mInputParameters.device = Pa_GetDefaultInputDevice();
	/**
					Error handling
	*/

	mInputParameters.channelCount = mNChannels;
	mInputParameters.sampleFormat = paFloat32;
	mInputParameters.suggestedLatency = Pa_GetDeviceInfo(mInputParameters.device)->defaultLowInputLatency;
	mInputParameters.hostApiSpecificStreamInfo = NULL;

	mErr = Pa_OpenStream(
		&m_pStream,
		&mInputParameters,
		NULL,
		mFs,
		mFramePerBuffer,
		paClipOff,
		RecordCallback,
		this
	);
	/**
					Error handling
	*/


	// Allocate audio buffer and sound analizer.
	m_pAudioBuffer = new double[mFramePerBuffer];
	mAnalizer = new SoundAnalizer(m_pAudioBuffer, mFramePerBuffer, fs);

	// Initialize mutex for update.
	mUpdateMutex = CreateMutex(NULL, FALSE, NULL);
}

Tuner::~Tuner() {
	//Release PortAudio data
	Pa_Terminate();
	delete[] m_pAudioBuffer;
	
	//Release sound analizer.
	delete mAnalizer;

	//Release mutex for update
	CloseHandle(mUpdateMutex);
}


void Tuner::StartTune() {
	mErr = Pa_StartStream(m_pStream);
	/**
				Error handling
	*/
}

void Tuner::EndTune() {
	mErr = Pa_StopStream(m_pStream);
	/**
	Error handling
	*/
}

void Tuner::ChangeInputDevice(PaDeviceIndex index) {
	
	mErr = Pa_StopStream(m_pStream);
	/**
	Error handling
	*/

	mInputParameters.device = index;
	/**
	Error handling
	*/

	mInputParameters.channelCount = mNChannels;
	mInputParameters.suggestedLatency = Pa_GetDeviceInfo(mInputParameters.device)->defaultLowInputLatency;

	mErr = Pa_OpenStream(
		&m_pStream,
		&mInputParameters,
		NULL,
		mFs,
		mFramePerBuffer,
		paClipOff,
		RecordCallback,
		this
	);

	mErr = Pa_StartStream(m_pStream);
	/**
	Error handling
	*/
}

PaDeviceIndex Tuner::GetDeviceIndex() {
	return mInputParameters.device;
}

double Tuner::GetMaxFreq() {
	double freqValue;

	WaitForSingleObject(mUpdateMutex, INFINITE);
	freqValue = mMaxFreq;
	ReleaseMutex(mUpdateMutex);

	return freqValue;
}

float Tuner::GetSample(int i) {
	if (i > mFramePerBuffer) return 0.f;
	float sampleValue;

	WaitForSingleObject(mUpdateMutex, INFINITE);
	sampleValue = (float)abs(m_pAudioBuffer[i]);
	ReleaseMutex(mUpdateMutex);

	return sampleValue;
}

std::vector<const PaDeviceInfo*> Tuner::EnumerateAudioInputDevice() {
	std::vector<const PaDeviceInfo*> devices;
	int n = Pa_GetDeviceCount();
		
	if ( n > 0)
		for (int i = 0; i < n; i++) {
			const PaDeviceInfo* device = Pa_GetDeviceInfo(i);
			//if (i == Pa_GetHostApiInfo(device->hostApi)->defaultInputDevice)
				devices.push_back(device);
		}
	return devices;
}


double Tuner::Freq2key(double freq, double freq_ref) {
	if (freq > 26.30)
		return 12.0 * log2(freq / freq_ref) + 49.0;
	
	return 0.0;
}

double Tuner::Key2freq(int key, double freq_ref) {
	if (key < 0) return 0.0;
	return freq_ref * pow(2, ((double)key - 49) / 12.0);
}




void Tuner::Analize() {
	double freq = mAnalizer->MPMethod();

	WaitForSingleObject(mUpdateMutex, INFINITE);
	mMaxFreq = freq;
	ReleaseMutex(mUpdateMutex);
}

int Tuner::RecordCallback(const void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo * timeInfo, PaStreamCallbackFlags statusFlags, void * userData) {
	Tuner* tuner = reinterpret_cast<Tuner*>(userData);
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;

	if (inputBuffer == NULL) {
		for (long i = 0; i < tuner->mFramePerBuffer; i++)
			tuner->m_pAudioBuffer[i] = 0.0;
	}
    else {
		if (tuner->mNChannels == 2) {
			for (long i = 0; i < tuner->mFramePerBuffer; i++) {
				tuner->m_pAudioBuffer[i] = ((double)(*rptr++) + (double)(*rptr++))/2.0;
			}
		}
		else {
			for (long i = 0; i < tuner->mFramePerBuffer; i++)
				tuner->m_pAudioBuffer[i] = (double)rptr[i];
		}
	}

	tuner->Analize();

	return paContinue;
}
