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

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

#define CHANNELS        8
#define STEPS           16
#define FRAMES          4
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

#define MAX_MICROSTEPS 3

const char* INSTRS_FILE = "INSTRS.DAT";
const char* FONT_FILE   = "8x10.PBM";

const int DEFAULT_BPM = 120;

const char STEP_KEYS[]       = "qwertyuiasdfghjk";
const char STEP_UPPER_KEYS[] = "QWERTYUIASDFGHJK";

const char KEYBOARD_KEYS[]       = "awsedftgyhuj";
const char KEYBOARD_UPPER_KEYS[] = "AWSEDFTGYHUJ";

const char CHANNEL_KEYS[] = "12345678";

const unsigned int MICROSTEP_KEYS[] = {
    K_Alt_Q, K_Alt_W, K_Alt_E, K_Alt_R, K_Alt_T, K_Alt_Y, K_Alt_U, K_Alt_I,
    K_Alt_A, K_Alt_S, K_Alt_D, K_Alt_F, K_Alt_G, K_Alt_H, K_Alt_J, K_Alt_K
};

const uint8_t CHANNEL_COLORS[CHANNELS]   = { 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
const uint8_t CHANNEL_COLORS_B[CHANNELS] = { 0x18, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

const note_t KEYBOARD_NOTES[] = { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B };

int current_selected_channel = 0;
int current_selected_frame = 0;

uclock_t current_usecs_per_step = 0;
float    current_bpm;
int      current_frame = 0;
int      current_step = 0;
uclock_t prev_tap = NULL;

bool dirty = true;
bool pause_after_current_step = false;
bool stop_after_current_bar = false;
bool playing = false;
bool metronome_on = false;
bool instrument_editor_enabled = false;

instr_t      instrs[CHANNELS] = {};
// TODO mseq and seq should be merged (1 microstep == 1 step...)
bool         seq[CHANNELS][FRAMES][STEPS] = {};
unsigned int mseq[CHANNELS][FRAMES][STEPS] = {};

int current_instr_field = 0;

pbm_file_t pbm;
font_t font;

extern fm_instr_t tick1;


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
        fread(instrs, sizeof(instr_t), CHANNELS, f);
        fclose(f);
    } else {
        fprintf(stderr, "Could not find %s. Resetting instrument parameters...\n", INSTRS_FILE);
        getch();

        for (int c = 0; c < CHANNELS; c++) {
            instrs[c].note = A;
            instrs[c].octave = 2;
            fm_instr_t* fi = &instrs[c].fm_instr;
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
        fm_set_instrument(c, &instrs[c].fm_instr);
    }
}

bool save_instruments()
{
    FILE* f = fopen(INSTRS_FILE, "wb");
    if (!f) {
        fprintf(stderr, "Could not write instruments parameters to %s.\n", INSTRS_FILE);
        return false;
    }
    fwrite(instrs, sizeof(instr_t), CHANNELS, f);
    fclose(f);
    printf("Instrument parameters were written to %s.\n", INSTRS_FILE);
    return true;
}

void tick()
{
    current_step++;
    if (current_step == STEPS) {
        current_step = 0;
        current_frame++;
        if (stop_after_current_bar) {
            playing = false;
            stop_after_current_bar = false;
        }
    }
    if (current_frame == FRAMES) {
        current_frame = 0;
    }
    dirty = true;
}

void play_channel(const unsigned int c)
{
    fm_key_off(c);
    fm_key_on(c, instrs[c].octave, instrs[c].note);
}

void play_step()
{
    if (metronome_on) {
        if (current_step == 0) {
            fm_key_off(METRONOME_CH);
            fm_key_on(METRONOME_CH, 4, E);
        } else if (current_step % 4 == 0) {
            fm_key_off(METRONOME_CH);
            fm_key_on(METRONOME_CH, 3, E);
        }
    }

    for (int c = 0; c < CHANNELS; c++) {
        if (seq[c][current_frame][current_step]) {
            play_channel(c);
        }
    }
}

void toggle_metronome()
{
    metronome_on = Not(metronome_on);
    if (metronome_on) {
        //printf("Metronome enabled ");
    } else {
        //printf("Metronome disabled ");
    }
}

void set_bpm(const float value)
{
    current_bpm = value;
    current_usecs_per_step = (USECS_PER_MINUTE / value) / 4;
}

void set_bpm_from_usecs_per_beat(uclock_t usecs)
{
    current_bpm = usecs * USECS_PER_MINUTE;
    current_usecs_per_step = usecs / 4;
}

void tap_tempo()
{
    uclock_t now = uclock();
    if (prev_tap) {
        set_bpm_from_usecs_per_beat(now - prev_tap);
    }
    prev_tap = now;
}

void clear_seq(int channel)
{
    memset(seq[channel][current_frame], 0, STEPS * sizeof(bool));
    memset(mseq[channel][current_frame], 0, STEPS * sizeof(unsigned int));
    dirty = true;
}

void clear_seq_all()
{
    memset(seq, 0, CHANNELS * FRAMES * STEPS * sizeof(bool));
    dirty = true;
}

void select_prev_channel() {
    if (current_selected_channel > 0) {
        current_selected_channel--;
        dirty = true;
    }
}

void select_next_channel() {
    if (current_selected_channel < CHANNELS - 1) {
        current_selected_channel++;
        dirty = true;
    }
}

void select_prev_frame() {
    if (current_selected_frame > 0) {
        current_selected_frame--;
        dirty = true;
    }
}

void select_next_frame() {
    if (current_selected_frame < FRAMES - 1) {
        current_selected_frame++;
        dirty = true;
    }
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
    } else if (dir == Down && current_instr_field < 9) {
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
        return fm_get_modulator_attack_rate(fm);
      case 6:
        return fm_get_modulator_decay_rate(fm);
      case 7:
        return fm_get_modulator_sustain_level(fm);
      case 8:
        return fm_get_modulator_release_rate(fm);
      case 9:
        return fm_get_modulator_waveform_type(fm);
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
        fm_set_modulator_attack_rate(fm, value);
        break;
      case 6:
        fm_set_modulator_decay_rate(fm, value);
        break;
      case 7:
        fm_set_modulator_sustain_level(fm, value);
        break;
      case 8:
        fm_set_modulator_release_rate(fm, value);
        break;
      case 9:
        fm_set_modulator_waveform_type(fm, value);
        break;
    }
}

typedef enum { Increase, Decrease } action_t;

void instrument_editor_change(const action_t action) {
    instr_t* ins = &instrs[current_selected_channel];
    unsigned int value = get_value_for_instrument(ins, current_instr_field);

    if (action == Increase &&
            (((current_instr_field == 4 || current_instr_field == 9) && value < 7) ||
             ((current_instr_field != 4 && current_instr_field != 9) && value < 15)))
    {
        value++;
        set_value_for_instrument(ins, current_instr_field, value);
        fm_set_instrument(current_selected_channel, &ins->fm_instr);
        dirty = true;
    } else if (action == Decrease && value > 0) {
        value--;
        set_value_for_instrument(ins, current_instr_field, value);
        fm_set_instrument(current_selected_channel, &ins->fm_instr);
        dirty = true;
    }
}

void render_board()
{
    int color = CHANNEL_COLORS[current_selected_channel];
    int top = BOARD_TOP;
    int z = 0;
    for (int i = 0; i < BOARD_ROWS; i++) {
        int left = BOARD_LEFT;
        for (int j = 0; j < BOARD_COLS; j++) {
            // if it is about to render the square for the current step,
            // use a different color.
            if (current_frame == current_selected_frame && z == current_step) {
                color = CHANNEL_COLORS_B[current_selected_channel];
            }

            // render a filled square if the step is toggled
            const unsigned int cur_step = (i * BOARD_COLS) + j;
            const unsigned int microsteps = mseq[current_selected_channel][current_selected_frame][cur_step] + 1;
            const unsigned int width = (BOARD_SQUARE_SIZE - 3 * (microsteps - 1)) / microsteps;

            for (int k = 0; k < microsteps; k++) {
                unsigned int r_left = left + (width + 3) * k;
                unsigned int r_right = left + width * (k + 1) + 3 * k;
                if (k == microsteps - 1 && microsteps % 2 == 0) r_right++;
                if (seq[current_selected_channel][current_selected_frame][cur_step]) {
                    rect_fill(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                } else {
                    rect(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                }
            }

            // restore color
            if (current_frame == current_selected_frame && z == current_step) {
                color = CHANNEL_COLORS[current_selected_channel];
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
        if (i == current_selected_channel) {
            rect_fill(left, top, right, bottom, CHANNEL_COLORS[i]);
        } else {
            rect(left, top, right, bottom, CHANNEL_COLORS[i]);
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
        if (seq[i][current_frame][current_step]) {
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

const unsigned int instr_fields_pos[10][2] = {
    { C_COL_LEFT + 120, C_COL_TOP + 20 },
    { C_COL_LEFT + 120, C_COL_TOP + 35 },
    { C_COL_LEFT + 120, C_COL_TOP + 50 },
    { C_COL_LEFT + 120, C_COL_TOP + 65 },
    { C_COL_LEFT + 120, C_COL_TOP + 80 },
    { M_COL_LEFT + 120, M_COL_TOP + 20 },
    { M_COL_LEFT + 120, M_COL_TOP + 35 },
    { M_COL_LEFT + 120, M_COL_TOP + 50 },
    { M_COL_LEFT + 120, M_COL_TOP + 65 },
    { M_COL_LEFT + 120, M_COL_TOP + 80 },
};

void render_instrument_editor()
{
    const instr_t* ins = &instrs[current_selected_channel];
    const fm_instr_t* fm = &ins->fm_instr;

    // Render labels
    render_str(&font, C_COL_LEFT, C_COL_TOP, 7, "Carrier");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 20, 7, "Attack Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 35, 7, "Decay Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 50, 7, "Sustain Level");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 65, 7, "Release Rate:");
    render_str(&font, C_COL_LEFT, C_COL_TOP + 80, 7, "Waveform:");
    render_str(&font, M_COL_LEFT, M_COL_TOP, 7, "Modulator");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 20, 7, "Attack Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 35, 7, "Decay Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 50, 7, "Sustain Level:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 65, 7, "Release Rate:");
    render_str(&font, M_COL_LEFT, M_COL_TOP + 80, 7, "Waveform:");

    // Render field values
    render_strf(&font, instr_fields_pos[0][0], instr_fields_pos[0][1], 7, "%X", (fm->c__attack_decay >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[1][0], instr_fields_pos[1][1], 7, "%X", fm->c__attack_decay & 0xf);
    render_strf(&font, instr_fields_pos[2][0], instr_fields_pos[2][1], 7, "%X", (fm->c__sustain_release >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[3][0], instr_fields_pos[3][1], 7, "%X", fm->c__sustain_release & 0xf);
    render_strf(&font, instr_fields_pos[4][0], instr_fields_pos[4][1], 7, "%X", fm_get_carrier_waveform_type(fm));

    render_strf(&font, instr_fields_pos[5][0], instr_fields_pos[5][1], 7, "%X", (fm->m__attack_decay >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[6][0], instr_fields_pos[6][1], 7, "%X", fm->m__attack_decay & 0xf);
    render_strf(&font, instr_fields_pos[7][0], instr_fields_pos[7][1], 7, "%X", (fm->m__sustain_release >> 4) & 0xf);
    render_strf(&font, instr_fields_pos[8][0], instr_fields_pos[8][1], 7, "%X", fm->m__sustain_release & 0xf);
    render_strf(&font, instr_fields_pos[9][0], instr_fields_pos[9][1], 7, "%X", fm_get_modulator_waveform_type(fm));

    // TODO draw "current field" rectangle
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
        render_hits();
        render_channel_selector();
        render_board();
    }

    //render_strf(&font, 6, 5, 7, "FMTribe v%i.%i", MAJOR_VERSION, MINOR_VERSION);
    render_strf(&font, 6, 185, 7, "f: %i, sf: %i", current_frame, current_selected_frame);

    update();
}


int main(int argc, char* argv[])
{
    load_font();

    fm_reset();
    fm_init();

    load_instruments();
    fm_set_instrument(METRONOME_CH, &tick1);

    if (argc == 1) {
        set_bpm(DEFAULT_BPM);
    } else {
        const unsigned int custom_bpm = atoi(argv[1]);
        set_bpm(custom_bpm);
        printf("BPM set to %i\n", custom_bpm);
        getch();
    }

    init_vga();
    set_mode(VIDEO_MODE);

    bool is_running = true;
    uclock_t prev = uclock();
    uclock_t mprev[CHANNELS];
    for (int c = 0; c < CHANNELS; c++) mprev[c] = prev;

    while (is_running) {
        if (kbhit()) {
            int key = getkey();

            switch (key) {
              case K_Escape:
                is_running = false;
                break;
              case K_F5:
                if (playing) {
                    pause_after_current_step = true;
                } else {
                    playing = true;
                    prev = uclock();
                    play_step();
                }
                break;
              case K_F7:
                if (playing) {
                    // on second F7, stop immediately, by pausing after current
                    // step and resetting current_step.
                    if (stop_after_current_bar) {
                        stop_after_current_bar = false;
                        pause_after_current_step = true;
                        current_step = 0;
                        current_frame = 0;
                    } else {
                        stop_after_current_bar = true;
                    }
                }
                break;
              case K_Shift_F9:
                toggle_metronome();
                break;
              case K_F9:
                tap_tempo();
                break;
              case K_Tab:
                switch_instrument_editor();
                break;
            }

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
                    if (instrs[current_selected_channel].octave > 1) {
                        instrs[current_selected_channel].octave--;
                        dirty = true;
                    }
                    break;
                  case K_PageUp:
                    if (instrs[current_selected_channel].octave < 8) {
                        instrs[current_selected_channel].octave++;
                        dirty = true;
                    }
                    break;
                }

                for (int i = 0; i < KEYBOARD_KEYS_COUNT; i++) {
                    if (key == KEYBOARD_KEYS[i] || key == KEYBOARD_UPPER_KEYS[i]) {
                        instrs[current_selected_channel].note = KEYBOARD_NOTES[i];
                        if (!playing) play_channel(current_selected_channel);
                    }
                }
            } else {
                switch (key) {
                  case '<':
                    select_prev_channel();
                    break;
                  case '>':
                    select_next_channel();
                    break;
                  case K_Left:
                    select_prev_frame();
                    break;
                  case K_Right:
                    select_next_frame();
                    break;
                  case K_Delete:
                    clear_seq(current_selected_channel);
                    break;
                  case K_Control_Delete:
                    clear_seq_all();
                    break;
                }

                for (int i = 0; i < STEPS; i++) {
                    if (key == STEP_KEYS[i] || key == STEP_UPPER_KEYS[i]) {
                        seq[current_selected_channel][current_selected_frame][i] =
                          Not(seq[current_selected_channel][current_selected_frame][i]);
                        dirty = true;
                    }
                }

                for (int i = 0; i < STEPS; i++) {
                    if (key == MICROSTEP_KEYS[i]) {
                        seq[current_selected_channel][current_selected_frame][i] = true;
                        mseq[current_selected_channel][current_selected_frame][i] =
                          (mseq[current_selected_channel][current_selected_frame][i] + 1) % MAX_MICROSTEPS;
                        dirty = true;
                    }
                }
            }

            for (int i = 0; i < CHANNELS; i++) {
                if (key == CHANNEL_KEYS[i]) {
                    current_selected_channel = i;
                    dirty = true;
                    break;
                }
            }
        }

        if (playing) {
            uclock_t now = uclock();

            // play step
            if (now >= prev + current_usecs_per_step) {
                if (pause_after_current_step) {
                    pause_after_current_step = false;
                    playing = false;
                    dirty = true;
                } else {
                    tick();
                    play_step();
                }
                prev = now;
                for (int c = 0; c < CHANNELS; c++) mprev[c] = prev;
            }

            // play microsteps (if any)
            for (int c = 0; c < CHANNELS; c++) {
                if (seq[c][current_frame][current_step]) {
                    if (now >= mprev[c] + (current_usecs_per_step / (mseq[c][current_frame][current_step] + 1))) {
                        play_channel(c);
                        mprev[c] = now;
                    }
                }
            }
        }

        if (dirty) {
            render();
            dirty = false;
        }
    }

    set_mode(TEXT_MODE);

    save_instruments();

    fm_reset();
    free_font(&font);
    return EXIT_SUCCESS;
}
