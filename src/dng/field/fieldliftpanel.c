/* Persona 1 (JP) - the lift's panel and its arrival effect.  DNG only.
 *   0x8007270C func_8007270C
 *   0x80072D98 FieldLiftBoxes
 *   0x80073058 FieldLiftBoxesOff
 *   0x80073068 func_80073068
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
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

INCLUDE_ASM("dng/nonmatchings/field/fieldliftpanel", func_80073068);
