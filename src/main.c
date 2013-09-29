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

#include "seq.h"

#include "base_ctl.h"
#include "pe_ctl.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 8

#define METRONOME_CH    CHANNELS

#define KEYBOARD_KEYS_COUNT 12

#define BOARD_SQUARE_SIZE 30
#define BOARD_SQUARE_PADDING 5
#define BOARD_ROWS 2
#define BOARD_COLS 8

#define BOARD_WIDTH  ((BOARD_SQUARE_SIZE * BOARD_COLS) + (BOARD_SQUARE_PADDING * (BOARD_COLS - 1)))
#define BOARD_HEIGHT ((BOARD_SQUARE_SIZE * BOARD_ROWS) + (BOARD_SQUARE_PADDING * (BOARD_ROWS - 1)))
#define BOARD_SIZE   (BOARD_WIDTH * BOARD_HEIGHT)
#define BOARD_LEFT   ((SCREEN_WIDTH  / 2) - (BOARD_WIDTH  / 2))
#define BOARD_TOP    ((SCREEN_HEIGHT / 2) - (BOARD_HEIGHT / 2) - 30)

#define CHANNEL_SELECTOR_WIDTH  15
#define CHANNEL_SELECTOR_HEIGHT 6
#define CHANNEL_SELECTOR_TOP    (BOARD_TOP - CHANNEL_SELECTOR_HEIGHT - 5)
#define CHANNEL_SELECTOR_LEFT   BOARD_LEFT

const char* INSTRS_FILE  = "INSTRS.DAT";
const char* PATTERN_FILE = "PATTERN.DAT";
const char* FONT_FILE    = "8x10.PBM";

const unsigned int DEFAULT_BPM = 120;

const char KEYBOARD_KEYS[]       = "awsedftgyhuj";
const char KEYBOARD_UPPER_KEYS[] = "AWSEDFTGYHUJ";

const uint8_t CHANNEL_COLORS[CHANNELS]   = { 0x68, 0x6c, 0x6f, 0x72, 0x74, 0x77, 0x7c, 0x08 };
const uint8_t CHANNEL_COLORS_B[CHANNELS] = { 0x20, 0x24, 0x27, 0x2a, 0x2c, 0x2f, 0x34, 0x1f };

const note_t KEYBOARD_NOTES[] = { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B };

bool dirty = true; // instrument-editor dirty flag
bool instrument_editor_enabled = false;

int current_instr_field = 0;

pbm_file_t pbm;
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


void load_font()
{
    if (read_pbm_file(FONT_FILE, &pbm)) {
        printf("%s ~ %ix%i\n", FONT_FILE, pbm.width, pbm.height);

        bool res = create_font_from_pbm(&pbm, 12, &font);
        free_pbm(&pbm);

        if (res) {
            printf("Font loaded ~ %ix%i\n", font.width, font.height);
        } else {
            printf("Error loading font.\n");
            free_font(&font);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Error reading font.pbm\n");
        free_pbm(&pbm);
        exit(EXIT_FAILURE);
    }
}

void load_instruments()
{
    FILE* f = fopen(INSTRS_FILE, "rb");
    if (f) {
        fread(seq.instrs, sizeof(instr_t), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not find %s. Resetting instrument parameters...\n", INSTRS_FILE);
        getch();

        for (int c = 0; c < CHANNELS; c++) {
            seq.instrs[c].note = A;
            seq.instrs[c].octave = 2;
            fm_instr_t* fi = &seq.instrs[c].fm_instr;
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
        fm_set_instrument(c, &seq.instrs[c].fm_instr);
    }
}

bool save_instruments()
{
    FILE* f = fopen(INSTRS_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not write instruments parameters to %s.\n", INSTRS_FILE);
        return false;
    }
    fwrite(seq.instrs, sizeof(instr_t), CHANNELS, f);
    fclose(f);
    printf("Instrument parameters were written to %s.\n", INSTRS_FILE);
    return true;
}

void load_pattern()
{
    FILE* f = fopen(PATTERN_FILE, "rb");
    if (f) {
        unsigned int bpm = 0;
        fread(&bpm, sizeof(unsigned int), 1, f);
        if (bpm) {
            seq_set_bpm(&seq, bpm);
        } else {
            fprintf(stderr, "Invalid BPM value in %s\n", PATTERN_FILE);
        }

        fread(seq.seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
        fread(seq.mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
        fread(seq.muted_channels, sizeof(bool), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not load %s.\n", PATTERN_FILE);
    }
}

bool save_pattern()
{
    FILE* f = fopen(PATTERN_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not write pattern to %s.\n", PATTERN_FILE);
        return false;
    }

    fwrite(&(seq.current_bpm), sizeof(unsigned int), 1, f);
    fwrite(seq.seq, sizeof(bool), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq.mseq, sizeof(unsigned int), CHANNELS * FRAMES * STEPS, f);
    fwrite(seq.muted_channels, sizeof(bool), CHANNELS, f);

    fclose(f);
    printf("Pattern was written to %s.\n", PATTERN_FILE);
    return true;
}

void switch_instrument_editor() {
    instrument_editor_enabled = Not(instrument_editor_enabled);
    dirty = true;
}

typedef enum { Up, Down } direction_t;

void instrument_editor_move(const direction_t dir) {
    if (dir == Up && current_instr_field > 0) {
        current_instr_field--;
        dirty = true;
    } else if (dir == Down && current_instr_field < 11) {
        current_instr_field++;
        dirty = true;
    }
}

unsigned int get_value_for_instrument(const instr_t* ins, const unsigned int field) {
    const fm_instr_t* fm = &ins->fm_instr;
    switch (field) {
      case 0:
        return fm_get_carrier_attack_rate(fm);
      case 1:
        return fm_get_carrier_decay_rate(fm);
      case 2:
        return fm_get_carrier_sustain_level(fm);
      case 3:
        return fm_get_carrier_release_rate(fm);
      case 4:
        return fm_get_carrier_waveform_type(fm);
      case 5:
        return fm_get_carrier_level(fm);
      case 6:
        return fm_get_modulator_attack_rate(fm);
      case 7:
        return fm_get_modulator_decay_rate(fm);
      case 8:
        return fm_get_modulator_sustain_level(fm);
      case 9:
        return fm_get_modulator_release_rate(fm);
      case 10:
        return fm_get_modulator_waveform_type(fm);
      case 11:
        return fm_get_modulator_level(fm);
    }
    return 0;
}

void set_value_for_instrument(instr_t* ins, const unsigned int field, const unsigned int value) {
    fm_instr_t* fm = &ins->fm_instr;
    switch (field) {
      case 0:
        fm_set_carrier_attack_rate(fm, value);
        break;
      case 1:
        fm_set_carrier_decay_rate(fm, value);
        break;
      case 2:
        fm_set_carrier_sustain_level(fm, value);
        break;
      case 3:
        fm_set_carrier_release_rate(fm, value);
        break;
      case 4:
        fm_set_carrier_waveform_type(fm, value);
        break;
      case 5:
        fm_set_carrier_level(fm, value);
        break;
      case 6:
        fm_set_modulator_attack_rate(fm, value);
        break;
      case 7:
        fm_set_modulator_decay_rate(fm, value);
        break;
      case 8:
        fm_set_modulator_sustain_level(fm, value);
        break;
      case 9:
        fm_set_modulator_release_rate(fm, value);
        break;
      case 10:
        fm_set_modulator_waveform_type(fm, value);
        break;
      case 11:
        fm_set_modulator_level(fm, value);
        break;
    }
}

typedef enum { Increase, Decrease } action_t;

void instrument_editor_change(const action_t action) {
    instr_t* ins = &seq.instrs[seq.current_selected_channel];
    unsigned int value = get_value_for_instrument(ins, current_instr_field);

    if (action == Increase &&
            ( ((current_instr_field == 4 || current_instr_field == 10) && value < 7) ||
              ((current_instr_field == 5 || current_instr_field == 11) && value < 31) ||
              ((current_instr_field != 4 && current_instr_field != 5 &&
                current_instr_field != 10 && current_instr_field != 11) && value < 15)) )
    {
        value++;
        set_value_for_instrument(ins, current_instr_field, value);
        fm_set_instrument(seq.current_selected_channel, &ins->fm_instr);
        dirty = true;
    } else if (action == Decrease && value > 0) {
        value--;
        set_value_for_instrument(ins, current_instr_field, value);
        fm_set_instrument(seq.current_selected_channel, &ins->fm_instr);
        dirty = true;
    }
}

#define MAP__STEP_SQUARE_SIZE   5
#define MAP__TOP                120
#define MAP__HEIGHT             (CHANNELS * MAP__STEP_SQUARE_SIZE)
#define MAP__FRAME_WIDTH        (STEPS * MAP__STEP_SQUARE_SIZE)
#define MAP__COLOR              0x12
#define MAP__HI_COLOR           0x13

void render_pattern_map()
{
    // highlight current frame block
    rect_fill(MAP__FRAME_WIDTH * seq.current_selected_frame, MAP__TOP,
              MAP__FRAME_WIDTH * (seq.current_selected_frame + 1) - 1, MAP__TOP + MAP__HEIGHT - 1,
              MAP__COLOR);

    // highlight current frame with an underscore
    rect_fill(MAP__FRAME_WIDTH * seq.current_frame, MAP__TOP + MAP__HEIGHT + 5,
              MAP__FRAME_WIDTH * (seq.current_frame + 1) - 1, MAP__TOP + MAP__HEIGHT + 7,
              MAP__COLOR);

    // step cursor
    unsigned int cursor_left = (seq.current_frame * MAP__FRAME_WIDTH) + (seq.current_step * MAP__STEP_SQUARE_SIZE);
    rect_fill(cursor_left, 0, cursor_left + MAP__STEP_SQUARE_SIZE - 1, SCREEN_HEIGHT - 1, MAP__COLOR);
    // highlight frame block slice
    if (seq.current_selected_frame == seq.current_frame) {
        rect_fill(cursor_left, MAP__TOP, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT - 1, MAP__HI_COLOR);
    } else {
        rect_fill(cursor_left, MAP__TOP, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT - 1, MAP__COLOR);
    }
    // highlight frame underscore slice
    rect_fill(cursor_left, MAP__TOP + MAP__HEIGHT + 5, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT + 7, MAP__HI_COLOR);

    // channel cursor
    unsigned int cursor_top = MAP__TOP + (seq.current_selected_channel * MAP__STEP_SQUARE_SIZE);
    rect_fill(0, cursor_top, SCREEN_WIDTH - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1, MAP__COLOR);
    // highlight frame block slice
    rect_fill(MAP__FRAME_WIDTH * seq.current_selected_frame, cursor_top,
              MAP__FRAME_WIDTH * (seq.current_selected_frame + 1) - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1,
              MAP__HI_COLOR);

    // highlight intersection of step cursor and channel cursor
    rect_fill(cursor_left, cursor_top,
              cursor_left + MAP__STEP_SQUARE_SIZE - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1,
              MAP__HI_COLOR);

    // steps
    unsigned int step_top = MAP__TOP;
    for (int i = 0; i < CHANNELS; i++) {
        unsigned int color = CHANNEL_COLORS[i];
        unsigned int step_left = 0;
        for (int j = 0; j < FRAMES; j++) {
            for (int k = 0; k < STEPS; k++) {
                if (seq.seq[i][j][k]) {
                    // use highlighted color if cursor is over current frame+step
                    if (j == seq.current_frame && k == seq.current_step) {
                        color = CHANNEL_COLORS_B[i];
                    }

                    rect_fill(step_left,
                              step_top,
                              step_left + MAP__STEP_SQUARE_SIZE - 1,
                              step_top + MAP__STEP_SQUARE_SIZE - 1,
                              color);

                    // restore color
                    if (j == seq.current_frame && k == seq.current_step) {
                        color = CHANNEL_COLORS[i];
                    }
                }
                step_left += MAP__STEP_SQUARE_SIZE;
            }
        }
        step_top += MAP__STEP_SQUARE_SIZE;
    }
}

void render_board()
{
    int color = CHANNEL_COLORS[seq.current_selected_channel];
    int top = BOARD_TOP;
    int z = 0;
    for (int i = 0; i < BOARD_ROWS; i++) {
        int left = BOARD_LEFT;
        for (int j = 0; j < BOARD_COLS; j++) {
            // if it is about to render the square for the current step,
            // use a different color.
            if (seq.current_frame == seq.current_selected_frame && z == seq.current_step) {
                color = CHANNEL_COLORS_B[seq.current_selected_channel];
            }

            // render a filled square if the step is toggled
            const unsigned int cur_step = (i * BOARD_COLS) + j;
            const unsigned int microsteps = seq.mseq[seq.current_selected_channel][seq.current_selected_frame][cur_step] + 1;
            const unsigned int width = (BOARD_SQUARE_SIZE - 3 * (microsteps - 1)) / microsteps;

            for (int k = 0; k < microsteps; k++) {
                unsigned int r_left = left + (width + 3) * k;
                unsigned int r_right = left + width * (k + 1) + 3 * k;
                if (k == microsteps - 1 && microsteps % 2 == 0) r_right++;
                if (seq.seq[seq.current_selected_channel][seq.current_selected_frame][cur_step]) {
                    rect_fill(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                } else {
                    rect(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                }
            }

            // restore color
            if (seq.current_frame == seq.current_selected_frame && z == seq.current_step) {
                color = CHANNEL_COLORS[seq.current_selected_channel];
            }

            left += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
            z++;
        }
        top += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
    }
}

void render_channel_selector()
{
    const int top = CHANNEL_SELECTOR_TOP;
    const int bottom = top + CHANNEL_SELECTOR_HEIGHT;
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    for (int i = 0; i < CHANNELS; i++) {
        if (seq.muted_channels[i]) {
            rect(left, top, right, bottom, CHANNEL_COLORS[i]);
        } else {
            rect_fill(left, top, right, bottom, CHANNEL_COLORS[i]);
        }

        if (i == seq.current_selected_channel) {
            rect(left, top, right, bottom, CHANNEL_COLORS_B[i]);
        }

        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}

void render_hits()
{
    const int top = CHANNEL_SELECTOR_TOP - (CHANNEL_SELECTOR_HEIGHT / 2) - 3;
    const int bottom = top + (CHANNEL_SELECTOR_HEIGHT / 2);
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    for (int i = 0; i < CHANNELS; i++) {
        if (!seq.muted_channels[i] && seq.seq[i][seq.current_frame][seq.current_step]) {
            rect_fill(left, top, right, bottom, CHANNEL_COLORS_B[i]);
        }
        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}

#define C_COL_LEFT  10
#define M_COL_LEFT  150
#define C_COL_TOP   10
#define M_COL_TOP   C_COL_TOP

const unsigned int instr_fields_pos[12][2] = {
    { C_COL_LEFT + 120, C_COL_TOP + 20 },
    { C_COL_LEFT + 120, C_COL_TOP + 35 },
    { C_COL_LEFT + 120, C_COL_TOP + 50 },
    { C_COL_LEFT + 120, C_COL_TOP + 65 },
    { C_COL_LEFT + 120, C_COL_TOP + 80 },
    { C_COL_LEFT + 120, C_COL_TOP + 95 },
    { M_COL_LEFT + 120, M_COL_TOP + 20 },
    { M_COL_LEFT + 120, M_COL_TOP + 35 },
    { M_COL_LEFT + 120, M_COL_TOP + 50 },
    { M_COL_LEFT + 120, M_COL_TOP + 65 },
    { M_COL_LEFT + 120, M_COL_TOP + 80 },
    { M_COL_LEFT + 120, M_COL_TOP + 95 },
};

void render_instrument_editor()
{
    const instr_t* ins = &seq.instrs[seq.current_selected_channel];
    const fm_instr_t* fm = &ins->fm_instr;

    // Render labels
    render_str(&font, C_COL_LEFT, C_COL_TOP, 7, "Carrier");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 20, 7, "Attack Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 35, 7, "Decay Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 50, 7, "Sustain Level");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 65, 7, "Release Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 80, 7, "Waveform:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 95, 7, "Volume:");
    render_str(&font, M_COL_LEFT, M_COL_TOP, 7, "Modulator");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 20, 7, "Attack Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 35, 7, "Decay Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 50, 7, "Sustain Level:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 65, 7, "Release Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 80, 7, "Waveform:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 95, 7, "Volume:");

    // Render field values
    render_strf(&font, instr_fields_pos[0][0], instr_fields_pos[0][1], 7, "%X", (fm->c__attack_decay >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[1][0], instr_fields_pos[1][1], 7, "%X", fm->c__attack_decay & 0xf);
    render_strf(&font, instr_fields_pos[2][0], instr_fields_pos[2][1], 7, "%X", (fm->c__sustain_release >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[3][0], instr_fields_pos[3][1], 7, "%X", fm->c__sustain_release & 0xf);
    render_strf(&font, instr_fields_pos[4][0], instr_fields_pos[4][1], 7, "%X", fm_get_carrier_waveform_type(fm));
    render_strf(&font, instr_fields_pos[5][0], instr_fields_pos[5][1], 7, "%X", fm_get_carrier_level(fm));

    render_strf(&font, instr_fields_pos[6][0], instr_fields_pos[6][1], 7, "%X", (fm->m__attack_decay >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[7][0], instr_fields_pos[7][1], 7, "%X", fm->m__attack_decay & 0xf);
    render_strf(&font, instr_fields_pos[8][0], instr_fields_pos[8][1], 7, "%X", (fm->m__sustain_release >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[9][0], instr_fields_pos[9][1], 7, "%X", fm->m__sustain_release & 0xf);
    render_strf(&font, instr_fields_pos[10][0], instr_fields_pos[10][1], 7, "%X", fm_get_modulator_waveform_type(fm));
    render_strf(&font, instr_fields_pos[11][0], instr_fields_pos[11][1], 7, "%X", fm_get_modulator_level(fm));

    // draw "current field" rectangle
    rect(instr_fields_pos[current_instr_field][0] - 4,
         instr_fields_pos[current_instr_field][1] - 2,
         instr_fields_pos[current_instr_field][0] + 10,
         instr_fields_pos[current_instr_field][1] + 12,
         6);
}

void render()
{
    clear();

    if (instrument_editor_enabled) {
        render_instrument_editor();
    } else {
        render_pattern_map();
        render_hits();
        render_channel_selector();
        render_board();
        if (seq.recording) {
          render_strf(&font, 284, 185, 7, "R");
        }
        if (seq.play_instruments) {
          render_strf(&font, 292, 185, 7, "p");
        }
        if (seq.follow) {
          render_strf(&font, 300, 185, 7, "f");
        }
        if (seq.apply_all_frames) {
          render_strf(&font, 308, 185, 7, "*");
        }
    }

    //render_strf(&font, 6, 5, 7, "FMTribe v%i.%i", MAJOR_VERSION, MINOR_VERSION);
    //render_strf(&font, 6, 185, 7, "f: %i, sf: %i", current_frame, current_selected_frame);
    render_strf(&font, 6, 185, 7, "%u", seq.current_bpm);

    update();
}

int main(int argc, char* argv[])
{
    load_font();

    fm_reset();
    fm_init();

    fm_set_instrument(METRONOME_CH, &tick1);

    // models
    seq = seq_new();

    // views
    // ...

    // controllers
    base_ctl_t base_ctl = base_ctl_new(&seq);
    pe_ctl_t   pe_ctl   = pe_ctl_new(&seq);

    load_pattern();
    load_instruments();

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

    init_vga();
    set_mode(VIDEO_MODE);

    bool is_running = true;

    while (is_running) {
        if (kbhit()) {
            int key = getkey();

            switch (key) {
              case K_Escape:
                is_running = false;
                break;
              case K_Tab:
                switch_instrument_editor();
                break;
            }

            base_ctl_handle_keyboard(&base_ctl, key);

            if (instrument_editor_enabled) {
                switch (key) {
                  case K_Right:
                    instrument_editor_change(Increase);
                    break;
                  case K_Left:
                    instrument_editor_change(Decrease);
                    break;
                  case K_Up:
                    instrument_editor_move(Up);
                    break;
                  case K_Down:
                    instrument_editor_move(Down);
                    break;
                  case K_PageDown:
                    if (seq.instrs[seq.current_selected_channel].octave > 1) {
                        seq.instrs[seq.current_selected_channel].octave--;
                        dirty = true;
                    }
                    break;
                  case K_PageUp:
                    if (seq.instrs[seq.current_selected_channel].octave < 8) {
                        seq.instrs[seq.current_selected_channel].octave++;
                        dirty = true;
                    }
                    break;
                }

                for (int i = 0; i < KEYBOARD_KEYS_COUNT; i++) {
                    if (key == KEYBOARD_KEYS[i] || key == KEYBOARD_UPPER_KEYS[i]) {
                        seq.instrs[seq.current_selected_channel].note = KEYBOARD_NOTES[i];
                        if (!seq.playing) seq_play_channel(&seq, seq.current_selected_channel);
                    }
                }
            } else { // pattern editor mode
                pe_ctl_handle_keyboard(&pe_ctl, key);
            }
        }

        seq_tick(&seq);

        // render everything if something changed (dirty flag is set)
        if (seq.dirty || dirty) {
            render();
            seq.dirty = false;
            dirty = false;
        }
    }

    set_mode(TEXT_MODE);

    save_instruments();
    save_pattern();

    fm_reset();
    free_font(&font);
    return EXIT_SUCCESS;
}
