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

#ifdef _MSC_VER
#pragma comment(lib, "advapi32")
#endif

#include "is_reboot_needed.h"

// Reboot required
const TCHAR * __regkey_reboot_in_progress = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootInProgress");
const TCHAR * __regkey_reboot_required = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired");
const TCHAR * __regkey_reboot_attempts = _T("SOFTWARE\\Microsoft\\ServerManager\\CurrentRebootAttempts");

const TCHAR * __regkey_reboot_netlogon = _T("SYSTEM\\CurrentControlSet\\Services\\Netlogon");
const TCHAR * __regparam_reboot_netlogon1 = _T("JoinDomain");
const TCHAR * __regparam_reboot_netlogon2 = _T("AvoidSpnSet");

// !!!
// HKLM:\SYSTEM\CurrentControlSet\Control\ComputerName\ActiveComputerName &&
// HKLM:\SYSTEM\CurrentControlSet\Control\ComputerName\ComputerName is different

// Reboot pending
const TCHAR * __regkey_reboot_pending = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootPending");
const TCHAR * __regkey_reboot_packages_pending = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\PackagesPending");

// Rename pending
const TCHAR * __regkey_rename_pending = _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager");
const TCHAR * __regparam_rename_pending = _T("PendingFileRenameOperations");
const TCHAR * __regparam_rename_pending2 = _T("PendingFileRenameOperations2");

const TCHAR * __regkey_dvd_reboot_signal = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
const TCHAR * __regparam_dvd_reboot_signal = _T("DVDRebootSignal");

#if defined(_WIN64) || defined(WIN64) || defined(__x86_64__)
#define is_win64() TRUE
#else
typedef BOOL(WINAPI *fn_IsWow64Process)(HANDLE, PBOOL);

__declspec(noinline)
static int __cdecl is_win64() {
	fn_IsWow64Process isWow64Process;
	int result;
	HMODULE hModule = GetModuleHandle(_T("kernel32.dll"));
	if (hModule) {
		isWow64Process = (fn_IsWow64Process)GetProcAddress(hModule, "IsWow64Process");
		if (isWow64Process && isWow64Process(GetCurrentProcess(), &result)) return result;
	}
	return 0;
}
#endif

__declspec(noinline)
static int __cdecl check_reg_key(const TCHAR * key) {
	HKEY hKey = NULL;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ | (is_win64() ? KEY_WOW64_64KEY : 0), &hKey);
	if (hKey) {
		RegCloseKey(hKey);
		return TRUE;
	}
	return FALSE;
}

__declspec(noinline)
static int __cdecl check_reg_param(const TCHAR * key, const TCHAR * param, LPBYTE * value) {
	HKEY hKey = NULL;
	DWORD dwType = 0;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey);
	if (hKey) {
		DWORD dwSize = 0;
		if(SUCCEEDED(RegQueryValueEx(hKey, param, NULL, &dwType, NULL, &dwSize))) {
			if(value && dwSize) {
				*value = MemAlloc(dwSize);
				if(*value) RegQueryValueEx(hKey, param, NULL, &dwType, *value, &dwSize);
			}
		}
		RegCloseKey(hKey);
	}
	return (dwType == REG_MULTI_SZ);
}

__declspec(noinline)
int __cdecl is_reboot_needed(int * status) {
	return is_reboot_needed_ex(status, NULL);
}

__declspec(noinline)
int __cdecl is_reboot_needed_ex(int * status, LPBYTE * files) {
	int i, *dummy = status ? status : &i;
	*dummy = REBOOT_STATUS_CLEAN;

	if (
		check_reg_key(__regkey_reboot_required) ||
		check_reg_key(__regkey_reboot_attempts) ||
		check_reg_key(__regkey_reboot_in_progress) ||
		check_reg_param(__regkey_reboot_netlogon, __regparam_reboot_netlogon1, NULL) ||
		check_reg_param(__regkey_reboot_netlogon, __regparam_reboot_netlogon2, NULL)
	) *dummy |= REBOOT_STATUS_REBOOT_REQUIRED;

	if (
		check_reg_key(__regkey_reboot_pending) ||
		check_reg_key(__regkey_reboot_packages_pending) ||
		check_reg_param(__regkey_dvd_reboot_signal, __regparam_dvd_reboot_signal, NULL)
	) *dummy |= REBOOT_STATUS_REBOOT_PENDING;

	if (
		check_reg_param(__regkey_rename_pending, __regparam_rename_pending, files) ||
		check_reg_param(__regkey_rename_pending, __regparam_rename_pending2, NULL)
	) *dummy |= REBOOT_STATUS_RENAME_PENDING;

	return *dummy > REBOOT_STATUS_RENAME_PENDING;
}
