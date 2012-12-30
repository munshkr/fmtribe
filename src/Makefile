BIN := fmtribe.exe
SOURCES := main.c common.c vga.c fm.c
OBJS := $(SOURCES:.c=.o)

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

fmtribe.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
