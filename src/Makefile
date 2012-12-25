BIN := fm.exe
SOURCES := main.c common.c vga.c sound.c
OBJS := $(SOURCES:.c=.o)

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

fm.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
