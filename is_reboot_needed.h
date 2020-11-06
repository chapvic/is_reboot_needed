/***************************************************************************

MIT License

Copyright (c) 2019-2020 FoxTeam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*****************************************************************************/

#ifndef IS_REBOOT_NEEDED_H
#define IS_REBOOT_NEEDED_H

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif // UNICODE

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif // _UNICODE

#if defined(UNICODE) && defined(_MBCS)
#undef _MBCS
#endif

#include <tchar.h>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32")
#endif

#define REBOOT_STATUS_CLEAN            0UL
#define REBOOT_STATUS_RENAME_PENDING   1UL
#define REBOOT_STATUS_REBOOT_PENDING   2UL
#define REBOOT_STATUS_REBOOT_REQUIRED  4UL

#ifndef MemAlloc
#define MemAlloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#endif

#ifndef MemFree
#define MemFree(p) if(p){HeapFree(GetProcessHeap(),0,(p));(p)=NULL;}
#endif

#undef _bitcheck
#define _bitcheck(a,v) ((a)&(v))==(v)

int __cdecl is_reboot_needed(int * status);
int __cdecl is_reboot_needed_ex(int * status, LPBYTE * files);

#endif
