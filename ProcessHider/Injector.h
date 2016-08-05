#ifndef INJECTOR_H
#define INJECTOR_H

#include <Windows.h>
#include <stdio.h>
#include "Preparations.h"
BOOL InjectorFunc(DWORD dwProcessId, BOOL isTarget64Bit);
#endif
