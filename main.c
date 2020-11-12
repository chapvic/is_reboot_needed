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

#undef _tfprintf

#if defined(_UNICODE) || defined(UNICODE)
#define _tfprintf fwprintf
#else
#define _tfprintf fprintf
#endif

static char * __cdecl basename(char * path) {
    char* p = strchr(path, 0);
    while (p > path && !(p[-1] == '/' || p[-1] == '\\')) p--;
    return p;
}

static void __cdecl logo(void) {
	fprintf(stdout, "Check if reboot is needed, version 0.2.5b\n");
	fprintf(stdout, "Copyright (c) 2019-2020, FoxTeam\n\n");
};

__declspec(noinline)
static void __cdecl usage(char *cmd) {
	logo();
	fprintf(stdout, "Usage: %s [options]\n", basename(cmd));
	fprintf(stdout, "    -h      this help\n");
	fprintf(stdout, "    -a      combination of -s, -d, -f\n");
	fprintf(stdout, "    -s      show status information (default)\n");
	fprintf(stdout, "    -d      list driver updates\n");
	fprintf(stdout, "    -f      list update pending files\n");
	fprintf(stdout, "    -n      suppress logo\n");
	fprintf(stdout, "    -q      don't show any messages (quiet mode)\n");
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
		if (_print) _tfprintf(stdout, _T("  (%d) %s\n"), ++cnt, p+4);
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
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
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
        fprintf(stdout, "Driver updates   : %ld\n", cValues);

    if (cValues) {
	// Get registry timestamp
	SYSTEMTIME st;
	FileTimeToSystemTime(&ftLastWriteTime, &st);
        fprintf(stdout, "Last update time : %02d.%02d.%04d %02d:%02d:%02d\n",
		st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
	
	if (!detailed) return retval;
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);
            if (retCode == ERROR_SUCCESS) { 
                _tprintf(TEXT("  (%ld) "), i+1);

		HKEY drvKey, clsKey;
		_tcscpy(achKey, TEXT("SYSTEM\\CurrentControlSet\\Enum\\"));
		_tcscat(achKey, achValue);
		if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, achKey, 0, KEY_READ, &drvKey))) {
			achValue[0] = '\0';
			cchValue = MAX_VALUE_NAME;
			if (SUCCEEDED(RegQueryValueEx(drvKey, TEXT("Driver"), NULL, NULL, (LPBYTE)&achValue, &cchValue)) && achValue[0]) {
				// Load driver information
				_tcscpy(achKey, TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\"));
				_tcscat(achKey, achValue);
				if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, achKey, 0, KEY_READ, &clsKey))) {
					// Get provider name
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, TEXT("ProviderName"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_tfprintf(stdout, TEXT(" %s"), achValue);
					// Get driver decription
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, TEXT("DriverDesc"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_tfprintf(stdout, TEXT(" - %s"), achValue);
					// Get driver version
					achValue[0] = '\0';
					cchValue = MAX_VALUE_NAME;
					RegQueryValueEx(clsKey, TEXT("DriverVersion"), NULL, NULL, (LPBYTE)&achValue, &cchValue);
					_tfprintf(stdout, TEXT(" - %s\n"), achValue);
				}
			}
			RegCloseKey(drvKey);
			_tfprintf(stdout, TEXT("---\n"));
		}
            } 
        }
    }
	return retval;
}

int main(int argc, char* argv[]) {
	LPBYTE files = NULL;
	int status = 0, argn = 0;
	int _h = 0, _s = 1, _d = 0, _f = 0, _n = 0, _q = 0;
	char state[100] = {0};
	if (argc > 1) {
		while (++argn < argc) {
			if (!_stricmp(argv[argn], "-h")) _h = 1;
			else if (!_stricmp(argv[argn], "-a")) { _s = 1; _d = 1, _f = 1; }
			else if (!_stricmp(argv[argn], "-s")) _s = 1;
			else if (!_stricmp(argv[argn], "-d")) _d = 1;
			else if (!_stricmp(argv[argn], "-f")) _f = 1;
			else if (!_stricmp(argv[argn], "-n")) _n = 1;
			else if (!_stricmp(argv[argn], "-q")) _q = 1;
			else _h = 1;
		}
	}
	if (_h) {
		usage(argv[0]);
		goto Exit;
	}
	is_reboot_needed_ex(&status, &files);
	if (!_q) {
		if (!_n) logo();
		if (_s) {
			if (_bitcheck(status,REBOOT_STATUS_RENAME_PENDING)) strcat(state, ", RENAME_PENDING");
			if (_bitcheck(status,REBOOT_STATUS_REBOOT_PENDING)) strcat(state, ", REBOOT_PENDING");
			if (_bitcheck(status,REBOOT_STATUS_REBOOT_REQUIRED)) strcat(state, ", REBOOT_REQUIRED");
			if (!status) strcat(state, ", CLEAN");
			fprintf(stdout, "Is reboot needed : %s\n", 
				(status > REBOOT_STATUS_RENAME_PENDING) ? "yes" :
				(status > 0) ? "optional" : "no");
			fprintf(stdout, "Status           : %s\n", &state[2]);

			if (status > REBOOT_STATUS_RENAME_PENDING) {
				HKEY hKey;
				if (SUCCEEDED(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("System\\CurrentControlSet\\Control\\NotifyDeviceReboot"), 0, KEY_READ, &hKey))) {
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
	}
Exit:
	return status;
}
