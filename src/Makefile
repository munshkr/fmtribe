BIN := FMTRIBE.EXE
SOURCES := main.c seq.c base_ctl.c pe_ctl.c vga.c fm.c pbm.c font.c
OBJS := $(SOURCES:.c=.o)
CFLAGS := -g -Wall -std=gnu99 -fgnu89-inline

ZIP_FILES := $(BIN) INSTRS.DAT 8x10.PBM ..\README.md
TAG := 07
ZIP := FMTRIB$(TAG).ZIP

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJS)

zip: $(ZIP)

$(ZIP): $(ZIP_FILES)
	pkzip $(ZIP) $(ZIP_FILES)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $<
