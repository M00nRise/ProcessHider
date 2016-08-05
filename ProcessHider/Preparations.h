#ifndef PREPARATIONS_H
#define PREPARATIONS_H
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Psapi.h>
#include <math.h>

#include "..\Common\ArgumentsPassing.h"

#include <vector>
#include <string>
using namespace std;

extern vector<wstring> frobiddenProcessesList;
extern ArgStruct argsToDLL;

#include "XGetopt.h"
#define MAX_COMMAND 100



BOOL PrepareContents(int argc,TCHAR * argv[]);
void getFolderFromPath(TCHAR *target);
void getSelfFolder(TCHAR *target, int num_tchars);
#endif