#pragma once

/**
 *	GENERAL
 * */
#define M_PI				(3.14159265358979323846f)


/**
 *	UI
 * */
#define REFRESH_TIMER_ID	(5)
#define REFRESH_PERIOD		(30)

#define N_SEC_GRAPH			(5)
#define MAX_AMPLITUDE		(1.f)
#define SAMPLE_PER_REFRESH	(20)
#define GRAPH_BUFF_SIZE		(N_SEC_GRAPH * 1000 / REFRESH_PERIOD * SAMPLE_PER_REFRESH)

#define BAR_STEPS			(30)

#define MIC_THRESHOLD		(BAR_STEPS*0.05f)
#define SMOOTHING_FACTOR	(0.125f)

 /**
  *	 TUNER
  * */
#define FS					(44100) 
#define DELTA_MILLS         (30)
#define N_SAMPLES			(FS*DELTA_MILLS/1000*N_CHANNELS)
#define N_FRAMES			(FS*DELTA_MILLS/1000)
#define FRAMES_PER_BUFFER	(N_FRAMES)
	
const char UI_CLASS_NAME[] = "tuner_app_wc";
const WCHAR NOTE_TABLE[24][3] = { L"C", L"C#", L"D", L"D#", L"E", L"F", L"F#", L"G", L"G#", L"A", L"A#", L"B",
								  L"C", L"D♭", L"D", L"E♭", L"E", L"F", L"G♭", L"G", L"A♭", L"A", L"B♭", L"B" };
