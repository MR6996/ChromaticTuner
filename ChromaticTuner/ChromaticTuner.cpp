#include "TunerUI.h"
#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	if (SUCCEEDED(CoInitialize(NULL))) {
		try {
			TunerUI app;

			if (SUCCEEDED(app.Initialize()))
				app.RunMessageLoop();
			else
				MessageBox(NULL, "Can't run the program!", "Error", MB_ICONHAND);
		}
		catch (pa_error ex) {
			MessageBox(NULL, ex.getErrMessage(), "Audio input device error", MB_OK | MB_ICONHAND);
		}
		catch (std::bad_alloc ex) {
			MessageBox(NULL, "Can't allocate memory for the program.", "Error", MB_OK | MB_ICONHAND);
		}

		CoUninitialize();
	}
	else
		MessageBox(NULL, "Can't initialize COM library!", "Error", MB_ICONHAND);

	return 0;
}