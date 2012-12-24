BIN := fm.exe
SOURCES := main.c common.c sound.c
OBJS := $(SOURCES:.c=.o)
LIBS := -lpdcur~1

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

fm.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
