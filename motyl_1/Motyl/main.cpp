#include "gk2_butterfly.h"
#include "gk2_window.h"
#include "gk2_exceptions.h"

using namespace std;
using namespace gk2;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	unique_ptr<ApplicationBase> app;
	int exitCode;
	try
	{
		app = make_unique<Butterfly>(hInstance);
		exitCode = app->Run(make_unique<Window>(hInstance, 800, 800, L"Triangle Demo"), cmdShow);
	}
	catch (Exception& e)
	{
		MessageBoxW(nullptr, e.getMessage().c_str(), L"B³¹d", MB_OK);
		exitCode = e.getExitCode();
	}
	catch(...)
	{
		MessageBoxW(nullptr, L"Nieznany B³¹d", L"B³¹d", MB_OK);
		exitCode = -1;
	}
	app.reset();
	return exitCode;
}