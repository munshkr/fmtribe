#include "pe_vw.h"
#include "vga.h"

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

#define MAP__STEP_SQUARE_SIZE   5
#define MAP__TOP                120
#define MAP__HEIGHT             (CHANNELS * MAP__STEP_SQUARE_SIZE)
#define MAP__FRAME_WIDTH        (STEPS * MAP__STEP_SQUARE_SIZE)
#define MAP__COLOR              0x12
#define MAP__HI_COLOR           0x13

const uint8_t CHANNEL_COLORS[CHANNELS]   = { 0x68, 0x6c, 0x6f, 0x72, 0x74, 0x77, 0x7c, 0x08 };
const uint8_t CHANNEL_COLORS_B[CHANNELS] = { 0x20, 0x24, 0x27, 0x2a, 0x2c, 0x2f, 0x34, 0x1f };

static void render_pattern_map(const pe_vw_t* this);
static void render_board(const pe_vw_t* this);
static void render_channel_selector(const pe_vw_t* this);
static void render_hits(const pe_vw_t* this);


pe_vw_t pe_vw_new(const seq_t* seq, const font_t* font)
{
    return (pe_vw_t) {
        .seq  = seq,
        .font = font,

        .dirty = false,
    };
};

void pe_vw_render(pe_vw_t* this)
{
    render_pattern_map(this);
    render_hits(this);
    render_channel_selector(this);
    render_board(this);

    if (this->seq->recording) {
        font_render_strf(this->font, 284, 185, 7, "R");
    }
    if (this->seq->play_instruments) {
        font_render_strf(this->font, 292, 185, 7, "p");
    }
    if (this->seq->follow) {
        font_render_strf(this->font, 300, 185, 7, "f");
    }
    if (this->seq->apply_all_frames) {
        font_render_strf(this->font, 308, 185, 7, "*");
    }

    this->dirty = false;
}

static void render_pattern_map(const pe_vw_t* this)
{
    // highlight current frame block
    vga_rect_fill(MAP__FRAME_WIDTH * this->seq->current_selected_frame, MAP__TOP,
              MAP__FRAME_WIDTH * (this->seq->current_selected_frame + 1) - 1, MAP__TOP + MAP__HEIGHT - 1,
              MAP__COLOR);

    // highlight current frame with an underscore
    vga_rect_fill(MAP__FRAME_WIDTH * this->seq->current_frame, MAP__TOP + MAP__HEIGHT + 5,
              MAP__FRAME_WIDTH * (this->seq->current_frame + 1) - 1, MAP__TOP + MAP__HEIGHT + 7,
              MAP__COLOR);

    // step cursor
    unsigned int cursor_left = (this->seq->current_frame * MAP__FRAME_WIDTH) + (this->seq->current_step * MAP__STEP_SQUARE_SIZE);
    vga_rect_fill(cursor_left, 0, cursor_left + MAP__STEP_SQUARE_SIZE - 1, SCREEN_HEIGHT - 1, MAP__COLOR);
    // highlight frame block slice
    if (this->seq->current_selected_frame == this->seq->current_frame) {
        vga_rect_fill(cursor_left, MAP__TOP, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT - 1, MAP__HI_COLOR);
    } else {
        vga_rect_fill(cursor_left, MAP__TOP, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT - 1, MAP__COLOR);
    }
    // highlight frame underscore slice
    vga_rect_fill(cursor_left, MAP__TOP + MAP__HEIGHT + 5, cursor_left + MAP__STEP_SQUARE_SIZE - 1, MAP__TOP + MAP__HEIGHT + 7, MAP__HI_COLOR);

    // channel cursor
    unsigned int cursor_top = MAP__TOP + (this->seq->current_selected_channel * MAP__STEP_SQUARE_SIZE);
    vga_rect_fill(0, cursor_top, SCREEN_WIDTH - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1, MAP__COLOR);
    // highlight frame block slice
    vga_rect_fill(MAP__FRAME_WIDTH * this->seq->current_selected_frame, cursor_top,
              MAP__FRAME_WIDTH * (this->seq->current_selected_frame + 1) - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1,
              MAP__HI_COLOR);

    // highlight intersection of step cursor and channel cursor
    vga_rect_fill(cursor_left, cursor_top,
              cursor_left + MAP__STEP_SQUARE_SIZE - 1, cursor_top + MAP__STEP_SQUARE_SIZE - 1,
              MAP__HI_COLOR);

    // steps
    unsigned int step_top = MAP__TOP;
    for (int i = 0; i < CHANNELS; i++) {
        unsigned int color = CHANNEL_COLORS[i];
        unsigned int step_left = 0;
        for (int j = 0; j < FRAMES; j++) {
            for (int k = 0; k < STEPS; k++) {
                if (this->seq->seq[i][j][k]) {
                    // use highlighted color if cursor is over current frame+step
                    if (j == this->seq->current_frame && k == this->seq->current_step) {
                        color = CHANNEL_COLORS_B[i];
                    }

                    vga_rect_fill(step_left,
                              step_top,
                              step_left + MAP__STEP_SQUARE_SIZE - 1,
                              step_top + MAP__STEP_SQUARE_SIZE - 1,
                              color);

                    // restore color
                    if (j == this->seq->current_frame && k == this->seq->current_step) {
                        color = CHANNEL_COLORS[i];
                    }
                }
                step_left += MAP__STEP_SQUARE_SIZE;
            }
        }
        step_top += MAP__STEP_SQUARE_SIZE;
    }
}

static void render_board(const pe_vw_t* this)
{
    int color = CHANNEL_COLORS[this->seq->current_selected_channel];
    int top = BOARD_TOP;
    int z = 0;
    for (int i = 0; i < BOARD_ROWS; i++) {
        int left = BOARD_LEFT;
        for (int j = 0; j < BOARD_COLS; j++) {
            // if it is about to render the square for the current step,
            // use a different color.
            if (this->seq->current_frame == this->seq->current_selected_frame && z == this->seq->current_step) {
                color = CHANNEL_COLORS_B[this->seq->current_selected_channel];
            }

            // render a filled square if the step is toggled
            const unsigned int cur_step = (i * BOARD_COLS) + j;
            const unsigned int microsteps = this->seq->mseq[this->seq->current_selected_channel][this->seq->current_selected_frame][cur_step] + 1;
            const unsigned int width = (BOARD_SQUARE_SIZE - 3 * (microsteps - 1)) / microsteps;

            for (int k = 0; k < microsteps; k++) {
                unsigned int r_left = left + (width + 3) * k;
                unsigned int r_right = left + width * (k + 1) + 3 * k;
                if (k == microsteps - 1 && microsteps % 2 == 0) r_right++;
                if (this->seq->seq[this->seq->current_selected_channel][this->seq->current_selected_frame][cur_step]) {
                    vga_rect_fill(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                } else {
                    vga_rect(r_left, top, r_right, top + BOARD_SQUARE_SIZE, color);
                }
            }

            // restore color
            if (this->seq->current_frame == this->seq->current_selected_frame && z == this->seq->current_step) {
                color = CHANNEL_COLORS[this->seq->current_selected_channel];
            }

            left += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
            z++;
        }
        top += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
    }
}

static void render_channel_selector(const pe_vw_t* this)
{
    const int top = CHANNEL_SELECTOR_TOP;
    const int bottom = top + CHANNEL_SELECTOR_HEIGHT;
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    for (int i = 0; i < CHANNELS; i++) {
        if (this->seq->muted_channels[i]) {
            vga_rect(left, top, right, bottom, CHANNEL_COLORS[i]);
        } else {
            vga_rect_fill(left, top, right, bottom, CHANNEL_COLORS[i]);
        }

        if (i == this->seq->current_selected_channel) {
            vga_rect(left, top, right, bottom, CHANNEL_COLORS_B[i]);
        }

        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}

static void render_hits(const pe_vw_t* this)
{
    const int top = CHANNEL_SELECTOR_TOP - (CHANNEL_SELECTOR_HEIGHT / 2) - 3;
    const int bottom = top + (CHANNEL_SELECTOR_HEIGHT / 2);
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    for (int i = 0; i < CHANNELS; i++) {
        if (!this->seq->muted_channels[i] && this->seq->seq[i][this->seq->current_frame][this->seq->current_step]) {
            vga_rect_fill(left, top, right, bottom, CHANNEL_COLORS_B[i]);
        }
        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}
