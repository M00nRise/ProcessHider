#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "Daemon.h"
#include "Preparations.h"
#define MAX_COMMANDLINE_LEN 1000
#define ID_BUFFER_SIZE 10
#define x64LauncherFile L"x64Hider.exe"

LPCWSTR x64filesList[] = { x64LauncherFile };
int x64resourceIDint[] = { IDR_RCDATA1 }; //dont care about x86, can't be called
int x64ResNum = 1;


BOOL IsElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}


BOOL isSystem64BitWow()
{
	BOOL res;
	IsWow64Process(GetCurrentProcess(), &res);
	return res;
} 
int CreateProcessLine(TCHAR *command_line,bool newConsole)
{
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	return CreateProcess(NULL, command_line, NULL, NULL, FALSE,newConsole? CREATE_NEW_CONSOLE:0, NULL, NULL, &si, &pi);
}

void createCommandLine(int argc,  _TCHAR* argv[], TCHAR *resBuffer, int resBufferLen) //create single line
{
	TCHAR folderBuffer[MAX_PATH];
	if (resBufferLen == 0 ||resBuffer==NULL)
		return;
	wcscpy_s(resBuffer,resBufferLen,L"\"");
	getSelfFolder(folderBuffer, MAX_PATH);
	wcscat_s(resBuffer, resBufferLen, folderBuffer);
	wcscat_s(resBuffer, resBufferLen, x64LauncherFile);
	wcscat_s(resBuffer, resBufferLen, L"\"");

	int i;
	for (i = 1; i < argc; i++) //first arg is irrelevant
	{
		wcscat_s(resBuffer, resBufferLen, L" \"");
		wcscat_s(resBuffer, resBufferLen, argv[i]);
		wcscat_s(resBuffer, resBufferLen, L"\"");
	}
}

BOOL CopyResourceIntoFile(LPCWSTR ResourceName, LPTSTR  resID)
{
	TCHAR resFullName[MAX_PATH];
	HRSRC hrsrc = FindResource(NULL, resID, MAKEINTRESOURCE(RT_RCDATA));

	HGLOBAL hglobal = LoadResource(NULL, hrsrc);
	DWORD res_size = SizeofResource(NULL, hrsrc), returned_size;
	LPVOID pResource = LockResource(hglobal);
	if (!(hrsrc && hglobal && res_size && pResource))
	{
		return FALSE;
	}
	getSelfFolder(resFullName,MAX_PATH);
	wcscat_s(resFullName, MAX_PATH, ResourceName);
	HANDLE fileHandle = CreateFile(resFullName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL write_res = WriteFile(fileHandle, pResource, res_size, &returned_size, NULL);
	CloseHandle(fileHandle);
	return write_res;
}

int _tmain(int argc, _TCHAR* argv[])
{

	if (!IsElevated())
	{
		printf("This process needs to run as an admin!\nRun again please\n");
		return 1;
	}
	if (isSystem64BitWow())
	{
		TCHAR buffer[MAX_COMMANDLINE_LEN];
		for (int i = 0; i < x64ResNum; i++)
		{
			TCHAR resource_int_buffer[ID_BUFFER_SIZE];
			_itow_s(x64resourceIDint[i], resource_int_buffer, ID_BUFFER_SIZE, 10);
			if (!CopyResourceIntoFile(x64filesList[i], MAKEINTRESOURCE(x64resourceIDint[i])))
			{
				printf("Failed building %ws, exiting!\n", x64filesList[i]);
				return 1;
			}
		}
		createCommandLine(argc, argv, buffer, MAX_COMMANDLINE_LEN);
		CreateProcessLine(buffer,false);
		return 0;
	}

	if(PrepareContents(argc, argv))
	LaunchDaemon();
	return 0;
}
