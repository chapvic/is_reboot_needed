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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "is_reboot_needed.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef __GNUC__
#pragma comment(lib, "advapi32")
#endif

static TCHAR * __cdecl basename(TCHAR * path) {
    TCHAR * p = _tcschr(path, 0);
    while (p > path && !(p[-1] == _T('/') || p[-1] == _T('\\'))) p--;
    return p;
}

static void __cdecl logo(void) {
	_ftprintf(stdout, _T("Is reboot needed, version 0.3.0\n"));
	_ftprintf(stdout, _T("Copyright (c) 2019-2021, FoxTeam\n\n"));
};

__declspec(noinline)
static void __cdecl usage(TCHAR *cmd) {
	logo();
	_ftprintf(stdout, _T("Usage: %s [options]\n"), basename(cmd));
	_ftprintf(stdout, _T("    -h      this help\n"));
	_ftprintf(stdout, _T("    -a      combination of -s, -d, -f\n"));
	_ftprintf(stdout, _T("    -s      show status information (default)\n"));
	_ftprintf(stdout, _T("    -d      list driver updates\n"));
	_ftprintf(stdout, _T("    -f      list update pending files\n"));
	_ftprintf(stdout, _T("    -n      suppress logo\n"));
	_ftprintf(stdout, _T("    -q      don't show any messages (quiet mode)\n"));
	_ftprintf(stdout, _T("    -r      automatic reboot if needed with no messages\n"));
	_ftprintf(stdout, _T("    -i      start reboot if notification is active (Windows 10)\n"));
	_ftprintf(stdout, _T("            (implies `-r` option)\n"));
	exit(0);
}

__declspec(noinline)
int __cdecl pending_files(LPBYTE * _files, int _print) {
	TCHAR *p, *pp;
	int cnt = 0;
	if (!_files) goto Exit;
	p = (TCHAR *)*_files;
	do {
		pp = _tcschr(p, 0);
		if (_print) _ftprintf(stdout, _T("  (%d) %s\n"), ++cnt, p+4);
		else ++cnt;
	} while(*(p = pp + 2));
Exit:
	return cnt;
}

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

int __cdecl check_drivers(HKEY hKey, int detailed) { 
	int retval = 0;
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
//	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = _T("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys=0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 
 
	DWORD i, retCode; 
 
	TCHAR  achValue[MAX_VALUE_NAME]; 
	DWORD cchValue = MAX_VALUE_NAME; 
 
	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time

	retval = cValues;

	// Display updates count
        _ftprintf(stdout, _T("Driver updates   : %ld\n"), cValues);

    if (cValues) {
	// Get registry timestamp
	SYSTEMTIME st;
	FileTimeToSystemTime(&ftLastWriteTime, &st);
        _ftprintf(stdout, _T("Last update time : %02d.%02d.%04d %02d:%02d:%02d\n"),
		st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
	
	if (!detailed) return retval;
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);
            if (retCode == ERROR_SUCCESS) { 
                _tprintf(_T("  (%ld) "), i+1);

		HKEY drvKey, clsKey;
		_tcscpy(achKey, _T("SYSTEM\\CurrentControlSet\\Enum\\"));
		_tcscat(achKey, achValue);
		if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, achKey, 0, KEY_READ, &drvKey))) {
			achValue[0] = '\0';
			cchValue = MAX_VALUE_NAME;
			if (SUCCEEDED(RegQueryValueEx(drvKey, _T("Driver"), NULL, NULL, (LPBYTE)&achValue, &cchValue)) && achValue[0]) {
				// Load driver information
				_tcscpy(achKey, _T("SYSTEM\\CurrentControlSet\\Control\\Class\\"));
				_tcscat(achKey, achValue);
				if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, achKey, 0, KEY_READ, &clsKey))) {
					// Get provider name
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, _T("ProviderName"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_ftprintf(stdout, _T(" %s"), achValue);
					// Get driver decription
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, _T("DriverDesc"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_ftprintf(stdout, _T(" - %s"), achValue);
					// Get driver version
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, _T("DriverVersion"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_ftprintf(stdout, _T(" - %s\n"), achValue);
				}
			}
			RegCloseKey(drvKey);
			_ftprintf(stdout, _T("---\n"));
		}
            } 
        }
    }
	return retval;
}

int _tmain(int argc, TCHAR * argv[]) {
	HANDLE hToken;
	TOKEN_PRIVILEGES * NewState = NULL;
	BOOL notification = FALSE;
	LPBYTE files = NULL;
	int needed = 0, status = 0, argn = 0;
	int _h = 0, _s = 1, _d = 0, _f = 0, _n = 0, _q = 0, _r = 0, _i = 0;
	TCHAR state[100] = {0};
	TCHAR error[256] = {0};

	if (argc > 1) {
		while (++argn < argc) {
			if (!_tcsicmp(argv[argn], _T("-h"))) _h = 1;
			else if (!_tcsicmp(argv[argn], _T("-a"))) { _s = 1; _d = 1, _f = 1; }
			else if (!_tcsicmp(argv[argn], _T("-s"))) _s = 1;
			else if (!_tcsicmp(argv[argn], _T("-d"))) _d = 1;
			else if (!_tcsicmp(argv[argn], _T("-f"))) _f = 1;
			else if (!_tcsicmp(argv[argn], _T("-n"))) _n = 1;
			else if (!_tcsicmp(argv[argn], _T("-q"))) _q = 1;
			else if (!_tcsicmp(argv[argn], _T("-r"))) { _r = 1; _q = 1; }
			else if (!_tcsicmp(argv[argn], _T("-i"))) { _i = 1; _r = 1; }
			else _h = 1;
		}
	}
	if (_h) {
		usage(argv[0]);
		goto Exit;
	}

	//	
	// Do check for reboot is needed
	//
	needed = is_reboot_needed_ex(&status, &files);
	//	
	// Find a reboot notification process (when updates are completed)
	//

	if (!_q) {
		if (!_n) logo();
		if (_s) {
			if (_bitcheck(status,REBOOT_STATUS_RENAME_PENDING)) _tcscat(state, _T(", RENAME_PENDING"));
			if (_bitcheck(status,REBOOT_STATUS_REBOOT_PENDING)) _tcscat(state, _T(", REBOOT_PENDING"));
			if (_bitcheck(status,REBOOT_STATUS_REBOOT_REQUIRED)) _tcscat(state, _T(", REBOOT_REQUIRED"));
			if (_bitcheck(status,REBOOT_STATUS_NOTIFICATION_ACTIVE)) {
				_tcscat(state, _T(", NOTIFICATION_ACTIVE"));
				notification = TRUE;
			}
			if (!status) _tcscat(state, _T(", CLEAN"));
			_ftprintf(stdout, _T("Is reboot needed : %s\n"), 
				(status > REBOOT_STATUS_RENAME_PENDING) ? _T("yes") :
				(status > 0) ? _T("optional") : _T("no"));
			_ftprintf(stdout, _T("Status           : %s\n"), &state[2]);

			if (status > REBOOT_STATUS_RENAME_PENDING) {
				HKEY hKey;
				if (SUCCEEDED(RegOpenKeyEx(HKEY_CURRENT_USER, _T("System\\CurrentControlSet\\Control\\NotifyDeviceReboot"), 0, KEY_READ, &hKey))) {
					check_drivers(hKey, _d);
					RegCloseKey(hKey);
				}
			}
		}
		if (status) {
			if (_bitcheck(status,REBOOT_STATUS_RENAME_PENDING)) {
				fprintf(stdout, "Pending files    : %d\n", pending_files(&files, 0));
				if (_f) {
					pending_files(&files, 1);
				}
				MemFree(files);
			}
		}
		if (notification) {
			_ftprintf(stdout, _T("\nWARNING: The Windows Update notification was detected!\n"));
			_ftprintf(stdout, _T("         Restart your device to install updates.\n"));
		}
	}

	if (_r && needed) {
		if (!_i || notification) {
			OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
			NewState = (TOKEN_PRIVILEGES *)MemAlloc(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES));
			NewState->PrivilegeCount = 1;
			if(!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &NewState->Privileges[0].Luid)) goto Error;
			NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if(!AdjustTokenPrivileges(hToken, FALSE, NewState, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) goto Error;
			CloseHandle(hToken);
			if(InitiateSystemShutdown(NULL, _T("Reboot"), 0, TRUE, TRUE)) goto Exit;
Error:
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
				MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), error, 255, NULL);
			_ftprintf(stdout, _T("Reboot error: %s\n"), error);
		} else {
			if (!_q) _ftprintf(stdout, _T("Waiting while update is running\n"));
		}
	}
Exit:
	MemFree(NewState);
	return status;
}
