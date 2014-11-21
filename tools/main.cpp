
#include <windows.h>
#include <tchar.h>
#include <string>

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	char path[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH, path);

	const std::string base(path);
	const std::string bin(base + "/bin");
	const std::string lib(base + "/lib");
	
	//add lib dir to PATH
	const std::string path_var = std::string("PATH=") + getenv("PATH") + ";" + lib;
	_putenv(path_var.c_str());
	
	//some extra setup
	SetDllDirectory(lib.c_str());
	SetCurrentDirectory(bin.c_str());

	//launch process
	CreateProcess(NULL, "photo_broom.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}
