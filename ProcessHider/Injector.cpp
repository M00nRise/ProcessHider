////===============================================================================================//
// Copyright (c) 2012, Stephen Fewer of Harmony Security (www.harmonysecurity.com)
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted 
// provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice, this list of 
// conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above copyright notice, this list of 
// conditions and the following disclaimer in the documentation and/or other materials provided 
// with the distribution.
// 
//     * Neither the name of Harmony Security nor the names of its contributors may be used to
// endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//===============================================================================================//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "LoadLibraryR.h"
#include "..\Common\DLLs_hex.h"
#pragma comment(lib,"Advapi32.lib")

#define BREAK_WITH_ERROR( e ) { printf( "[-] %s. Error=%d\n", e, GetLastError() ); return FALSE; }
#include "Injector.h"
#include "Daemon.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
//if you want to use reflective injector, define REFLECTIVE_INJ_FUNC


BOOL InjectorFunc(DWORD dwProcessId, BOOL isTarget64Bit)
{
	HANDLE hFile = NULL;
	HANDLE hModule = NULL;
	HANDLE hProcess = NULL;
	HANDLE hToken = NULL;
	LPVOID lpBuffer = NULL;
	DWORD dwLength = 0;
	DWORD dwBytesRead = 0;
	TOKEN_PRIVILEGES priv = { 0 };

	do
	{


		dwLength = isTarget64Bit ? x64PayloadSize : x86PayloadSize;
		if (dwLength == INVALID_FILE_SIZE || dwLength == 0)
			BREAK_WITH_ERROR("Failed to get the DLL file size");
		lpBuffer = HeapAlloc(GetProcessHeap(), 0, dwLength);
		if (!lpBuffer)
			BREAK_WITH_ERROR("Failed to get the DLL file size");


		if(0!=memcpy_s(lpBuffer, dwLength, isTarget64Bit ? x64PayloadByteArr : x86PayloadByteArr, isTarget64Bit ? x64PayloadSize : x86PayloadSize))
			BREAK_WITH_ERROR("Failed to copy buffer!");
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			priv.PrivilegeCount = 1;
			priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
				AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

			CloseHandle(hToken);
		}

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, dwProcessId);
		if (!hProcess)
			BREAK_WITH_ERROR("Failed to open the target process");

		hModule = LoadRemoteLibraryR(hProcess, lpBuffer, dwLength, &argsToDLL);
		if (!hModule)
			BREAK_WITH_ERROR("Failed to inject the DLL");
		TCHAR cpDllFile[DLL_NAME_LENGTH];
		wcscpy_s(cpDllFile, DLL_NAME_LENGTH, isTarget64Bit ? x64PayloadName: x86PayloadName);
		printf("[+] Injected the '%ws' DLL into process %d.\n", cpDllFile, dwProcessId);

		WaitForSingleObject(hModule, -1);

	} while (0);

	if (lpBuffer)
		HeapFree(GetProcessHeap(), 0, lpBuffer);

	if (hProcess)
		CloseHandle(hProcess);

	return TRUE;
}



