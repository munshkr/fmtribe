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

bool instrument_editor_enabled = false;

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
static void load_instruments(seq_t*);
static bool save_instruments(seq_t*);
static void load_pattern(seq_t*);
static bool save_pattern(seq_t*);


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

    load_pattern(&seq);
    load_instruments(&seq);

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
                is_running = false;
                break;
              case K_Tab:
                instrument_editor_enabled = Not(instrument_editor_enabled);
                ie_vw.dirty = true;
                break;
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

            //render_strf(&font, 6, 5, 7, "FMTribe v%i.%i", MAJOR_VERSION, MINOR_VERSION);
            //render_strf(&font, 6, 185, 7, "f: %i, sf: %i", current_frame, current_selected_frame);
            render_strf(&font, 6, 185, 7, "%u", seq.current_bpm);

            seq.dirty = false;

            vga_update();
        }
    }

    vga_set_mode(TEXT_MODE);

    save_instruments(&seq);
    save_pattern(&seq);

    fm_reset();
    free_font(&font);

    return EXIT_SUCCESS;
}


static void load_font()
{
    if (pbm_read(FONT_FILE, &pbm)) {
        printf("%s ~ %ix%i\n", FONT_FILE, pbm.width, pbm.height);

        bool res = create_font_from_pbm(&pbm, 12, &font);
        pbm_free(&pbm);

        if (res) {
            printf("Font loaded ~ %ix%i\n", font.width, font.height);
        } else {
            printf("Error loading font.\n");
            free_font(&font);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Error reading %s\n", FONT_FILE);
        pbm_free(&pbm);
        exit(EXIT_FAILURE);
    }
}

static void load_instruments(seq_t* seq)
{
    FILE* f = fopen(INSTRS_FILE, "rb");
    if (f) {
        fread(seq->instrs, sizeof(instr_t), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not find %s. Resetting instrument parameters...\n", INSTRS_FILE);
        getch();

        for (int c = 0; c < CHANNELS; c++) {
            seq->instrs[c].note = A;
            seq->instrs[c].octave = 2;
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

static bool save_instruments(seq_t* seq)
{
    FILE* f = fopen(INSTRS_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not write instruments parameters to %s.\n", INSTRS_FILE);
        return false;
    }
    fwrite(seq->instrs, sizeof(instr_t), CHANNELS, f);
    fclose(f);
    printf("Instrument parameters were written to %s.\n", INSTRS_FILE);
    return true;
}

static void load_pattern(seq_t* seq)
{
    FILE* f = fopen(PATTERN_FILE, "rb");
    if (f) {
        unsigned int bpm = 0;
        fread(&bpm, sizeof(unsigned int), 1, f);
        if (bpm) {
            seq_set_bpm(seq, bpm);
        } else {
            fprintf(stderr, "Invalid BPM value in %s\n", PATTERN_FILE);
        }

        fread(seq->seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
        fread(seq->mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
        fread(seq->muted_channels, sizeof(bool), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not load %s.\n", PATTERN_FILE);
    }
}

static bool save_pattern(seq_t* seq)
{
    FILE* f = fopen(PATTERN_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not write pattern to %s.\n", PATTERN_FILE);
        return false;
    }

    fwrite(&(seq->current_bpm), sizeof(unsigned int), 1, f);
    fwrite(seq->seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq->mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq->muted_channels, sizeof(bool), CHANNELS, f);

    fclose(f);
    printf("Pattern was written to %s.\n", PATTERN_FILE);
    return true;
}
