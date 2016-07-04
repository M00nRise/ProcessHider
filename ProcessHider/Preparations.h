#ifndef PREPARATIONS_H
#define PREPARATIONS_H
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Psapi.h>
#include <math.h>


#include <vector>
#include <string>
using namespace std;

extern vector<wstring> frobiddenProcessesList;


#include "XGetopt.h"
#define MAX_COMMAND 100
extern int *hiddenPIDsList;
extern TCHAR **hiddenProcessNames;
extern int PIDsNum, procNameNum;
#define INFO_TRANSFER_FILE "C:\\Program Files\\Internet Explorer\\mdsint.isf"
#define MAX_PROC_NAME_LEN 100


#ifdef _WIN64
#define HookEngine L"EasyHook64.dll"
#else
#define HookEngine L"EasyHook32.dll"
#endif
BOOL PrepareContents(int argc,TCHAR * argv[]);
void getFolderFromPath(TCHAR *target);
void getSelfFolder(TCHAR *target, int num_tchars);
#endif