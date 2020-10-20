# Checks if you need to restart Windows

There are three types of Windows reboot events:
1. Rename pending
2. Reboot pending
3. Reboot required

Associated status codes with these events are defined in is_reboot_needed.h:

```C
static const int REBOOT_STATUS_CLEAN = 0;
static const int REBOOT_STATUS_RENAME_PENDING = 1;
static const int REBOOT_STATUS_REBOOT_PENDING = 2;
static const int REBOOT_STATUS_REBOOT_REQUIRED = 3;
```

This utility determines which event has occurred and returns it's associated return code. When ___rename pending___ is occured then will returns `REBOOT_STATUS_RENAME_PENDING` value.
When ___reboot pending___ is occured then will returns `REBOOT_STATUS_REBOOT_PENDING` value. If ___reboot is required___ then will returns `REBOOT_STATUS_REBOOT_REQUIRED` value.

When system is not reboot needed this utility returns `REBOOT_STATUS_CLEAN` value.

All of these values are returned through the passed pointer to an `int` variable as the parameter of the `is_reboot_needed` function. The returned `boolean` value is the result of checking whether a reboot is required.

### Compile with GCC
```
gcc -O2 -Wall -s -static -ladvapi32 -o is_reboot_needed.exe is_reboot_needed.c main.c
```

### Compile with MSVC
```
cl /MT /O2 /W3 is_reboot_needed.c main.c
```
