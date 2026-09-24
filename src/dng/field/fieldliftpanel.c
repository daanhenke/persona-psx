/* Persona 1 (JP) - the lift's panel and its arrival effect.  DNG only.
 *   0x8007270C func_8007270C
 *   0x80072D98 FieldLiftBoxes
 *   0x80073058 FieldLiftBoxesOff
 *   0x80073068 FieldLiftPanelStep
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/dng/field.h>

/* The arrival effect: 2 while it grows, 1 once it has, 0 off. */
extern u_char g_box_state;

/* The growing box: its size step, the corner of the next row, and which
   of the three rows is drawn next. */
extern int   g_box_t;
extern short g_box_x;
extern short g_box_y;
extern short g_box_row;

INCLUDE_ASM("dng/nonmatchings/field/fieldliftpanel", func_8007270C);

/* Clears the three rows of box lines, then grows the box a row a frame -
   each row the next size up, a line down and to the left - until it has
   reached eight times its first size. */
void FieldLiftBoxes(void)
{
    int    i, j;
    int    t;
    int    w, h;
    short  x, y;
    int    n;

    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            g_scene->boxes[j][i].attribute = 0;
            g_scene->boxes[j][i].r = 0;
            g_scene->boxes[j][i].g = 0;
            g_scene->boxes[j][i].b = 0;
            g_scene->boxes[j][i].x0 = 0;
            g_scene->boxes[j][i].y0 = 0;
            g_scene->boxes[j][i].x1 = 0;
            g_scene->boxes[j][i].y1 = 0;
        }
    }
    g_box_t = 0x1000;
    g_box_x = 0x48;
    g_box_y = 0;
    g_box_row = 0;
    func_80065978();
    func_80065978();
    func_80065978();
    g_box_state = 2;
    while (g_box_t <= 0x8000) {
        t = g_box_t;
        g_box_t = t + 0xAAA;
        n = g_box_row;
        w = t * 8 >> 12;
        h = t * 21 >> 12;
        g_scene->boxes[n][0].x0 = x = g_box_x;
        g_scene->boxes[n][0].y0 = y = g_box_y;
        g_scene->boxes[n][0].x1 = x + w;
        g_scene->boxes[n][0].y1 = y;
        g_scene->boxes[n][1].x0 = x + w;
        g_scene->boxes[n][1].y0 = y;
        g_scene->boxes[n][1].x1 = x + w;
        g_scene->boxes[n][1].y1 = y + h;
        g_scene->boxes[n][2].x0 = x + w;
        g_scene->boxes[n][2].y0 = y + h;
        g_scene->boxes[n][2].x1 = x;
        g_scene->boxes[n][2].y1 = y + h;
        g_scene->boxes[n][3].x0 = x;
        g_scene->boxes[n][3].y0 = y + h;
        g_scene->boxes[n][3].x1 = x;
        g_scene->boxes[n][3].y1 = y;
        g_box_x = x + 1;
        g_box_y = y - 8;
        g_box_row = (n + 1) % 3;
        func_80065978();
    }
    g_box_state = 1;
}

void FieldLiftBoxesOff(void)
{
    g_box_state = 0;
}

/* The panel's buttons: two columns of floors and a close row below them,
   the cursor a sprite; the sequences it clicks, confirms and buzzes with. */
#define PANEL_CLOSE_ROW 3
#define PANEL_CURSOR    72
#define SEQ_CLICK   3
#define SEQ_CONFIRM 4
#define SEQ_BUZZ    5

#define PANEL_FLOOR(x, y) (g_lift_from[g_scene->lift][(y) * 2 + (x)])

/* A frame of the panel: OK on a floor other than this one leaves for it,
   OK on the close row or back closes the panel, and up, down and the sides
   move the cursor past the empty buttons. Returns the button + 1 chosen,
   -1 when closed, 0 otherwise. */
int FieldLiftPanelStep(void)
{
    int     b;
    u_char *row;

    if (g_scene->pad_new & BIND(lift_ok)) {
        b = g_scene->lift_x + g_scene->lift_y * 2;
        if (g_scene->lift_y == PANEL_CLOSE_ROW) {
            SsPlayBack(g_seq_handles[SEQ_CONFIRM], 0, 1);
            return -1;
        }
        row = g_lift_from[g_scene->lift];
        if (row[b] != 0xFF && b != g_scene->lift_btn) {
            SsPlayBack(g_seq_handles[SEQ_CONFIRM], 0, 1);
            return b + 1;
        }
        SsPlayBack(g_seq_handles[SEQ_BUZZ], 0, 1);
    }
    if (g_scene->pad_new & BIND(lift_back)) {
        SsPlayBack(g_seq_handles[SEQ_BUZZ], 0, 1);
        return -1;
    }
    if ((g_scene->pad_new & PAD_UP) && g_scene->lift_y != 0) {
        SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
        g_scene->lift_y--;
        while (g_scene->lift_y >= 0 &&
               PANEL_FLOOR(g_scene->lift_x, g_scene->lift_y) == 0xFF) {
            g_scene->lift_y--;
        }
    } else if ((g_scene->pad_new & PAD_DOWN) && g_scene->lift_y != PANEL_CLOSE_ROW) {
        SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
        g_scene->lift_y++;
        while ((g_scene->lift_y < 4) &
               (PANEL_FLOOR(g_scene->lift_x, g_scene->lift_y) == 0xFF)) {
            g_scene->lift_y++;
        }
    } else if ((g_scene->pad_new & (PAD_LEFT | PAD_RIGHT)) &&
               g_scene->lift_y != PANEL_CLOSE_ROW) {
        SsPlayBack(g_seq_handles[SEQ_CLICK], 0, 1);
        g_scene->lift_x ^= 1;
        while (PANEL_FLOOR(g_scene->lift_x, g_scene->lift_y) == 0xFF) {
            g_scene->lift_x ^= 1;
        }
    }
    if (g_scene->lift_y == PANEL_CLOSE_ROW) {
        g_scene->sprites[PANEL_CURSOR].x = 0x58;
        g_scene->sprites[PANEL_CURSOR + 1].attribute &= 0x7FFFFFFF;
        g_scene->sprites[PANEL_CURSOR + 2].attribute &= 0x7FFFFFFF;
    } else {
        if (g_scene->lift_one_col) {
            g_scene->sprites[PANEL_CURSOR].x = 0x68;
        } else {
            g_scene->sprites[PANEL_CURSOR].x = g_scene->lift_x * 32 + 0x58;
        }
        g_scene->sprites[PANEL_CURSOR + 1].attribute |= 0x80000000;
        g_scene->sprites[PANEL_CURSOR + 2].attribute |= 0x80000000;
    }
    g_scene->sprites[PANEL_CURSOR].y = g_scene->lift_y * 24 - 16;
    return 0;
}
