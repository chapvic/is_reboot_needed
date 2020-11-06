# Checks if you need to restart Windows

Version: 0.2.2b

There are three types of Windows reboot events:
1. Rename pending
2. Reboot pending
3. Reboot required

Status codes of these events are defined in `is_reboot_needed.h`:

```C
#define REBOOT_STATUS_CLEAN            0UL
#define REBOOT_STATUS_RENAME_PENDING   1UL
#define REBOOT_STATUS_REBOOT_PENDING   2UL
#define REBOOT_STATUS_REBOOT_REQUIRED  4UL
```

Return code is a set of these flags.

When system is not reboot needed this utility returns `REBOOT_STATUS_CLEAN` (zero value).

Set of these flags returns through the passed pointer to an `int` variable as the parameter of the `is_reboot_needed` function. That returns `boolean` value is the result of check if reboot is required.

### Compile with GCC
```
gcc -O2 -Wall -s -static -ladvapi32 -o is_reboot_needed.exe is_reboot_needed.c main.c
```

### Compile with MSVC
```
cl /MT /O2 /W3 is_reboot_needed.c main.c
```
