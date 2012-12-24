BIN := test.exe
SOURCES := test.c
OBJS := $(SOURCES:.c=.o)
#LIBS := -lcurses

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

test.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
