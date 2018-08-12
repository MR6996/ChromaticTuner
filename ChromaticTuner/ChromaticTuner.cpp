#include "TunerUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	if (SUCCEEDED(CoInitialize(NULL))) {
		TunerUI app;

		if (SUCCEEDED(app.Initialize()))
			app.RunMessageLoop();
		else 
			MessageBox(NULL, "Can't create the window!", "Error", MB_ICONHAND);

		CoUninitialize();
	}
	else
		MessageBox(NULL, "Can't initialize COM library!", "Error", MB_ICONHAND);

	return 0;
}