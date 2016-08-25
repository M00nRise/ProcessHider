/*
W64oWoW64
Copyright (C) 2012  George Nicolaou <nicolaou.george[at]gmail.[dot]com>

This file is part of W64oWoW64.

W64oWoW64 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

W64oWoW64 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with W64oWoW64.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __W64WOW64_H_
#define __W64WOW64_H_
#include <Windows.h>
__declspec(dllexport) unsigned __int64 X64Call( void * lvpFunctionPtr, 
	int nArgc, ... );
__declspec(dllexport) void * GetProcAddress64( void * lvpBaseAddress, 
	char * lpszProcName );
__declspec(dllexport) void * LoadLibrary64A( char * lpcLibraryName );
 BOOL __cdecl InitializeW64oWoW64();
#endif