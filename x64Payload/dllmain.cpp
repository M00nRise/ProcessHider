// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Payload.h"

#include "../Common/ArgumentsPassing.h"



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	pArgStruct args = (pArgStruct)lpReserved;

	switch (ul_reason_for_call)
		{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		if (!isUp)
		{
#ifdef DEBUG_MODE
			PrintToFile("Injected!");
#endif // DEBUG
			isUp = true;
			InitializeDLL(args);
		}
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		isUp = false;
		UnhookDLL();
		break;
	}
	return TRUE;
}

