/* Persona 1 (JP) - one party member's block in the field status HUD.
 *
 *   ADV @ 0x8007C094   S2D @ 0x8007AAF0
 *
 * Both HUD redraws call this five times in a row, once per party slot, and the
 * five blocks sit at fixed places in the character-map layer: cells 2 and 14 on
 * the top row, then 122, 134 and 146 three rows below. Two members above,
 * three below.
 *
 * A block is three rows tall - the name, then HP, then SP - which is why the
 * two groups are 120 cells apart, three rows of the 40-cell-wide layer. Within
 * a block:
 *
 *      row 0   .NNNNNNNN.II.        N name, I ailment icon
 *      row 1   .HP.###/###.
 *      row 2   .SP.###/###.
 *
 * Each number is right-aligned into its three cells, and switches to the
 * warning bank once it has fallen to a quarter of its maximum.
 *
 * S2D builds this same source against a layer 0x20000 higher, which is what
 * WORK_BIAS says.
 */
#include <types.h>
#include <persona/common/char.h>

/* Reached by hardcoded address; the second of the three layers cleared at
   0x80069824, 40 cells wide. */
#define g_tilemap1   ((short *)(0x800EF580 + WORK_BIAS))
#define g_party      ((u_char *)0x801F256C)
#define g_chars      ((Char *)0x801F1BCC)

#define MAP_W        40

/* Glyph banks are 0xD7 cells apart - DrawPlaceLabel draws from bank 2 with a
   base of 0x1AE - and bank 0 puts the ten digits at 0xC0..0xC9. Bank 3 is the
   one the game turns a number to when it is running out. */
#define GLYPH_BANK   0xD7
#define BANK_DANGER  3
#define DIGIT_BASE   0xC0
#define CELL_SLASH   0xCA        /* the cell right after the ten digits */

#define LABEL_HP     0x378       /* two consecutive cells each               */
#define LABEL_SP     0x37A
#define ICON_STATUS_D  0x3DA     /* two-cell icons for the two ailments the  */
#define ICON_STATUS_10 0x3D8     /* HUD shows at all                         */

/* The scratch FormatDecimal writes its digit bytes into, per overlay. */
extern u_char g_hud_digits[];

extern const u_char str_empty[];
extern const u_char str_cell_run[];

extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);
extern void TileMapWriteRowRev(const u_char *src, short *dst, u_short base,
                               u_short count);
extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);
extern short FormatDecimal(u_int value, u_char *dst, u_short width);

void DrawPartySlotStatus(u_char slot, short cell_offset)
{
    short rows[5] = { 2, 14, 122, 134, 146 };
    short *m;
    Char *c;
    /* `n` holds the character index first and each field's digit count after.
       Both uses want the same register; splitting them into two locals does not
       compile to the same code. */
    int n;
    int bank;

    n = g_party[slot];
    c = &g_chars[n];
    m = g_tilemap1 + rows[slot] + cell_offset;
    if (n != 0xFF) {
        TileMapFillRect(m + 1, 0, 11, 1, MAP_W);
        TileMapWriteRow(c->name, m + 2, 0, 8);
        if (g_chars[n].status == 0xD) {
            m[10] = ICON_STATUS_D;
            m[11] = ICON_STATUS_D + 1;
        } else if (g_chars[n].status == 0x10) {
            m[10] = ICON_STATUS_10;
            m[11] = ICON_STATUS_10 + 1;
        }
        TileMapWriteRow(str_cell_run, m + 41, LABEL_HP, 2);
        m[47] = CELL_SLASH;
        TileMapFillRect(m + 44, 0, 3, 2, MAP_W);
        TileMapFillRect(m + 48, 0, 3, 2, MAP_W);
        n = FormatDecimal(c->hp, g_hud_digits, 3);
        bank = 0;
        if (c->hp <= c->hp_max / 4) {
            bank = BANK_DANGER;
        }
        TileMapWriteRowRev(g_hud_digits, m + 46,
                           bank * GLYPH_BANK + DIGIT_BASE, n);
        n = FormatDecimal(c->hp_max, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, m + 50, DIGIT_BASE, n);
        TileMapWriteRow(str_cell_run, m + 81, LABEL_SP, 2);
        m[87] = CELL_SLASH;
        n = FormatDecimal(c->sp, g_hud_digits, 3);
        bank = 0;
        if (c->sp <= c->sp_max / 4) {
            bank = BANK_DANGER;
        }
        TileMapWriteRowRev(g_hud_digits, m + 86,
                           bank * GLYPH_BANK + DIGIT_BASE, n);
        n = FormatDecimal(c->sp_max, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, m + 90, DIGIT_BASE, n);
    } else {
        TileMapWriteRow(str_empty, m + 44, GLYPH_BANK, 5);
    }
}
