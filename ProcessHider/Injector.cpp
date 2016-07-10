#include "Injector.h"
#include "Daemon.h"

#include <stdio.h>
#include <stdlib.h>




BOOL InjectorFunc(int procID, TCHAR *DLL_Address_local)
{
	TCHAR DLL_Address[MAX_PATH];
	GetModuleFileNameEx(GetCurrentProcess(), NULL, DLL_Address, MAX_PATH);
	getFolderFromPath(DLL_Address);
	wcscat_s(DLL_Address, MAX_PATH, DLL_Address_local);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	if (addr == NULL){
		printf("Error: the LoadLibraryA function was not found inside kernel32.dll library.\n");
		return FALSE;
	}

	/*
	* Allocate new memory region inside the process's address space.
	*/
	LPVOID arg = (LPVOID)VirtualAllocEx(hProcess, NULL, sizeof(TCHAR)* (wcslen(DLL_Address) + 1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL){
		printf("Error: the memory could not be allocated inside the chosen process.\n");
		return FALSE;
	}

	/*
	* Write the argument to LoadLibraryA to the process's newly allocated memory region.
	*/
	int n = WriteProcessMemory(hProcess, arg, DLL_Address, sizeof(TCHAR)* (wcslen(DLL_Address) + 1), NULL);
	if (n == 0){
		printf("Error: there was no bytes written to the process's address space.\n");
		return FALSE;
	}
	/*
	* Inject our DLL into the process's address space.
	*/
	HANDLE threadID = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
	if (threadID == NULL)
	{
		printf("Error: the remote thread could not be created.\n");
		return FALSE;
	}
	else
	{
		printf("Injected succesfully to %d\n", procID);
		return TRUE;
	}
	return FALSE;
}
