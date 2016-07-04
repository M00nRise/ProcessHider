#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#define MAX_COMMANDLINE_LEN 1000
#define x64LauncherFile L"\"x64 Launcher.exe\""

int isSystem64Bit()
{
	BOOL res;
	IsWow64Process(GetCurrentProcess(), &res);
	return !res;
}

void CreateProcessLine(TCHAR *command_line,BOOL newConsole)
{
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter

	STARTUPINFO StartupInfo; //This is an [in] parameter

	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	int x=CreateProcess(NULL, command_line, NULL, NULL, FALSE,newConsole? CREATE_NEW_CONSOLE:0, NULL, NULL, &si, &pi);
	int q = GetLastError();
}

void createCommandLine(int argc,  _TCHAR* argv[], TCHAR *resBuffer, int resBufferLen)
{
	if (resBufferLen == 0 ||resBuffer==NULL)
		return;
	wcscpy_s(resBuffer, resBufferLen, x64LauncherFile);
	int i;
	for (i = 1; i < argc; i++) //first arg is irrelevant
	{
		wcscat_s(resBuffer, resBufferLen, L" \"");
		wcscat_s(resBuffer, resBufferLen, argv[i]);
		wcscat_s(resBuffer, resBufferLen, L"\"");
	}
}
int _tmain(int argc, _TCHAR* argv[])
{

	if (isSystem64Bit)
	{
		TCHAR buffer[MAX_COMMANDLINE_LEN];
		createCommandLine(argc, argv, buffer, MAX_COMMANDLINE_LEN);
		CreateProcessLine(buffer,FALSE);
		return 0;
	}
	//getchar();
	return 0;
}
