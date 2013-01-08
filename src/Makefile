BIN := fmtribe.exe
SOURCES := main.c vga.c fm.c pbm.c font.c
OBJS := $(SOURCES:.c=.o)
CFLAGS := -g -Wall -std=gnu99 -fgnu89-inline

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

fmtribe.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
