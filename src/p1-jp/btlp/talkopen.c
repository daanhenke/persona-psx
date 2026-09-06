/* Persona 1 (JP) - filling the negotiation box.  BTLP only.
 *   0x8007B670 BtlTalkOpen
 *
 * Four command labels, the character's level and their name. The labels come
 * out of g_btl_talk_labels by Char key, so each character offers their own
 * four; the other two lines are built here because they are not fixed text.
 *
 * Both are written in the escaped form the box's font wants - 0x80 before
 * every glyph - and terminated with 0xFF 0xF5. The level's digits are taken
 * down by 0x40 on the way, which is what puts them in the small set the box
 * draws numbers with, and the name is capped at sixteen bytes so a long one
 * cannot run past the buffer.
 */
#include <types.h>

/* Commands a character offers. */
#define TALK_LABELS 4

/* The escape the box's font wants before every glyph, and the end marker. */
#define TALK_ESCAPE 0x80
#define TALK_END    0xFF
#define TALK_END2   0xF5

/* Digits arrive 0x40 above the set the box draws them in. */
#define TALK_DIGIT_BIAS 0x40

/* As much of a name as the line holds. */
#define TALK_NAME_MAX 0x10

/* One entry: the text, and where it goes. */
typedef struct {
    /* 0x0 */ const u_char *text;
    /* 0x4 */ short         x;
    /* 0x6 */ short         y;
} BtlMenuCell;                      /* 8 bytes */

extern BtlMenuCell    g_btl_talk_cells[];
extern const u_char  *g_btl_talk_labels[];
extern u_char         g_btl_talk_level_text[];
extern u_char         g_btl_talk_name_text[];
extern int            g_btl_talk_state;
extern int            g_btl_talk_index;

extern void    BtlUploadText(const BtlMenuCell *cell, const u_char *text);
extern u_char *BtlFormatDecimal(int value, u_char *dst, int pad);
extern void    BtlQueueVramClear(short x, short y, short w, short h,
                                 int r, int g, int b);
extern void    BtlTalkIdle(void);

void BtlTalkOpen(int key, int level, const char *name)
{
    u_char digits[24];
    /* The two glyphs for LV, already escaped. */
    u_char head[4] = { TALK_ESCAPE, 0xB1, TALK_ESCAPE, 0xBB };
    u_char spare[8];
    BtlMenuCell *cell;
    u_char *out;
    const u_char *p;
    int i;

    i = 0;
    cell = g_btl_talk_cells;
    do {
        BtlUploadText(cell, g_btl_talk_labels[key * TALK_LABELS + i]);
        cell++;
        i++;
    } while (i < TALK_LABELS);

    BtlFormatDecimal(level, digits, 0);

    i = 0;
    p = head;
    do {
        g_btl_talk_level_text[i] = *p;
        i++;
        p++;
    } while (i < 4);

    p = digits;
    if (digits[0] != TALK_END) {
        out = &g_btl_talk_level_text[i];
        do {
            *out++ = TALK_ESCAPE;
            i += 2;
            *out++ = *p++ - TALK_DIGIT_BIAS;
        } while (*p != TALK_END);
    }
    g_btl_talk_level_text[i] = TALK_END;
    g_btl_talk_level_text[i + 1] = TALK_END2;

    i = 0;
    if (*name != (char)TALK_END) {
        out = g_btl_talk_name_text;
        for (;;) {
            if (i > TALK_NAME_MAX - 1) {
                break;
            }
            *out++ = TALK_ESCAPE;
            i += 2;
            *out++ = *name++;
            if (*name == (char)TALK_END) {
                break;
            }
        }
    }
    g_btl_talk_name_text[i] = TALK_END;
    g_btl_talk_name_text[i + 1] = TALK_END2;

    BtlUploadText(&g_btl_talk_cells[4], g_btl_talk_level_text);
    BtlUploadText(&g_btl_talk_cells[5], g_btl_talk_name_text);
    BtlQueueVramClear(0x380, 0x150, 0x40, 0x30, 0, 0, 0);
    g_btl_talk_state = 1;
    g_btl_talk_index = 0;
    BtlTalkIdle();
}
