/* Persona 1 (JP) - the lift's panel and its arrival effect.  DNG only.
 *   0x8007270C FieldLiftPanelOpen
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

/* The growing box: its size step, the corner of the next row, and which
   of the three rows is drawn next. */
extern int   g_box_t;
extern short g_box_x;
extern short g_box_y;
extern short g_box_row;

/* Where each lift stands: the map, floor and tile, and the lift's row of
   g_lift_stops with LIFT_ONE_COL set for a panel of one column. */
typedef struct {
    u_char map, floor, x, y;
    u_char lift;
} LiftSpot;
#define LIFT_ONE_COL 0x80
extern LiftSpot g_lift_spots[];

/* Per floor number, the u of its button's cell; and the indicator digits'
   places on the panel. */
extern int g_lift_btn_u[];
extern int g_lift_digit_x[];
extern int g_lift_digit_y[];

/* The indicator's digit sprites, as in fieldlift.c. */
#define LIFT_SPRITE 82
#define LIFT_DIGITS 7

#define PANEL_BUTTONS 6
#define PANEL_BUTTON  0x4C

/* Opens the panel of the lift the party stands in: which lift and which
   of its buttons is this floor, the panel's frame, the cursor and close
   button, a button for each floor the lift stops at (none where it does
   not), and the indicator's digits. */
void FieldLiftPanelOpen(void)
{
    int i;
    int n;
    u_char *row;

    for (i = 0; ; i++) {
        if (g_lift_spots[i].map == g_dng->map && g_lift_spots[i].floor == g_dng->floor &&
            g_lift_spots[i].x == g_dng->pos[POS_X] && g_lift_spots[i].y == g_dng->pos[POS_Y]) {
            break;
        }
    }
    g_scene->lift = g_lift_spots[i].lift & ~LIFT_ONE_COL;
    g_scene->lift_one_col = g_lift_spots[i].lift & LIFT_ONE_COL;
    for (g_scene->lift_btn = 0; g_dng->floor != g_lift_from[g_scene->lift][g_scene->lift_btn];
         g_scene->lift_btn++) {
    }

    FieldInitSprite(0x46, 0x40, 0x40, 0x15, 0, 0, 0, 0x1E1);
    g_scene->sprites[0x46].x = 0x50;
    g_scene->sprites[0x46].y = -0x58;
    FieldInitSprite(0x47, 0x40, 0x68, 0x15, 0x40, 0, 0, 0x1E1);
    g_scene->sprites[0x47].x = 0x50;
    g_scene->sprites[0x47].y = -0x18;
    FieldInitSprite(0x48, 0x10, 0x10, 5, 0xA0, 0x68, 0x100, 0x1EB);
    g_scene->sprites[0x48].attribute = 0x40000000;
    FieldInitSprite(0x49, 0x10, 0x10, 5, 0xA0, 0x68, 0x100, 0x1EB);
    g_scene->sprites[0x49].attribute = 0x40000000;
    g_scene->sprites[0x49].x = 0x68;
    g_scene->sprites[0x49].y = 0x38;
    FieldInitSprite(0x4A, 0x10, 0x10, 5, 0xA0, 0x68, 0x100, 0x1EB);
    g_scene->sprites[0x4A].attribute = 0x40000000;
    g_scene->sprites[0x4A].x = 0x78;
    g_scene->sprites[0x4A].y = 0x38;
    FieldInitSprite(0x4B, 0x30, 0x10, 5, 0x70, 0x68, 0x100, 0x1EB);
    g_scene->sprites[0x4B].x = 0x58;
    g_scene->sprites[0x4B].y = 0x38;
    g_scene->sprites[0x4B].attribute = 0;
    FieldInitSprite(0x59, 0x10, 0x10, 5, 0x60, 0x68, 0x100, 0x1EB);
    g_scene->sprites[0x59].x = 0x58;
    g_scene->sprites[0x59].y = -0x38;
    g_scene->sprites[0x59].attribute = 0;

    for (i = 0; i < PANEL_BUTTONS; i++) {
        if (g_lift_stops[g_scene->lift][i] == 0) {
            g_scene->sprites[PANEL_BUTTON + i].attribute = 0x80000000;
        } else {
            FieldInitSprite(PANEL_BUTTON + i, 0x10, 0x10, 5, g_lift_btn_u[g_lift_stops[g_scene->lift][i]],
                            g_lift_stops[g_scene->lift][i] / 13 * 16 + 0x58, 0x100, 0x1EB);
            g_scene->sprites[PANEL_BUTTON + i].attribute = 0;
        }
        if (g_scene->lift_one_col) {
            g_scene->sprites[PANEL_BUTTON + i].x = 0x68;
        } else {
            g_scene->sprites[PANEL_BUTTON + i].x = (i & 1) * 32 + 0x58;
        }
        g_scene->sprites[PANEL_BUTTON + i].y = i / 2 * 24 - 16;
    }
    for (i = 0; i < LIFT_DIGITS; i++) {
        n = LIFT_SPRITE + i;
        FieldInitSprite(n, 0, 0, 5, 0, 0, 0x100, 0x1EB);
        g_scene->sprites[n].x = g_lift_digit_x[i] + 0x50;
        g_scene->sprites[n].y = g_lift_digit_y[i] - 0x58;
        g_scene->sprites[n].attribute = 0;
    }
    g_scene->lift_x = g_scene->lift_btn & 1;
    g_scene->lift_y = g_scene->lift_btn >> 1;
    row = g_lift_stops[g_scene->lift];
    g_scene->lift_at = row[g_scene->lift_btn];
}

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
    FieldFrame();
    FieldFrame();
    FieldFrame();
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
        FieldFrame();
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
