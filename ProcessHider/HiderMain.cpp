#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "Daemon.h"
#include "Preparations.h" 
#include "..\Common\DLLs_hex.h"
#define MAX_COMMANDLINE_LEN 1000
#define ID_BUFFER_SIZE 10
#define x64LauncherFile L"x64Hider.exe"

LPCWSTR x64filesList[] = { x64LauncherFile };
int x64resourceIDint[] = { IDR_RCDATA1 }; //dont care about x86, can't be called
int x64ResNum = 1;

void getFolderFromPath(TCHAR *target);
void getSelfFolder(TCHAR *target, int num_tchars);

//Get the folder of the exe
void getSelfFolder(TCHAR *target, int num_tchars)
{
	GetModuleFileNameEx(GetCurrentProcess(), NULL, target, num_tchars);
	getFolderFromPath(target);
}

//Extract the folder's name  (including the trailing \) from a file's path
void getFolderFromPath(TCHAR *target)
{
	TCHAR *x;
	TCHAR buffer[MAX_PATH] = L"\0";
	TCHAR *pwc1 = wcstok_s(target, L"\\/", &x), *pwc2 = L"";
	while (pwc1 != NULL)
	{

		wcscat_s(buffer, MAX_PATH, pwc2);
		if (wcslen(pwc2) != 0)
			wcscat_s(buffer, MAX_PATH, L"\\");
		pwc2 = pwc1;
		pwc1 = wcstok_s(NULL, L"\\/", &x);
	}
	target[0] = L'\0';
	wcscat_s(target, MAX_PATH, buffer);
	return;
}


//Check if this process is in elevated mode, because most threats require elevation to deal with
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
	IsWow64Process(GetCurrentProcess(), &res); //this code shall be compiled to 32-bit. If the process is WoW64 - The system is 64 bit 
	return res;
} 



// If we use 64-bit version, we need to copy the DLLs to the new process' memory to avoid disk writes
int WriteDLLsToProcess(PROCESS_INFORMATION pi)
{
	DWORD newPID = pi.dwProcessId;
	HANDLE hMutex = CreateMutexEx(NULL, MutexName, 0, SYNCHRONIZE);
	//Allocate and write
	LPVOID x64PayloadRemoteAddr = VirtualAllocEx(pi.hProcess, NULL, x64PayloadSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (x64PayloadRemoteAddr == NULL) ERROR_PRINT("Couldn't allocate memory for x64 Payload!\n");
	WriteProcessMemory(pi.hProcess, x64PayloadRemoteAddr, x64PayloadByteArr, x64PayloadSize, NULL);
	LPVOID x86PayloadRemoteAddr = VirtualAllocEx(pi.hProcess, NULL, x86PayloadSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (x86PayloadRemoteAddr == NULL) ERROR_PRINT("Couldn't allocate memory for x86 Payload!\n");
	WriteProcessMemory(pi.hProcess, x86PayloadRemoteAddr, x86PayloadByteArr, x86PayloadSize, NULL);
	//structure - x64Addr,x64Size,x86Addr,x86Size,flag (will be explained later)
	DWORD PayloadsAddresses[mappingSize] = { (DWORD)x64PayloadRemoteAddr,x64PayloadSize,(DWORD)x86PayloadRemoteAddr,x86PayloadSize ,1};
	HANDLE hMapFile = CreateFileMapping( //use file mapping to write remote addresses to process
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		mappingSize,                // maximum object size (low-order DWORD)
		mappingName);                 // name of mapping object
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		return FALSE;
	}
	DWORD *pBuf = (DWORD *)MapViewOfFile(hMapFile,FILE_MAP_ALL_ACCESS,0,0,mappingSize);
	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());
		CloseHandle(hMapFile);
		return FALSE;
	}
	CopyMemory((PVOID)pBuf, PayloadsAddresses, mappingSize);
	ResumeThread(pi.hThread);

	//How to get ACK from the remote process - if the flag from the array changes to zero - the process finished the job and we can clean
	int signalCheck = 1;
	while (signalCheck)
	{
		HANDLE hCheck=OpenMutex(SYNCHRONIZE, FALSE, MutexName);
		signalCheck = pBuf[numOfMappingItems-1];
		ReleaseMutex(hCheck);
		CloseHandle(hCheck);
	}
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile); //
	return TRUE;
}


//start a new process using a given command line
int CreateProcessFromLine(TCHAR *command_line,bool newConsole)
{
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(NULL, command_line, NULL, NULL, FALSE, CREATE_SUSPENDED | (newConsole ? CREATE_NEW_CONSOLE : 0), NULL, NULL, &si, &pi))
		return 0;
	return WriteDLLsToProcess(pi);
	
	
}

//Create the correct command line for starting the x64 file
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


//make a file from a resource
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
		printf("This process needs to run as an admin! Do you wish to continue? [y\\n]\n");
		char c=getchar();
		while (c != 'y' && c != 'Y' && c != 'n' && c != 'N')
		{
			if (c == '\n') continue;
			printf("Enter a valid char! Do you wish to continue without admin privilages? [y\\n]\n");
			c = getchar();

		}
		if(c=='n' || c=='N')
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
		CreateProcessFromLine(buffer,false);
		return 0;
	}
	BOOL InjectAll = FALSE;
	if(PrepareContents(argc, argv,&InjectAll))
	LaunchDaemon(InjectAll);
	return 0;
}
