#ifndef NTHOOKENGINE_H
#define NTHOOKENGINE_H
#include "stdafx.h"

extern "C" void NtHookEngineInit();
extern "C" BOOL __cdecl HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
extern "C" VOID __cdecl UnhookFunction(ULONG_PTR Function);
extern "C" ULONG_PTR __cdecl GetOriginalFunction(ULONG_PTR Hook)
#endif