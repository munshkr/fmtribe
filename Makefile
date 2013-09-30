BIN := FMTRIBE.EXE
SOURCES := \
	src/main.c \
	src/seq.c \
	src/base_ctl.c \
	src/pe_ctl.c \
	src/ie_ctl.c \
	src/pe_vw.c \
	src/ie_vw.c \
	src/vga.c \
	src/fm.c \
	src/pbm.c \
	src/font.c
OBJS := $(SOURCES:.c=.o)
CFLAGS := -g -Wall -std=gnu99 -fgnu89-inline

ZIP_FILES := fonts/8x10.pbm INSTRS.DAT README.md $(BIN)
TAG := 08
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
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $@
