#include <stdint.h>
#include <conio.h>
#include <keys.h>
#include <string.h>
#include <dos.h>

#include "common.h"

#include "fm.h"
#include "vga.h"
#include "instr.h"
#include "font.h"

// models
#include "seq.h"

// views
#include "pe_vw.h"
#include "ie_vw.h"

// controllers
#include "base_ctl.h"
#include "pe_ctl.h"
#include "ie_ctl.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 8

#define METRONOME_CH    CHANNELS

const char* INSTRS_FILE  = "INSTRS.DAT";
const char* PATTERN_FILE = "PATTERN.DAT";
const char* FONT_FILE    = "FONTS/8x10.PBM";

const unsigned int DEFAULT_BPM = 120;

const note_t DEFAULT_NOTE = A;
const unsigned int DEFAULT_OCTAVE = 2;

bool instrument_editor_enabled = false;
bool quitting = false;

pbm_t pbm;
font_t font;

seq_t seq;

fm_instr_t tick1 = {
    .c__am_vib_eg = 0x05,
    .c__ksl_volume = 0x0b,
    .c__attack_decay = 0xf6,
    .c__sustain_release = 0x94,
    .c__waveform = 0x00,

    .m__am_vib_eg = 0x07,
    .m__ksl_volume = 0x40,
    .m__attack_decay = 0x09,
    .m__sustain_release = 0x53,
    .m__waveform = 0x00,

    .feedback_fm = 0x0e,
    .fine_tune = 0x00,
    .panning = Center,
    .voice_type = Melodic,
};


static void load_font();
static void load_instruments(seq_t*, const char* filename);
static bool save_instruments(seq_t*, const char* filename);
static void load_pattern(seq_t*, const char* filename);
static bool save_pattern(seq_t*, const char* filename);

int main(int argc, char* argv[])
{
    load_font();

    fm_reset();
    fm_init();

    fm_set_instrument(METRONOME_CH, &tick1);

    // models
    seq_t seq = seq_new();

    // views
    pe_vw_t pe_vw = pe_vw_new(&seq, &font);
    ie_vw_t ie_vw = ie_vw_new(&seq, &font);

    // controllers
    base_ctl_t base_ctl = base_ctl_new(&seq);
    pe_ctl_t   pe_ctl   = pe_ctl_new(&seq, &pe_vw);
    ie_ctl_t   ie_ctl   = ie_ctl_new(&seq, &ie_vw);

    load_pattern(&seq, PATTERN_FILE);
    load_instruments(&seq, INSTRS_FILE);

    if (argc == 2) {
        const unsigned int custom_bpm = atoi(argv[1]);
        seq_set_bpm(&seq, custom_bpm);
        printf("BPM set to %i\n", custom_bpm);
        getch();
    }

    if (!seq.current_bpm) {
        printf("No BPM, use default: %i\n", DEFAULT_BPM);
        getch();
        seq_set_bpm(&seq, DEFAULT_BPM);
    }

    vga_init();
    vga_set_mode(VIDEO_MODE);

    bool is_running = true;

    while (is_running) {
        if (kbhit()) {
            int key = getkey();

            switch (key) {
              case K_Escape:
                // if seq is not playing, exit immediately
                is_running = seq.playing;
                // toggle quitting state (Esc cancels confirmation dialog)
                quitting = !quitting;
                break;
              case K_Tab:
                quitting = false;
                instrument_editor_enabled = Not(instrument_editor_enabled);
                ie_vw.dirty = true;
                break;
            }

            if (quitting) {
                switch (key) {
                  case 'y':
                  case 'Y':
                    is_running = false;
                    break;
                  case 'n':
                  case 'N':
                    quitting = false;
                    break;
                }
            }

            base_ctl_handle_keyboard(&base_ctl, key);

            if (instrument_editor_enabled) {
                ie_ctl_handle_keyboard(&ie_ctl, key);
            } else {
                pe_ctl_handle_keyboard(&pe_ctl, key);
            }
        }

        seq_tick(&seq);

        // render everything if something changed (dirty flag is set)
        if (seq.dirty || pe_vw.dirty || ie_vw.dirty) {
            vga_clear();

            if (instrument_editor_enabled) {
                ie_vw_render(&ie_vw);
            } else {
                pe_vw_render(&pe_vw);
            }

            //font_render_strf(&font, 6, 5, 7, "FMTribe v%i.%i", MAJOR_VERSION, MINOR_VERSION);
            //font_render_strf(&font, 6, 185, 7, "f: %i, sf: %i", current_frame, current_selected_frame);
            font_render_strf(&font, 6, 185, 7, "%u", seq.current_bpm);

            if (quitting) {
                font_render_strf(&font, 40, 185, 7, "exit? (y/n/esc)");
            }

            seq.dirty = false;

            vga_update();
        }
    }

    vga_set_mode(TEXT_MODE);

    save_instruments(&seq, INSTRS_FILE);
    save_pattern(&seq, PATTERN_FILE);

    fm_reset();
    font_free(&font);

    return EXIT_SUCCESS;
}


static void load_font()
{
    if (pbm_read(FONT_FILE, &pbm)) {
        printf("%s ~ %ix%i\n", FONT_FILE, pbm.width, pbm.height);

        bool res = font_create_from_pbm(&pbm, 12, &font);
        pbm_free(&pbm);

        if (res) {
            printf("Font loaded ~ %ix%i\n", font.width, font.height);
        } else {
            printf("Error loading font.\n");
            font_free(&font);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Error reading %s\n", FONT_FILE);
        pbm_free(&pbm);
        exit(EXIT_FAILURE);
    }
}

static void load_instruments(seq_t* seq, const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (f) {
        fread(seq->instrs, sizeof(instr_t), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not find %s. Resetting instrument parameters...\n", filename);
        getch();

        for (int c = 0; c < CHANNELS; c++) {
            seq->instrs[c].note = DEFAULT_NOTE;
            seq->instrs[c].octave = DEFAULT_OCTAVE;
            fm_instr_t* fi = &seq->instrs[c].fm_instr;
            fi->c__am_vib_eg = 0x00;
            fi->m__am_vib_eg = 0x00;
            fi->c__ksl_volume = 0x00;
            fi->m__ksl_volume = 0x0b;
            fi->c__attack_decay = 0xd6;
            fi->m__attack_decay = 0xa8;
            fi->c__sustain_release = 0x4f;
            fi->m__sustain_release = 0x4c;
            fi->c__waveform = 0x00;
            fi->m__waveform = 0x00;
            fi->feedback_fm = 0x00;
            fi->fine_tune = 0x00;
            fi->panning = Center;
            fi->voice_type = Melodic;
        }
    }

    // Configure operators for all the instruments
    for (int c = 0; c < CHANNELS; c++) {
        fm_set_instrument(c, &seq->instrs[c].fm_instr);
    }
}

static bool save_instruments(seq_t* seq, const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not write instruments parameters to %s.\n", filename);
        return false;
    }
    fwrite(seq->instrs, sizeof(instr_t), CHANNELS, f);
    fclose(f);
    printf("Instrument parameters were written to %s.\n", filename);
    return true;
}

static void load_pattern(seq_t* seq, const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (f) {
        unsigned int bpm = 0;
        fread(&bpm, sizeof(unsigned int), 1, f);
        if (bpm) {
            seq_set_bpm(seq, bpm);
        } else {
            fprintf(stderr, "Invalid BPM value in %s\n", filename);
        }

        fread(seq->seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
        fread(seq->mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
        fread(seq->muted_channels, sizeof(bool), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not load %s.\n", filename);
    }
}

static bool save_pattern(seq_t* seq, const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not write pattern to %s.\n", filename);
        return false;
    }

    fwrite(&(seq->current_bpm), sizeof(unsigned int), 1, f);
    fwrite(seq->seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq->mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq->muted_channels, sizeof(bool), CHANNELS, f);

    fclose(f);
    printf("Pattern was written to %s.\n", filename);
    return true;
}
