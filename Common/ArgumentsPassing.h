#ifndef ARGUMENTS_PASSING_H
#define ARGUMENTS_PASSING_H

#include <windows.h>
#include <wchar.h>
#define MAX_PIDS_TO_SEND 20
#define MAX_PROC_NAMES_LINE_LEN 200

typedef struct _arguments_struct
{
	int pIDsNum = 0, procNamesLength = 0;
	int pIDs[MAX_PIDS_TO_SEND];
	TCHAR procNames[MAX_PROC_NAMES_LINE_LEN]= L"\0";
} ArgStruct,*pArgStruct;

#define mappingName L"ProcMap"
#define numOfMappingItems 5
#define mappingSize  ((numOfMappingItems) * (sizeof(DWORD)))
#define MutexName L"ProcMutex"

#endif // !ARGUMENTS_PASSING_H

