CC=cl /nologo /c
LD=link /nologo
RM=del /Q

OBJ=is_reboot_needed.obj main.obj
APP=is_reboot_needed.exe

CFLAGS=/MT /O2 /W3 /GS /DNDEBUG /DUNICODE /D_UNICODE /Fe:$(APP)
LDFLAGS=/RELEASE /VERSION:0.3 /SUBSYSTEM:CONSOLE

all:	$(APP)

$(APP):	$(OBJ)
	$(LD) $(LDFLAGS) $(OBJ)

.c.obj::
	$(CC) $(CFLAGS) $<

clean:
	-$(RM) $(OBJ) > nul 2>&1
	-$(RM) $(APP) > nul 2>&1
