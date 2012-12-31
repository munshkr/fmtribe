BIN := fmtribe.exe
SOURCES := main.c common.c vga.c fm.c pbm.c
OBJS := $(SOURCES:.c=.o)
CFLAGS := -Wall #-std=gnu99

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

fmtribe.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
