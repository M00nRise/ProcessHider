#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "../ProcessHider/Daemon.h"
#include "../ProcessHider/Preparations.h"
int _tmain(int argc, _TCHAR* argv[])
{
	if(PrepareContents(argc, argv))
	LaunchDaemon();
}