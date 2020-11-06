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

#undef _tfprintf
#undef _tstrchr

#if defined(_UNICODE) || defined(UNICODE)
#define _tfprintf fwprintf
#define _tstrchr wcschr
#else
#define _tfprintf fprintf
#define _tstrchr strchr
#endif

static char* basename(char* path) {
    char* p = strchr(path, 0);
    while (p > path && !(p[-1] == '/' || p[-1] == '\\')) p--;
    return p;
}

static void usage(char *cmd) {
	fprintf(stdout, "Check if reboot is needed, v0.2.1-beta\n");
	fprintf(stdout, "Copyright (c) 2019-2020, FoxTeam\n\n");
	fprintf(stdout, "Usage: %s [options]\n", basename(cmd));
	fprintf(stdout, "    -h      this help\n");
	fprintf(stdout, "    -a      combination of '-s' and '-f'\n");
	fprintf(stdout, "    -s      show reboot status information\n");
	fprintf(stdout, "    -f      list rename pending files\n");
	fprintf(stdout, "    -q      do not show messages (quiet mode)\n");
	exit(0);
}

int main(int argc, char* argv[]) {
	LPBYTE files = NULL;
	TCHAR *p, *pp;
	int status = 0, result = 0, argn = 0, cnt = 0;
	int _h = 0, _q = 0, _s = 0, _f = 0;
	char state[100] = {0};
	if (argc > 1) {
		while (++argn < argc) {
			if (!_stricmp(argv[argn], "-h")) _h = 1;
			else if (!_stricmp(argv[argn], "-a")) { _s = 1; _f = 1; }
			else if (!_stricmp(argv[argn], "-s")) _s = 1;
			else if (!_stricmp(argv[argn], "-f")) _f = 1;
			else if (!_stricmp(argv[argn], "-q")) _q = 1;
			else _h = 1;
		}
	} else {
		_h = 1;
	}
	if (_h) {
		usage(argv[0]);
		goto Exit;
	}
	result = is_reboot_needed_ex(&status, &files);
	if (!_q) {
		if (_s) {
			strcat(state, _bitcheck(status,REBOOT_STATUS_RENAME_PENDING) ? ", RENAME_PENDING" : "");
			strcat(state, _bitcheck(status,REBOOT_STATUS_REBOOT_PENDING) ? ", REBOOT_PENDING" : "");
			strcat(state, _bitcheck(status,REBOOT_STATUS_REBOOT_REQUIRED) ? ", REBOOT_REQUIRED" : "");
			strcat(state, status == 0 ? ", CLEAN" : "");
			fprintf(stdout, "Is reboot needed : %s\n", result ? "yes" : "no");
			fprintf(stdout, "Status           : %s\n", &state[2]);
		}
		if (_f) {
			if (files) {
				fputs("\nPending files:\n", stdout);
				p = (TCHAR *)files;
				do {
					pp = _tstrchr(p, 0);
					_tfprintf(stdout, _T("%03d: %s\n"), ++cnt, p+4);
				} while(*(p = pp + 2));
				MemFree(files);
			} else {
				fputs("\nNo rename pending files.\n", stdout);
			}
		}
	}
Exit:
	return status;
}
