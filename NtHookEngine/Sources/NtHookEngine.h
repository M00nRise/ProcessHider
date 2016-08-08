#ifndef NTHOOKENGINE_H
#define NTHOOKENGINE_H
#include "stdafx.h"

void NtHookEngineInit();
BOOL __cdecl HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID __cdecl UnhookFunction(ULONG_PTR Function);
 ULONG_PTR __cdecl GetOriginalFunction(ULONG_PTR Hook);
#endif