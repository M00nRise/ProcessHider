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
#define GETTEB() \
{	\
	EMIT(0x65) EMIT(0x48) EMIT(0x8b) EMIT(0x04) EMIT(0x25) EMIT(0x30) EMIT(0x00) EMIT(0x00) EMIT(0x00) \
}


typedef struct {
	void * LdrGetKnownDllSectionHandle;
	void * NtMapViewOfSection;
	void * NtFreeVirtualMemory;
	void * NtUnmapViewOfSection;
	void * LoadLibraryA;
} FUNCTIONPTRS;

BOOL InitializeW64oWoW64( void );