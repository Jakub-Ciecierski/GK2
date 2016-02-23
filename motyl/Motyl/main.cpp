#include "gk2_butterfly.h"
#include "gk2_window.h"
#include "gk2_exceptions.h"

using namespace std;
using namespace gk2;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	shared_ptr<ApplicationBase> app;
	shared_ptr<Window> w;
	int exitCode = 0;
	try
	{
		app.reset(new Butterfly(hInstance));
		w.reset(new Window(hInstance, 800, 800, L"Triangle Demo"));
		exitCode = app->Run(w.get(), cmdShow);
	}
	catch (Exception& e)
	{
		MessageBoxW(NULL, e.getMessage().c_str(), L"B³¹d", MB_OK);
		exitCode = e.getExitCode();
	}
	catch(...)
	{
		MessageBoxW(NULL, L"Nieznany B³¹d", L"B³¹d", MB_OK);
		exitCode = -1;
	}
	w.reset();
	app.reset();
	return exitCode;
}