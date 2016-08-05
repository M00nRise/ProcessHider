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
void LaunchDaemon();


typedef NTSTATUS(*PNtQueryFunc)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

typedef struct _PidName{
	DWORD pid;
	LPWSTR name;
} PidName;
#endif