/* Persona 1 (JP) - the debug page's switch board.  BTLP only.
 *   0x800A7B68 BtlDebugEditFlags
 *
 * Row 10 of g_btl_debug_actions. The main executable keeps thirty-two
 * one-byte switches for debugging the battle at g_btl_debug_flags - the few
 * the battle reads are named in battle.h - and this is the page that flips
 * them.
 *
 * They are laid out eight across and four down, and the cursor wraps round
 * both ways. A circle flips the switch under the cursor and a cross leaves.
 * Every frame the board is painted again from the switches: each cell's
 * palette says whether it is on, the one under the cursor blinks, the mark
 * beside each is the on or the off glyph, and the line along the bottom names
 * the switch under the cursor. It answers zero, like every row of that page.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>
#include <persona/btlp/text.h>

#define PAD_UP     0x1000
#define PAD_DOWN   0x4000
#define PAD_CIRCLE 0x20
#define PAD_CROSS  0x40

/* Thirty-two switches, eight to a row. */
#define FLAG_COUNT 32
#define FLAG_COLS  8
#define FLAG_TAIL  (FLAG_COUNT - FLAG_COLS)

/* The palettes a cell is drawn with, and the frame bit the cursor blinks on. */
#define FLAG_CLUT_ON    0x20
#define FLAG_CLUT_BLINK 0x21
#define FLAG_CLUT_OFF   0x23
#define FLAG_BLINK      4

/* The mark beside each switch. */
#define FLAG_MARK_OFF 0xE5
#define FLAG_MARK_ON  0xE6

/* Where the name of the switch under the cursor goes. */
#define FLAG_LINE_X 0x10
#define FLAG_LINE_Y 0x94

/* Which switch the cursor is on, kept between visits to the page. */
extern u_char g_btl_flag_row;

/* The name of each switch that has one, and the line every other switch
   shares. */
extern u_char g_btl_flag_name00[];
extern u_char g_btl_flag_name01[];
extern u_char g_btl_flag_name02[];
extern u_char g_btl_flag_name03[];
extern u_char g_btl_flag_name04[];
extern u_char g_btl_flag_name05[];
extern u_char g_btl_flag_name06[];
extern u_char g_btl_flag_name07[];
extern u_char g_btl_flag_name08[];
extern u_char g_btl_flag_name09[];
extern u_char g_btl_flag_name10[];
extern u_char g_btl_flag_name11[];
extern u_char g_btl_flag_name12[];
extern u_char g_btl_flag_name13[];
extern u_char g_btl_flag_name14[];
extern u_char g_btl_flag_name_unused[];

/* The line along the bottom for each switch. */
const u_char *g_btl_flag_lines[FLAG_COUNT] = {
    g_btl_flag_name00,
    g_btl_flag_name01,
    g_btl_flag_name02,
    g_btl_flag_name03,
    g_btl_flag_name04,
    g_btl_flag_name05,
    g_btl_flag_name06,
    g_btl_flag_name07,
    g_btl_flag_name08,
    g_btl_flag_name09,
    g_btl_flag_name10,
    g_btl_flag_name11,
    g_btl_flag_name12,
    g_btl_flag_name13,
    g_btl_flag_name14,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
    g_btl_flag_name_unused,
};

/* The board's cells, and the one-glyph lines their marks are drawn from. */
extern BtlGfxText g_btl_flag_cells[FLAG_COUNT];
extern u_char     g_btl_flag_marks[FLAG_COUNT];

int BtlDebugEditFlags(void)
{
    u_char *mark;
    int     keys;
    int     i;

    BtlDrawFrame();
    BtlOpenFlagBoard();
    for (;;) {
        keys = BtlMenuKey();
        if (keys & PAD_UP) {
            if (g_btl_flag_row < FLAG_COLS) {
                g_btl_flag_row += FLAG_TAIL;
            } else {
                g_btl_flag_row -= FLAG_COLS;
            }
        }
        if (keys & PAD_DOWN) {
            if (g_btl_flag_row < FLAG_TAIL) {
                g_btl_flag_row += FLAG_COLS;
            } else {
                g_btl_flag_row -= FLAG_TAIL;
            }
        }
        if (keys & PAD_LEFT) {
            if ((g_btl_flag_row & (FLAG_COLS - 1)) != 0) {
                g_btl_flag_row--;
            } else {
                g_btl_flag_row += FLAG_COLS - 1;
            }
        }
        if (keys & PAD_RIGHT) {
            if ((g_btl_flag_row & (FLAG_COLS - 1)) != FLAG_COLS - 1) {
                g_btl_flag_row++;
            } else {
                g_btl_flag_row -= FLAG_COLS - 1;
            }
        }
        if (keys & PAD_CROSS) {
            BtlCloseMessage(0);
            BtlCloseFlagBoard();
            return 0;
        }
        if (keys & PAD_CIRCLE) {
            g_btl_debug_flags[g_btl_flag_row] ^= 1;
        }
        for (i = 0; i < FLAG_COUNT; i++) {
            if (g_btl_debug_flags[i] != 0) {
                g_btl_flag_cells[i].clut = FLAG_CLUT_ON;
            } else {
                g_btl_flag_cells[i].clut = FLAG_CLUT_OFF;
            }
        }
        if (g_btl_tick & FLAG_BLINK) {
            g_btl_flag_cells[g_btl_flag_row].clut = FLAG_CLUT_BLINK;
        } else {
            g_btl_flag_cells[g_btl_flag_row].clut = FLAG_CLUT_OFF;
        }
        for (i = 0, mark = g_btl_flag_marks; i < FLAG_COUNT; i++, mark++) {
            *mark = g_btl_debug_flags[i] != 0 ? FLAG_MARK_ON : FLAG_MARK_OFF;
        }
        BtlOpenMessage(0, 0, g_btl_flag_lines[g_btl_flag_row], FLAG_LINE_X,
                       FLAG_LINE_Y);
        BtlDrawFrame();
    }
}
