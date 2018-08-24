#include "TunerUI.h"
#include <Commctrl.h>
#include <vector>



TunerUI::TunerUI() :
	mTuner(FS, FRAMES_PER_BUFFER),
	mA4Freq(440),
	mPointerValue(50.0),
	mLastPointerValue(50.0),
	mThreshold(0.0),
	mNote((int)Tuner::Freq2key(mA4Freq) - 40),
	mOctave(4),
	mBaseGraph(0),
	mLastSample(GRAPH_BUFF_SIZE + 1),
	mTableOffset(0),
	mHwnd(NULL), 
	mHDlgAbout(NULL),
	mHDlgPreferences(NULL), 
	m_pDirect2dFactory(NULL),
	m_pWriteFactory(NULL),
	m_pRenderTarget(NULL) {

	swprintf_s(mFreqText, L"F = %.2f Hz", (double)mA4Freq);
	swprintf_s(mOctaveText, L"Octave = %d", mOctave);
	ZeroMemory(mGraphBuffer, GRAPH_BUFF_SIZE);
}

TunerUI::~TunerUI() {
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);

	SafeRelease(&m_pDarkGrayBrush);
	SafeRelease(&m_pGrayWhiteBrush);
	SafeRelease(&m_pLightGrayBrush);
	SafeRelease(&m_pRedBrush);
	SafeRelease(&m_pGreenBrush);
	SafeRelease(&m_pSteelBlueBrush);

	for (int i = 0; i < BAR_STEPS; i++)
		SafeRelease(&m_pBarBrushs[i]);

	SafeRelease(&m_pWriteFactory);
	SafeRelease(&m_pNormalTextFormat);
	SafeRelease(&m_pNoteTextFormat);
	SafeRelease(&m_pBoundNoteTextFormat);
}



HRESULT TunerUI::Initialize() {
	HRESULT hResult = S_OK;

	hResult = CreateDeviceIndependentResources();

	if (SUCCEEDED(hResult)) {
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = TunerUI::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
		wcex.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = UI_CLASS_NAME;

		if (!RegisterClassEx(&wcex))
			MessageBox(NULL, "Can't Register the windows!", "Error", MB_ICONHAND);


		mHwnd = CreateWindowEx(
			0,
			UI_CLASS_NAME,
			"Chromatic Tuner v.01",
			WS_TILEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			600,
			420,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			this);

		hResult = mHwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hResult)) {
			ShowWindow(mHwnd, SW_SHOWNORMAL);
			UpdateWindow(mHwnd);
		}
	}
	else 
		MessageBox(mHwnd, "Can't initialize device resources!", "Directx2D Error", MB_OK | MB_ICONEXCLAMATION);

	return hResult;
}

void TunerUI::RunMessageLoop() {
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (mHDlgAbout == NULL || !IsDialogMessage(mHDlgAbout, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}



HRESULT TunerUI::CreateDeviceIndependentResources() {
	HRESULT hResult = S_OK;

	hResult = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&m_pDirect2dFactory
	);

	if (SUCCEEDED(hResult))
		hResult = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pWriteFactory),
			(IUnknown**)&m_pWriteFactory
		);

	return hResult;
}

HRESULT TunerUI::CreateDeviceResources() {
	HRESULT hResult = S_OK;

	if (!m_pRenderTarget) {
		RECT rc;
		GetClientRect(mHwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		hResult = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(mHwnd, size),
			&m_pRenderTarget);


		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(25, 25, 25)),
				&m_pDarkGrayBrush);

		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(230, 230, 230)),
				&m_pGrayWhiteBrush);

		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(144, 144, 144)),
				&m_pLightGrayBrush);


		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(0, 0, 225)),
				&m_pRedBrush);

		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(0, 210, 0)),
				&m_pGreenBrush);

		if (SUCCEEDED(hResult))
			hResult = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(RGB(180, 130, 70)),
				&m_pSteelBlueBrush);

		if (SUCCEEDED(hResult)) {
			int step = 510 / BAR_STEPS;
			for (int i = 0; i < BAR_STEPS && SUCCEEDED(hResult); i++) {
				hResult = m_pRenderTarget->CreateSolidColorBrush(
					D2D1::ColorF(RGB(0, min(255, 510 - (i*step)), min(255, (i*step)))),
					&m_pBarBrushs[i]);
			}

		}

		if (SUCCEEDED(hResult))
			hResult = m_pWriteFactory->CreateTextFormat
			(L"Times",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				18,
				L"", //locale
				&m_pNormalTextFormat);


		if (SUCCEEDED(hResult))
			hResult = m_pWriteFactory->CreateTextFormat
			(L"Times",
				NULL,
				DWRITE_FONT_WEIGHT_BOLD,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				50,
				L"", //locale
				&m_pNoteTextFormat);

		if (SUCCEEDED(hResult))
			m_pNoteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);


		if (SUCCEEDED(hResult))
			hResult = m_pWriteFactory->CreateTextFormat
			(L"Times",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				35,
				L"", //locale
				&m_pBoundNoteTextFormat);

		if (SUCCEEDED(hResult))
			m_pBoundNoteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	return hResult;
}

void TunerUI::DiscardDeviceResources() {
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pDarkGrayBrush);
	SafeRelease(&m_pGrayWhiteBrush);
	SafeRelease(&m_pLightGrayBrush);
	SafeRelease(&m_pRedBrush);
	SafeRelease(&m_pGreenBrush);
	SafeRelease(&m_pSteelBlueBrush);

	for (int i = 0; i < BAR_STEPS; i++)
		SafeRelease(&m_pBarBrushs[i]);

	SafeRelease(&m_pNormalTextFormat);
	SafeRelease(&m_pNoteTextFormat);
	SafeRelease(&m_pBoundNoteTextFormat);
}

void TunerUI::DrawTextContents(D2D1_SIZE_F rtSize) {

	const D2D1_RECT_F freqRect = D2D1::RectF(18.f, 18.f, 248.f, 30.f);

	const D2D1_RECT_F octaveRect = D2D1::RectF(
		rtSize.width - 18.f, 
		18.f, 
		rtSize.width - 108.f, 
		30.f);

	const D2D1_RECT_F noteRect = D2D1::RectF(
		rtSize.width / 2.f - 50,
		rtSize.height * 0.4f - 22.f - 25.f,
		rtSize.width / 2.f + 50,
		rtSize.height * 0.4f - 22.f + 25.f);

	const D2D1_RECT_F lb_noteRect = D2D1::RectF(
		32.f,
		rtSize.height * 0.4f - 22.f - 12.2f,
		132.f,
		rtSize.height * 0.4f - 22.f + 12.2f);

	const D2D1_RECT_F rb_noteRect = D2D1::RectF(
		rtSize.width - 32.f,
		rtSize.height * 0.4f - 22.f - 12.2f,
		rtSize.width - 132.f,
		rtSize.height * 0.4f - 22.f + 12.2f);

	m_pRenderTarget->DrawText(
		mFreqText,
		ARRAYSIZE(mFreqText) - 1,
		m_pNormalTextFormat,
		freqRect,
		m_pGrayWhiteBrush);

	m_pRenderTarget->DrawText(
		mOctaveText,
		ARRAYSIZE(mOctaveText) - 1,
		m_pNormalTextFormat,
		octaveRect,
		m_pGrayWhiteBrush);

	m_pRenderTarget->DrawText(
		NOTE_TABLE[mNote + mTableOffset],
		ARRAYSIZE(NOTE_TABLE[mNote + mTableOffset]) - 1,
		m_pNoteTextFormat,
		noteRect,
		m_pGrayWhiteBrush);

	m_pRenderTarget->DrawText(
		NOTE_TABLE[(mNote + 11) % 12 + mTableOffset],
		ARRAYSIZE(NOTE_TABLE[(mNote + 11) % 12 + mTableOffset]) - 1,
		m_pBoundNoteTextFormat,
		lb_noteRect,
		m_pLightGrayBrush);

	m_pRenderTarget->DrawText(
		NOTE_TABLE[(mNote + 1) % 12 + mTableOffset],
		ARRAYSIZE(NOTE_TABLE[(mNote + 1) % 12 + mTableOffset]) - 1,
		m_pBoundNoteTextFormat,
		rb_noteRect,
		m_pLightGrayBrush);
}

void TunerUI::DrawMeter(D2D1_SIZE_F rtSize) {
	D2D1_ELLIPSE fixed_circle = D2D1::Ellipse(
		D2D1::Point2F(rtSize.width / 2.f, rtSize.height * 0.6f - 22.f),
		12.f, 12.f
	);

	float radius = rtSize.height * 0.5f - 30.f;

	for (int i = 0; i < 101; i++)
		if (i % 10 == 0)
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(
					rtSize.width / 2.f + (radius - 15.f) * (float)cos(i / 100.0 * M_PI),
					rtSize.height * 0.6f - 22.f - (radius - 15.f) * (float)sin(i / 100.0 * M_PI)),
				D2D1::Point2F(
					rtSize.width / 2.f + (radius + 5.f) * (float)cos(i / 100.0 * M_PI),
					rtSize.height * 0.6f - 22.f - (radius + 5.f) * (float)sin(i / 100.0 * M_PI)
				),
				m_pSteelBlueBrush, 2.3f
			);
		else
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(
					rtSize.width / 2.f + (radius - 10.f) * (float)cos(i / 100.0 * M_PI),
					rtSize.height * 0.6f - 22.f - (radius - 10.f) * (float)sin(i / 100.0 * M_PI)),
				D2D1::Point2F(
					rtSize.width / 2.f + radius * (float)cos(i / 100.0 * M_PI),
					rtSize.height * 0.6f - 22.f - radius * (float)sin(i / 100.0 * M_PI)
				),
				m_pLightGrayBrush
			);

	m_pRenderTarget->DrawLine(
		D2D1::Point2F(rtSize.width / 2.f, rtSize.height * 0.6f - 22.f),
		D2D1::Point2F(
			rtSize.width / 2.f + (radius - 15.f) * -(float)cos(mPointerValue / 100 * M_PI),
			rtSize.height * 0.6f - 22.f - (radius - 15.f) * (float)sin(mPointerValue / 100 * M_PI)
		),
		m_pRedBrush,
		2.5f
	);

	m_pRenderTarget->FillEllipse(fixed_circle, m_pSteelBlueBrush);
}

void TunerUI::DrawGraph(D2D1_SIZE_F rtSize) {
	float startRtGraphW = rtSize.width * 0.2f + 6.f;
	float rtGraphW = rtSize.width - 12.f;
	float startRtGraphH = rtSize.height * 0.6f + 6.f;
	float rtGraphH = rtSize.height - 12.f;

	float y_0 = (rtGraphH - startRtGraphH) / 2 + startRtGraphH,
		delta_y = rtGraphH - y_0 - 8.f,
		dx = (rtGraphW - startRtGraphW) / GRAPH_BUFF_SIZE,
		x = startRtGraphW, y,
		y_scale = delta_y / MAX_AMPLITUDE;

	//Draw the graph of wave recorded
	for (int i = mBaseGraph; i < mBaseGraph + GRAPH_BUFF_SIZE + 1; i++) {
		x += dx;
		y = (mGraphBuffer[i % (GRAPH_BUFF_SIZE + 1)] + 0.001f)* y_scale;
		m_pRenderTarget->DrawLine(
			D2D1::Point2F(x, y_0 + y),
			D2D1::Point2F(x, y_0 - y),
			m_pGrayWhiteBrush,
			1.0f
		);
	}
}

void TunerUI::DrawBar(D2D1_SIZE_F rtSize) {
	float bar_step_lenght = (rtSize.height - 17.f - rtSize.height * 0.6f - 11.f) / (float)BAR_STEPS;

	//Draw the bar
	for (int i = 1; i <= BAR_STEPS; i++) {
		if (i <= mThreshold) {
			m_pRenderTarget->FillRectangle(D2D1::RectF(
				rtSize.width * 0.2f / 2.f - 23.f, rtSize.height - 17.f - (bar_step_lenght)*(i - 1) - 1.5f,
				rtSize.width * 0.2f / 2.f + 1.f, rtSize.height - 17.f - (bar_step_lenght)*(i)
			), m_pBarBrushs[i - 1]);
			m_pRenderTarget->FillRectangle(D2D1::RectF(
				rtSize.width * 0.2f / 2.f + 5.f, rtSize.height - 17.f - (bar_step_lenght)*(i - 1) - 1.5f,
				rtSize.width * 0.2f / 2.f + 29.f, rtSize.height - 17.f - (bar_step_lenght)*(i)
			), m_pBarBrushs[i - 1]);
		}
		else {
			m_pRenderTarget->FillRectangle(D2D1::RectF(
				rtSize.width * 0.2f / 2.f - 23.f, rtSize.height - 17.f - (bar_step_lenght)*(i - 1) - 1.5f,
				rtSize.width * 0.2f / 2.f + 1.f, rtSize.height - 17.f - (bar_step_lenght)*(i)
			), m_pDarkGrayBrush);
			m_pRenderTarget->FillRectangle(D2D1::RectF(
				rtSize.width * 0.2f / 2.f + 5.f, rtSize.height - 17.f - (bar_step_lenght)*(i - 1) - 1.5f,
				rtSize.width * 0.2f / 2.f + 29.f, rtSize.height - 17.f - (bar_step_lenght)*(i)
			), m_pDarkGrayBrush);
		}
	}
}

HRESULT TunerUI::OnRender() {
	HRESULT hResult = S_OK;

	hResult = CreateDeviceResources();

	if (SUCCEEDED(hResult)) {
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(RGB(64, 64, 64)));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
		float startRtGraphW = rtSize.width * 0.2f + 6.f;
		float rtGraphW = rtSize.width - 12.f;
		float startRtGraphH = rtSize.height * 0.6f + 6.f;
		float rtGraphH = rtSize.height - 12.f;

		D2D1_ROUNDED_RECT graphRect = D2D1::RoundedRect(
			D2D1::RectF(
				startRtGraphW, startRtGraphH,
				rtGraphW, rtGraphH
			),
			6.f, 6.f
		);

		D2D1_ROUNDED_RECT barRect = D2D1::RoundedRect(
			D2D1::RectF(
				12.f, rtSize.height - 12.f,
				rtSize.width * 0.2f - 6.f, rtSize.height * 0.6f + 6.f
			),
			6.f, 6.f
		);

		D2D1_ROUNDED_RECT tuneRect = D2D1::RoundedRect(
			D2D1::RectF(
				12.f, 12.f,
				rtSize.width - 12.f, rtSize.height * 0.6f - 6.f
			),
			6.f, 6.f
		);

		// Draw a filled rectangle for graph.
		m_pRenderTarget->FillRoundedRectangle(&graphRect, m_pDarkGrayBrush);

		// Draw a filled rectangle for tube ui.
		m_pRenderTarget->FillRoundedRectangle(&tuneRect, m_pDarkGrayBrush);

		// Draw a filled rectangle for tube ui.
		m_pRenderTarget->DrawRoundedRectangle(&barRect, m_pDarkGrayBrush, 1.5f);

		// Draw textual contents, such as note and freq
		DrawTextContents(rtSize);

		// Draw the meter ui
		DrawMeter(rtSize);

		// Draw the graph of the wave recorded
		DrawGraph(rtSize);

		// Draw the bar of volume
		DrawBar(rtSize);

		hResult = m_pRenderTarget->EndDraw();
	}

	if (hResult == D2DERR_RECREATE_TARGET) {
		hResult = S_OK;
		DiscardDeviceResources();
	}


	return hResult;
}

void TunerUI::OnResize(UINT width, UINT height) {
	if (m_pRenderTarget)
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
}

void TunerUI::UpdateValues() {
	mBaseGraph = (mBaseGraph + SAMPLE_PER_REFRESH) % (2 * GRAPH_BUFF_SIZE + 1);
	for (int i = 0, j = 0; i < SAMPLE_PER_REFRESH; i++, j += N_FRAMES / SAMPLE_PER_REFRESH)
		mGraphBuffer[(mLastSample + i) % (GRAPH_BUFF_SIZE + 1)] = mTuner.GetSample(j);
	mLastSample = (mLastSample + SAMPLE_PER_REFRESH) % (2 * GRAPH_BUFF_SIZE + 1);
	
	float tmpThreshold;
	mThreshold = 0.f;
	for (int i = 0; i < SAMPLE_PER_REFRESH; i++) {
		tmpThreshold = mGraphBuffer[(mLastSample + GRAPH_BUFF_SIZE - i) % (GRAPH_BUFF_SIZE + 1)] / MAX_AMPLITUDE * BAR_STEPS;
		if (tmpThreshold > mThreshold) mThreshold = tmpThreshold;
	}

	if (mThreshold > MIC_THRESHOLD) {
		double f = mTuner.GetMaxFreq(),
			   key = Tuner::Freq2key(f, mA4Freq);
		int key_int = (int)rint(key);
		
		mOctave = (key_int + 8) / 12;
		mNote = key_int + 8 - mOctave * 12;
		mPointerValue = (float)(100.0 * (key - key_int + 0.5));
		mPointerValue = mLastPointerValue + (mPointerValue - mLastPointerValue)*SMOOTHING_FACTOR;
		mLastPointerValue = mPointerValue;
	
		swprintf_s(mFreqText, L"F = %.2f Hz", f);
		swprintf_s(mOctaveText, L"Octave = %d", mOctave);
	}
}

LRESULT TunerUI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	if (message == WM_NCCREATE) {

		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		TunerUI *pTunerUI = (TunerUI*)pcs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToLong(pTunerUI));

		try {
			pTunerUI->mTuner.StartTune();
		}
		catch (pa_error ex) { 
			MessageBox(hWnd, ex.getErrMessage(), "Audio input device error", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		SetWindowText(hWnd, "Chromatic Tuner v.01");

		if (!SetTimer(hWnd, REFRESH_TIMER_ID, REFRESH_PERIOD, NULL)) {
			MessageBox(hWnd, "Could not set timer!", "Error", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		
		return TRUE;
	}
	else {
		TunerUI *pTunerUI = reinterpret_cast<TunerUI*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));

		if (pTunerUI) {
			switch (message) {
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pTunerUI->OnResize(width, height);
			}
			return TRUE;

			case WM_DISPLAYCHANGE:
				InvalidateRect(hWnd, NULL, FALSE);
				return TRUE;

			case WM_TIMER:
				pTunerUI->UpdateValues();
				PostMessage(hWnd, WM_PAINT, 0, 0);
				return TRUE;

			case WM_PAINT:
				pTunerUI->OnRender();
				ValidateRect(hWnd, NULL);
				return TRUE;

			case WM_COMMAND:
				switch (LOWORD(wParam)) {
				case ID_FILE_EXIT:
					PostMessage(hWnd, WM_DESTROY, 0, 0);
					break;
				case ID__ABOUT:
					pTunerUI->mHDlgAbout = CreateDialog(
						GetModuleHandle(NULL),
						MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
						hWnd,
						TunerUI::AboutDlgProc
					);
					break;
				case ID_FILE_PREFERENCES:
					pTunerUI->mHDlgPreferences = CreateDialogParam(
						GetModuleHandle(NULL),
						MAKEINTRESOURCE(IDD_PREF_DIALOG),
						hWnd,
						TunerUI::PreferencesDlgProc,
						(LPARAM)pTunerUI->mA4Freq
					);
					break;
				}
				return TRUE;

			case WM_DESTROY:
				PostQuitMessage(0);
				KillTimer(hWnd, REFRESH_TIMER_ID);
				pTunerUI->mTuner.EndTune();
				return TRUE;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	}

	return TRUE;
}

INT_PTR TunerUI::AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ABOUT_OK:
			TunerUI * pTunerUI = reinterpret_cast<TunerUI*>(
				static_cast<LONG_PTR>(GetWindowLongPtrW(GetParent(hDlg), GWLP_USERDATA))
				);

			DestroyWindow(hDlg);
			pTunerUI->mHDlgPreferences = NULL;
			return TRUE;
		}
	
	}

	return FALSE;
}

INT_PTR TunerUI::PreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	TunerUI * pTunerUI = reinterpret_cast<TunerUI*>(
		static_cast<LONG_PTR>(GetWindowLongPtrW(GetParent(hDlg), GWLP_USERDATA))
		);

	switch (message) {
	case WM_INITDIALOG: 
	{
		int a4freq = (int)lParam;
		HWND freqSlider = GetDlgItem(hDlg, IDC_FREQ_SLIDER);
		SendMessage(freqSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(415,445));
		SendMessage(freqSlider, TBM_SETPOS, (WPARAM)TRUE, a4freq);
		SendMessage(freqSlider, TBM_SETTICFREQ, (WPARAM)5, 0);

		HWND freqEdit = GetDlgItem(hDlg, IDC_FREQ_EDIT);
		SendMessage(freqEdit, EM_SETLIMITTEXT, (WPARAM)3, 0);
		char buffer[4]; sprintf_s(buffer, "%d", a4freq);
		SetWindowText(freqEdit, buffer);

		HWND micCombo = GetDlgItem(hDlg, IDC_MIC_COMBO);
		for(const PaDeviceInfo* device : pTunerUI->mTuner.EnumerateAudioInputDevice())
			SendMessage(micCombo, CB_ADDSTRING, 0, (LPARAM)device->name);
		SendMessage(micCombo, CB_SETCURSEL, (WPARAM)pTunerUI->mTuner.GetDeviceIndex(), 0);

		if(pTunerUI->mTableOffset == 0)
			CheckRadioButton(hDlg, IDC_SHARP_RADIO, IDC_FLAT_RADIO, IDC_SHARP_RADIO);
		else
			CheckRadioButton(hDlg, IDC_SHARP_RADIO, IDC_FLAT_RADIO, IDC_FLAT_RADIO);

		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREF_CANCEL:
			DestroyWindow(hDlg);
			pTunerUI->mHDlgPreferences = NULL;
			return TRUE;
		case IDC_PREF_OK:
		{
			try {
				HWND freqEdit = GetDlgItem(hDlg, IDC_FREQ_EDIT);
				TCHAR buffer[4];
				GetWindowText(freqEdit, buffer, 4);
				int a4freq = atoi(buffer);
				if (a4freq < 415 || a4freq > 445)
					pTunerUI->mA4Freq = 440;
				else
					pTunerUI->mA4Freq = a4freq;

				HWND micCombo = GetDlgItem(hDlg, IDC_MIC_COMBO);
				int deviceIndex = SendMessage(micCombo, CB_GETCURSEL, 0, 0);
				pTunerUI->mTuner.ChangeInputDevice(deviceIndex);

				HWND sharpRadio = GetDlgItem(hDlg, IDC_SHARP_RADIO);
				if (SendMessage(sharpRadio, BM_GETCHECK, 0, 0))
					pTunerUI->mTableOffset = 0;
				else
					pTunerUI->mTableOffset = 12;

				DestroyWindow(hDlg);
				pTunerUI->mHDlgPreferences = NULL;
				return TRUE;
			}
			catch (std::invalid_argument ex) {
				MessageBox(NULL, ex.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
			}
		}
		case IDC_PREF_RESET:
		{
			HWND freqSlider = GetDlgItem(hDlg, IDC_FREQ_SLIDER);
			SendMessage(freqSlider, TBM_SETPOS, (WPARAM)TRUE, 440);
			HWND freqEdit = GetDlgItem(hDlg, IDC_FREQ_EDIT);
			SetWindowText(freqEdit, "440");
			return TRUE;
		}
		}
		break;
	case WM_HSCROLL:
	{
		HWND freqSlider = GetDlgItem(hDlg, IDC_FREQ_SLIDER);
		HWND freqEdit = GetDlgItem(hDlg, IDC_FREQ_EDIT);
		int currentPos = SendMessage(freqSlider, TBM_GETPOS, 0, 0);
		char buffer[4]; sprintf_s(buffer, "%d", currentPos);
		SetWindowText(freqEdit, buffer);
		return TRUE;
	}
	case WM_CLOSE:
		DestroyWindow(hDlg);
		pTunerUI->mHDlgPreferences = NULL;
		return TRUE;
	}

	return FALSE;
}
