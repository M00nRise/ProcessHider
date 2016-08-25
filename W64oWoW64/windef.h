/*
W64oWoW64
Copyright (C) 2012  George Nicolaou <nicolaou.george[at]gmail.[dot]com>

This file is part of W64oWoW64.

W64oWoW64 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

W64oWoW64 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with W64oWoW64.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <Windows.h>

typedef struct _CLIENT_ID64
{
	DWORD64 UniqueProcess;
	DWORD64 UniqueThread;
} CLIENT_ID64, *PCLIENT_ID64;


typedef struct _PEB_LDR_DATA
{
	ULONG Length;
	UCHAR Initialized;
	DWORD64 SsHandle;
	LIST_ENTRY64 InLoadOrderModuleList;
	LIST_ENTRY64 InMemoryOrderModuleList;
	LIST_ENTRY64 InInitializationOrderModuleList;
	DWORD64 EntryInProgress;
} PEB_LDR_DATA, *PPEB_LDR_DATA;


typedef struct _PEB64
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG ImageUsesLargePages: 1;
	ULONG IsProtectedProcess: 1;
	ULONG IsLegacyProcess: 1;
	ULONG IsImageDynamicallyRelocated: 1;
	ULONG SpareBits: 4;
	DWORD64 Mutant;
	DWORD64 ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
} PEB64, *PPEB64;

typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	DWORD64 Buffer;
} UNICODE_STRING64, * PUNICODE_STRING64;

typedef struct _TEB64
{
	NT_TIB64 NtTib;
	DWORD64 EnvironmentPointer;
	CLIENT_ID64 ClientId;
	DWORD64 ActiveRpcHandle;
	DWORD64 ThreadLocalStoragePointer;
	PPEB64 ProcessEnvironmentBlock;
} TEB64, *PTEB64;

typedef struct _LDR_DATA_TABLE_ENTRY64
{
	LIST_ENTRY64 InLoadOrderLinks;
	LIST_ENTRY64 InMemoryOrderLinks;
	LIST_ENTRY64 InInitializationOrderLinks;
	DWORD64 DllBase;
	DWORD64 EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG Flags;
	WORD LoadCount;
} LDR_DATA_TABLE_ENTRY54, *PLDR_DATA_TABLE_ENTRY64;

#define LDRP_PROCESS_ATTACH_CALLED	0x000080000
#define LDRP_ENTRY_PROCESSED		0x000004000

typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT, * PSECTION_INHERIT;