#pragma once
#pragma comment(linker,"/manifestdependency:\"" \
    "type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' "  \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "Dwrite.lib")

#include "Tuner.h"
#include "Constants.h"
#include "resource.h"
#include <math.h>
#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>


/**
*	Function to release safetly an interface
*
*	@param ppInterfaceToRelease Interface to release
*
* */
template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease) {
	if (*ppInterfaceToRelease != NULL) {
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}



/**
 *
 * */
class TunerUI {

public:

	/**
	 *	Build and initialize the application handle.
	 * */
	TunerUI();

	/**
	 *	Release safetly the resources.
	 * */
	~TunerUI();

	/**
	*	Register the window class and call methods for
	*	instantiating drawing resources.
	*
	*	@return HRESULT value that indicate if the op. was successful.
	*
	* */
	HRESULT Initialize();

	/**
	*	Process and dispatch messages
	*
	* */
	void RunMessageLoop();

private:

	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceResources();

	// Release device-dependent resource.
	void DiscardDeviceResources();

	// Draw the textual contets
	void DrawTextContents(D2D1_SIZE_F rtSize);

	// Draw the graphical tuner
	void DrawMeter(D2D1_SIZE_F rtSize);

	// Draw the graph of the last recorded audio
	void DrawGraph(D2D1_SIZE_F rtSize);

	// Draw the bar of volume
	void DrawBar(D2D1_SIZE_F rtSize);

	// Draw contents.
	HRESULT OnRender();

	// Resize the render target.
	void OnResize(
		UINT width,
		UINT height
	);

	//	
	void UpdateValues();
	
	//
	static void CloseDialogBox(HWND hDlg, int type);

	// The windows procedure.
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	// The Dialog procedure for About window.
	static INT_PTR  CALLBACK AboutDlgProc(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	// The Dialog procedure for preferences window.
	static INT_PTR  CALLBACK PreferencesDlgProc(
		HWND hDlg,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	// MEMBER VARIABLE
	Tuner					mTuner;
	int						mA4Freq;
	float					mGraphBuffer[GRAPH_BUFF_SIZE + 1];
	float					mPointerValue;
	float					mLastPointerValue;
	int						mNote;
	int						mBaseGraph;
	int						mLastSample;
	WCHAR					mFreqText[45];

	HWND					mHwnd;
	HWND					mHDlgAbout;
	HWND					mHDlgPreferences;
	
	ID2D1Factory*			m_pDirect2dFactory;
	ID2D1HwndRenderTarget*	m_pRenderTarget;

	ID2D1SolidColorBrush*	m_pDarkGrayBrush;
	ID2D1SolidColorBrush*	m_pLightGrayBrush;
	ID2D1SolidColorBrush*	m_pGrayWhiteBrush;
	ID2D1SolidColorBrush*	m_pRedBrush;
	ID2D1SolidColorBrush*	m_pGreenBrush;
	ID2D1SolidColorBrush*	m_pSteelBlueBrush;
	ID2D1SolidColorBrush*	m_pBarBrushs[BAR_STEPS];

	IDWriteFactory*			m_pWriteFactory;
	IDWriteTextFormat*		m_pFreqTextFormat;
	IDWriteTextFormat*		m_pNoteTextFormat;
	IDWriteTextFormat*		m_pBoundNoteTextFormat;
};

