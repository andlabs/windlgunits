ifeq ($(MAKECMDGOALS),64)
	CC = x86_64-w64-mingw32-gcc
	RC = x86_64-w64-mingw32-windres
else
	CC = i686-w64-mingw32-gcc
	RC = i686-w64-mingw32-windres
endif
CFILES = main.c
RCFILES = mainwin.rc
HFILES = windlgunits.h winapi.h mainwin.h

OFILES = $(CFILES:.c=.o) $(RCFILES:.rc=.o)

all: $(OFILES)
	$(CC) -g -o windlgunits.exe $(OFILES) $(LDFLAGS) $(neededLDFLAGS)

clean:
	rm -f $(OFILES)

%.o: %.c $(HFILES)
	$(CC) -g -o $@ -c $< $(CFLAGS) $(neededCFLAGS)

%.o: %.rc $(HFILES)
	$(RC) $(RCFLAGS) $< $@

forceall:
	touch $(CFILES) $(RCFILES)

neededCFLAGS = --std=c99
neededLDFLAGS = \
	-luser32 -lkernel32 -lcomctl32 \
	-lcomdlg32
