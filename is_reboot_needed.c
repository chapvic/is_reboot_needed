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

#include <tchar.h>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32")
#endif

#include "is_reboot_needed.h"

const TCHAR* val_rename_pending = _T("PendingFileRenameOperations");
const TCHAR* key_rename_pending = _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager");
const TCHAR* key_reboot_pending = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootPending");
const TCHAR* key_reboot_required = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired");

typedef BOOL(WINAPI *fn_IsWow64Process)(HANDLE, PBOOL);

static int is_win64() {
#if defined(_WIN64) || defined(WIN64)
	return 1;
#else
	fn_IsWow64Process isWow64Process;
	int result;
	HMODULE hModule = GetModuleHandle(_T("kernel32.dll"));
	if (hModule) {
		isWow64Process = (fn_IsWow64Process)GetProcAddress(hModule, "IsWow64Process");
		if (isWow64Process && isWow64Process(GetCurrentProcess(), &result)) return result;
	}
	return 0;
#endif
}

static BOOL check_regkey(const TCHAR * key) {
	HKEY hKey = NULL;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ | (is_win64() ? KEY_WOW64_64KEY : 0), &hKey);
	if (hKey) {
		RegCloseKey(hKey);
		return TRUE;
	}
	return FALSE;
}

static BOOL check_regval(const TCHAR * key, const TCHAR * val) {
	HKEY hKey = NULL;
	DWORD dwType = 0;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey);
	if (hKey) {
		RegQueryValueEx(hKey, val, NULL, &dwType, NULL, NULL);
		RegCloseKey(hKey);
	}
	return (dwType == REG_MULTI_SZ);
}

int is_reboot_needed(int* status) {
	int i, *dummy = status ? status : &i;
	if (check_regkey(key_reboot_required)) *dummy = REBOOT_STATUS_REBOOT_REQUIRED;
	else if (check_regkey(key_reboot_pending)) *dummy = REBOOT_STATUS_REBOOT_PENDING;
	else if (check_regval(key_rename_pending, val_rename_pending)) *dummy = REBOOT_STATUS_RENAME_PENDING;
	else *dummy = REBOOT_STATUS_CLEAN;
	return *dummy > REBOOT_STATUS_CLEAN;
}
