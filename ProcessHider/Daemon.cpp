#include "Daemon.h"
#include "Injector.h"


BOOL isProcess64Bit(HANDLE hProcess)
{
#ifndef _WIN64 
	return FALSE;
#else
	BOOL res;
	IsWow64Process(hProcess, &res);
	return !res;
#endif // 

}

BOOL wstrcmp_ignorecase(LPCWSTR a, LPCWSTR b) //easier to implement than use existing functions
{
	int i = 0;
	TCHAR ax = a[0], bx = b[0];
	const int caseDiff = 'a' - 'A';
	while (ax != 0 && bx != 0)
	{
		if (ax == 0 || bx == 0)
			return 1;
		if (!(ax == bx || ax == bx + caseDiff || ax + caseDiff == bx))
			return 1;
		i++;
		ax = a[i]; bx = b[i];
	}
	return 0;
}

BOOL isInFrobProcList(LPCWSTR proc)
{
	UINT i;
	for (i = 0; i < frobiddenProcessesList.size(); i++)
	{
		if (!wstrcmp_ignorecase(frobiddenProcessesList[i].c_str(), proc))
			return TRUE;
	}
	return FALSE;
}

void reactToProcess(DWORD pid, LPWSTR name)
{
	
	if (!isInPidList(pid))
	{
		addData(pid);
		PidName x;
		x.pid = pid;
		x.name = name;
	}
	else
	{
		if ((!getOnline(pid)) && timeFromCreation(pid) >= TIME_TO_INJECT_SEC) //if you inject immediately, you might crash some processes
		{
			printf("Alert! %ws is online! pid=%d\n", name, pid);
			InjectorFunc(pid,isProcess64Bit(OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid)));
			setOnline(pid);
		}
	}
}



void startDaemonScan()
{

	while (1)
	{
		DWORD procIDsBuffer[MAX_PROCESSES];
		DWORD size_returned;

		EnumProcesses(procIDsBuffer, sizeof(procIDsBuffer), &size_returned);
		int numProc = size_returned / sizeof(DWORD);
		int i;
		TCHAR strBuffer[MAX_PATH];
		for (i = 0; i < numProc; i++)
		{
			DWORD curr_pid = (int)procIDsBuffer[i];
			HANDLE Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, curr_pid);
			GetModuleBaseName(Handle, NULL, strBuffer, MAX_PATH);
			if ((Handle) && isInFrobProcList(strBuffer))
			{
				reactToProcess(curr_pid, strBuffer);
			}
			CloseHandle(Handle);
		}
		updateList();
	}
}


void LaunchDaemon()
{

	printf("Daemon PID: %d\n", GetCurrentProcessId());
	startDaemonScan();
}