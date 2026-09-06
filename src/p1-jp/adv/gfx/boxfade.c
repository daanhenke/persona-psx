/* Persona 1 (JP) - fading the screen behind a filled box.
 *   0x80085F80 AdvBoxFadeDown   0x800860F0 AdvBoxFadeUp
 *
 * A grey box the size of the screen is sorted in front of everything at
 * priority 16 and its level ramped a step a frame, so the picture behind it
 * darkens or brightens without any of the sprite fades being touched. The box
 * is placed rather than fixed at the origin, which is what lets the ADVCHR
 * sequence fade the part of the screen its picture is on.
 *
 * The ramp overshoots on the last step, so the end level is drawn once more on
 * the way out rather than being left wherever the step landed.
 */
#include <types.h>
#include <libgs.h>

/* In front of the whole ordering table. */
#define BOX_PRI 16

/* The box covers the screen from wherever it is placed. */
#define BOX_W 320
#define BOX_H 240

/* Flat-shaded, semi-transparent off. */
#define BOX_ATTR 0x50000000

extern GsOT  g_ot[];
extern int   g_ot_index;

extern void AdvRunFrame(void);
extern void AdvRenderCharFrame(short extra);

/* Darkens from `from` down past `to`. */
void AdvBoxFadeDown(short step, short from, short to, short x, u_short y,
                    short char_frame)
{
    GsBOXF box;
    short  level;

    box.attribute = BOX_ATTR;
    box.x = x;
    box.y = y;
    box.w = BOX_W;
    box.h = BOX_H;
    box.r = 0;
    box.g = 0;
    box.b = 0;
    level = from;
    do {
        box.r = box.g = box.b = level;
        GsSortBoxFill(&box, &g_ot[g_ot_index], BOX_PRI);
        if (char_frame == 0) {
            AdvRunFrame();
        } else {
            AdvRenderCharFrame(0);
        }
        level -= step;
    } while (level > to);

    box.r = box.g = box.b = to;
    GsSortBoxFill(&box, &g_ot[g_ot_index], BOX_PRI);
    if (char_frame == 0) {
        AdvRunFrame();
    } else {
        AdvRenderCharFrame(0);
    }
}

/* The same upwards. */
void AdvBoxFadeUp(short step, short from, short to, short x, u_short y,
                  short char_frame)
{
    GsBOXF  box;
    GsBOXF *b;
    int     end;
    int     level;

    box.attribute = BOX_ATTR;
    box.x = x;
    box.y = y;
    box.w = BOX_W;
    box.h = BOX_H;
    box.r = 0;
    box.g = 0;
    box.b = 0;
    level = from;
    end   = to;
    do {
        box.r = box.g = box.b = level;
        b = &box;
        GsSortBoxFill(b, &g_ot[g_ot_index], BOX_PRI);
        if (char_frame == 0) {
            AdvRunFrame();
        } else {
            AdvRenderCharFrame(0);
        }
        level += step;
    } while (level < end);

    box.r = box.g = box.b = end;
    GsSortBoxFill(&box, &g_ot[g_ot_index], BOX_PRI);
    if (char_frame == 0) {
        AdvRunFrame();
    } else {
        AdvRenderCharFrame(0);
    }
}
