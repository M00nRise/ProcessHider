// x64Inj.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Common\ArgumentsPassing.h"
#include "..\ProcessHider\Preparations.h"





int _tmain(int argc, _TCHAR* argv[]) 
{
	BOOL InjectAll;
	PrepareContents(argc,argv,&InjectAll);
    return 0;
}

