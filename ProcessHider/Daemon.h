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
void LaunchDaemon();
#ifdef _WIN64
#define PayloadName L"x64Payload.dll"
#else
#define PayloadName L"x86Payload.dll"
#endif

typedef NTSTATUS(*PNtQueryFunc)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
#define MAX_PROCESSES 1000
#define TIME_TO_INJECT_SEC 0.2
typedef struct _PidName{
	DWORD pid;
	LPWSTR name;
} PidName;
#endif