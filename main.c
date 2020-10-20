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

#include "is_reboot_needed.h"
#include <stdio.h>

static char* basename(char* path) {
    char* p = strchr(path, 0);
    while (p > path && !(p[-1] == '/' || p[-1] == '\\')) p--;
    return p;
}

static void usage(char *exe) {
	fprintf(stdout, "Check system reboot is needed, v0.2\n");
	fprintf(stdout, "Copyright (c) 2019-2020, FoxTeam\n\n");
	fprintf(stdout, "Usage: %s [options]\n", basename(exe));
	fprintf(stdout, "    -h      this help\n");
	fprintf(stdout, "    -q      do not show messages (quiet)\n");
	exit(0);
}

int main(int argc, char* argv[]) {
	int status = 0, result = 0, quiet = 0;
	if (argc == 2 && !_stricmp(argv[1], "-h")) usage(argv[0]);
	if (argc == 2 && !_stricmp(argv[1], "-q")) quiet = 1;
	if (argc > 1 && !quiet) usage(argv[0]);
	result = is_reboot_needed(&status);
	if (!quiet) {
		fprintf(stdout, "Is reboot needed : %s\n", result ? "yes" : "no");
		fprintf(stdout, "Status           : %s\n",
			status == REBOOT_STATUS_REBOOT_REQUIRED ? "reboot required" :
			status == REBOOT_STATUS_REBOOT_PENDING ? "reboot pending" :
			status == REBOOT_STATUS_RENAME_PENDING ? "rename pending" :
			"clean");
	}
	return status;
}
