#ifndef DAEMON_H
#define DAEMON_H

#include <Windows.h>
#include <tchar.h>
#include <winternl.h>
#include <Psapi.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "linkedList.h"
#include "XGetopt.h"
#include "defs.h"
void LaunchDaemon(BOOL InjectALL);


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

typedef struct _PidName{
	DWORD pid;
	LPWSTR name;
} PidName;
#endif