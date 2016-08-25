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
#include "w64wow64.h"
#include "w64wow64defs.h"
#include "internal.h"
#include "windef.h"


FUNCTIONPTRS sFunctions = { 0 };



void PrintLastError(void) {
	DWORD dwErrorId = NULL;
	LPSTR lpaszErrorMessage[512];
	dwErrorId = GetLastError();
	FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM + FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwErrorId, NULL, (LPSTR)&lpaszErrorMessage, 512, NULL );
	MessageBoxA( 0, (LPCSTR)lpaszErrorMessage, "Error", 0 );

}

/**
*
* X64Call Part of WOW64Ext Library
* See internals.h
*/
extern __declspec(dllexport) unsigned __int64 X64Call( void * lvpFunctionPtr, 
	int nArgc, ... )
{
	va_list args;
	DWORD64 arg1, arg2, arg3, arg4, _nArgc, _lvpFunctionPtr, rest;
	DWORD dwEspBackup;
	union reg64 sRax;

	va_start( args, nArgc );
	arg1 = ( nArgc ) ? nArgc--, va_arg( args, DWORD64 ) : 0;
	arg2 = ( nArgc ) ? nArgc--, va_arg( args, DWORD64 ) : 0;
	arg3 = ( nArgc ) ? nArgc--, va_arg( args, DWORD64 ) : 0;
	arg4 = ( nArgc ) ? nArgc--, va_arg( args, DWORD64 ) : 0;

	rest = (DWORD64)&va_arg( args, DWORD64 );

	_nArgc = nArgc;
	_lvpFunctionPtr = (DWORD64)lvpFunctionPtr;
	dwEspBackup;
	sRax.v = 0;
	__asm {
		mov dwEspBackup, esp
		and sp, 0xFFF8
		X64_Start();
		push arg1
		X64_Pop(_RCX);
		push arg2
		X64_Pop(_RDX);
		push arg3
		X64_Pop(_R8);
		push arg4
		X64_Pop(_R9);
		
		push   edi
		push   rest
		X64_Pop(_RDI);
		push   _nArgc
		X64_Pop(_RAX);
		test   eax, eax
		jz     _ls_e
		lea    edi, dword ptr [edi + 8*eax - 8]
	_ls:
		test   eax, eax
		jz     _ls_e
		push   dword ptr [edi]
		sub    edi, 8
		sub    eax, 1
		jmp    _ls
	_ls_e:
		sub    esp, 0x20
		call   _lvpFunctionPtr
		push   _nArgc
		X64_Pop(_RCX);
		lea    esp, dword ptr [esp + 8*ecx + 0x20]
		pop    edi
		X64_Push(_RAX);
		pop    sRax.dw[0]
		X64_End();
		mov    esp, dwEspBackup
	}
}

PTEB64 NtTeb64( void )
{
	X64_Start();
	GETTEB();
	X64_End();
}

PLDR_DATA_TABLE_ENTRY64 GetModule64LdrTable( wchar_t * lwcModuleName )
{
	PTEB64 psTeb = NtTeb64();
	//PPEB64 psPeb = 
	PPEB_LDR_DATA Ldr = psTeb->ProcessEnvironmentBlock->Ldr;
	PLDR_DATA_TABLE_ENTRY64 psDataEntryStart = 
		(PLDR_DATA_TABLE_ENTRY64)Ldr->InLoadOrderModuleList.Flink;
	PLDR_DATA_TABLE_ENTRY64 psDataEntryCurrent = psDataEntryStart;

	do {
		if( memcmp( (void *)psDataEntryCurrent->BaseDllName.Buffer, lwcModuleName, 
			psDataEntryCurrent->BaseDllName.Length ) == 0 ) {
				return psDataEntryCurrent;
		}
		psDataEntryCurrent = 
			(PLDR_DATA_TABLE_ENTRY64)psDataEntryCurrent->InLoadOrderLinks.Flink;
	} while( psDataEntryStart != psDataEntryCurrent && psDataEntryCurrent );
	return NULL;
}

void * GetModuleBase64( wchar_t * lwcModuleName )
{
	PLDR_DATA_TABLE_ENTRY64 LdrEntry = GetModule64LdrTable( lwcModuleName );
	return (void *)LdrEntry->DllBase;
}

PIMAGE_NT_HEADERS64 GetModule64NtHeader( void * lvpBaseAddress )
{
	PIMAGE_DOS_HEADER psDosHeader = (PIMAGE_DOS_HEADER)lvpBaseAddress;
	return (PIMAGE_NT_HEADERS64)( ((__int8 *)lvpBaseAddress) + 
		psDosHeader->e_lfanew );
}
void * GetModule64PEBaseAddress( void * lvpBaseAddress )
{
	PIMAGE_NT_HEADERS64 psNtHeader = GetModule64NtHeader( lvpBaseAddress );
	return (void *)psNtHeader->OptionalHeader.ImageBase;
}

void * GetModule64EntryRVA( void * lvpBaseAddress )
{
	PIMAGE_NT_HEADERS64 psNtHeader = GetModule64NtHeader( lvpBaseAddress );
	return (void *)psNtHeader->OptionalHeader.AddressOfEntryPoint;
}

extern __declspec(dllexport) void * GetProcAddress64( void * lvpBaseAddress, char * lpszProcName )
{
	PIMAGE_NT_HEADERS64 psNtHeader = GetModule64NtHeader( lvpBaseAddress );
	char * lpcModBase = (char *)lvpBaseAddress;
	PIMAGE_EXPORT_DIRECTORY psExportDir = (PIMAGE_EXPORT_DIRECTORY)( lpcModBase + 
		psNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress );

	int nNumberOfNames = psExportDir->NumberOfNames;
	unsigned long * lpulFunctions = 
		(unsigned long *)( lpcModBase + psExportDir->AddressOfFunctions );

	unsigned long * lpulNames = 
		(unsigned long *)( lpcModBase + psExportDir->AddressOfNames );

	unsigned short * lpusOrdinals = 
		(unsigned short *) ( lpcModBase + psExportDir->AddressOfNameOrdinals );

	int i;
	char * lpszFunctionName;
	for( i = 0; i < nNumberOfNames; i++ ) {
		lpszFunctionName = ((__int8 *)lpulNames[i]) + (int)lvpBaseAddress;
		if( strcmp( lpszFunctionName, lpszProcName ) == 0 ) {
			return  ( (__int8 *)lvpBaseAddress ) + 
				lpulFunctions[ lpusOrdinals[i] ];
		}
	}
	return NULL;
}

BOOL FreeKnownDllPage( wchar_t * lpwzKnownDllName )
{
	DWORD64 hSection = 0;
	DWORD64 lvpBaseAddress = 0;
	DWORD64 lvpRealBaseAddress = 0;
	DWORD64 stViewSize = 0;
	DWORD64 stRegionSize = 0;
	PTEB64 psTeb;
	X64Call( sFunctions.LdrGetKnownDllSectionHandle, 3, 
		(DWORD64)lpwzKnownDllName, 
		(DWORD64)0, 
		(DWORD64)&hSection );

	psTeb = NtTeb64();
	psTeb->NtTib.ArbitraryUserPointer = (DWORD64)lpwzKnownDllName;

	X64Call( sFunctions.NtMapViewOfSection, 10, 
		(DWORD64)hSection, 
		(DWORD64)-1, 
		(DWORD64)&lvpBaseAddress, 
		(DWORD64)0, 
		(DWORD64)0, 
		(DWORD64)0, 
		(DWORD64)&stViewSize, 
		(DWORD64)ViewUnmap, 
		(DWORD64)0, 
		(DWORD64)PAGE_READONLY );

	lvpRealBaseAddress = 
		(DWORD64)GetModule64PEBaseAddress( (void *)lvpBaseAddress );

	X64Call( sFunctions.NtFreeVirtualMemory, 4, 
		(DWORD64)-1, 
		(DWORD64)&lvpRealBaseAddress, 
		(DWORD64)&stRegionSize, 
		(DWORD64)MEM_RELEASE );

	X64Call( sFunctions.NtUnmapViewOfSection, 2, (DWORD64)-1, 
		(DWORD64)lvpBaseAddress );
	return TRUE;
}

extern __declspec(dllexport) void * LoadLibrary64A( char * lpcLibraryName )
{
	if( sFunctions.LoadLibraryA == NULL ) {
		sFunctions.LoadLibraryA = 
			GetProcAddress64( GetModuleBase64( L"kernel32.dll" ), "LoadLibraryA" );
	}
	return (void *)X64Call( sFunctions.LoadLibraryA, 1, (DWORD64)lpcLibraryName );
}

BOOL InitializeW64oWoW64()
{

	void * lvpNtdll = GetModuleBase64( L"ntdll.dll" );
	UNICODE_STRING64 sUnicodeString;
	__int8 * lvpKernelBaseBase;
	__int8 * lvpKernel32Base;
	PLDR_DATA_TABLE_ENTRY64 lpsKernel32Ldr;
	PLDR_DATA_TABLE_ENTRY64 lpsKernelBaseLdr;

	sFunctions.LdrGetKnownDllSectionHandle = GetProcAddress64( lvpNtdll, 
		"LdrGetKnownDllSectionHandle" );
	sFunctions.NtFreeVirtualMemory = GetProcAddress64( lvpNtdll, 
		"NtFreeVirtualMemory" );
	sFunctions.NtMapViewOfSection = GetProcAddress64( lvpNtdll, 
		"NtMapViewOfSection" );
	sFunctions.NtUnmapViewOfSection = GetProcAddress64( lvpNtdll, 
		"NtUnmapViewOfSection" );

	if( FreeKnownDllPage( L"kernel32.dll" ) == FALSE) return FALSE;
	if( FreeKnownDllPage( L"user32.dll" ) == FALSE ) return FALSE;

	sUnicodeString.Length = 0x18;
	sUnicodeString.MaximumLength = 0x1a;
	sUnicodeString.Buffer = (DWORD64)L"kernel32.dll";
	if( X64Call( GetProcAddress64( lvpNtdll, "LdrLoadDll" ), 4, 
		(DWORD64)0, 
		(DWORD64)0, 
		(DWORD64)&sUnicodeString, 
		(DWORD64)&lvpKernel32Base ) != NULL ) {
			PrintLastError();
			return FALSE;
	}

	lvpKernelBaseBase = (__int8 *)GetModuleBase64( L"KERNELBASE.dll");
	X64Call( ( lvpKernelBaseBase + (int)GetModule64EntryRVA( lvpKernelBaseBase ) ), 
		3, 
		(DWORD64)lvpKernelBaseBase, 
		(DWORD64)DLL_PROCESS_ATTACH, 
		(DWORD64)0 );

	X64Call( ( lvpKernel32Base + (int)GetModule64EntryRVA( lvpKernel32Base ) ), 
		3, 
		(DWORD64)lvpKernel32Base, 
		(DWORD64)DLL_PROCESS_ATTACH, 
		(DWORD64)0 );

	lpsKernel32Ldr = GetModule64LdrTable( L"kernel32.dll" );
	lpsKernel32Ldr->LoadCount = 0xffff;
	lpsKernel32Ldr->Flags += LDRP_ENTRY_PROCESSED | LDRP_PROCESS_ATTACH_CALLED;

	lpsKernelBaseLdr = GetModule64LdrTable( L"KERNELBASE.dll" );
	lpsKernelBaseLdr->LoadCount = 0xffff;
	lpsKernelBaseLdr->Flags += LDRP_ENTRY_PROCESSED | LDRP_PROCESS_ATTACH_CALLED;

	return TRUE;
}