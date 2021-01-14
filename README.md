# Checks if you need to restart Windows

Version: 0.3.0

This utility returns a set of reboot event codes:
- Rename pending (1)
- Reboot pending (2)
- Reboot required (4)

Status codes of these events are defined in `is_reboot_needed.h`:

```C
#define REBOOT_STATUS_CLEAN                0UL
#define REBOOT_STATUS_RENAME_PENDING       1UL
#define REBOOT_STATUS_REBOOT_PENDING       2UL
#define REBOOT_STATUS_REBOOT_REQUIRED      4UL
#define REBOOT_STATUS_NOTIFICATION_ACTIVE  8UL  /* a reboot notification is active (Windows 10) */
```

When system is not reboot needed this utility returns `REBOOT_STATUS_CLEAN` (zero value).

Set of these flags returns through the passed pointer to an `int` variable as the parameter of the `is_reboot_needed` function. That returns `boolean` value is the result of check if reboot is required.

### Usage

```
is_reboot_needed [options]

    -a      Combination of '-s', '-d' and '-f' options.
    -s      Show status information. This is default option.
    -d      List driver updates, if present.
    -f      List update pending files.
    -n      Suppress logo.
    -q      Suppress all messages.
    -r      Automatic reboot if needed with no messages.
    -i      Start reboot if notification is active only (Windows 10).
            (implies `-r` option)
```

Warning: If you're use `-r` option and `is reboot needed` is `TRUE`, then system will be restarted immediately.
Option `-i` makes reboot if process `MusNotifyIcon.exe` is active (for Windows 10)

Note: Since version 0.3.0 a special warning will be displayed when detects a reboot notification. The `REBOOT_STATUS_NOTIFICATION_ACTIVE` (8) flag has been added for the return code.
This is only for Windows 10!
                                                       
### Compile with GCC
```
gcc -O2 -Wall -s -static -ladvapi32 -o is_reboot_needed.exe is_reboot_needed.c main.c
```

### Compile with MSVC
```
cl /MT /O2 /W3 is_reboot_needed.c main.c
```
