/* Persona 1 (JP) - stepping and drawing the display slots.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x800751BC   ADV 0x80065730   S2D 0x80065220
 *
 * Once a frame every slot's animation script is stepped and its current cel
 * list drawn. A script is a run of words: a cel list and a delay (the delay's
 * top bit says two offset halfwords follow), or one of the negative ops below.
 * A cel list is a run of 12-byte entries ended by -1, each a cel block, an
 * offset and a sort depth; a block is rows by cols cells of one size, each
 * cell its own u, v, texture page and CLUT, drawn as one GsSPRITE.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/slot.h>

/* Reached by hardcoded address; S2D's sit WORK_BIAS higher. */
#define g_slots     ((Slot *)(0x800DC10C + WORK_BIAS))
#define g_sprites   ((GsSPRITE *)(0x800DD64C + WORK_BIAS))
#define FADE_LEVEL  ((u_char *)(0x800DC00C + WORK_BIAS))

/* Script ops. */
#define OP_END  -1 /* free the slot                          */
#define OP_STOP -2 /* hold the last cel for good             */
#define OP_JUMP -3 /* continue at the script the next word gives */
#define OP_WAIT -4 /* draw nothing for the next word's frames */

/* The delay word's top bit: two offset halfwords follow. */
#define DELAY_OFFSET 0x80000000

/* A cel block: its own scale and centre follow the header when the attribute
   halfword's top bit is set; otherwise the slot's are used. */
typedef struct {
    u_char  cols;
    u_char  rows;
    u_short attr;
    u_short w;
    u_short h;
} CelBlock;

typedef struct {
    u_char  u;
    u_char  v;
    u_char  tpage;
    u_char  pad;
    u_short cx;
    u_short cy;
} CelCell;

typedef struct {
    CelBlock *block;
    short     dx;
    short     dy;
    u_short   z;
    u_short   pad;
} CelEntry;

extern u_char g_flicker_ramp[];
extern GsOT   g_ot[];
extern volatile int g_ot_index;

/* 99.65%: one instruction sits in the wrong place. In the unscaled branch
   the image keeps `cell = b + 8` after the eight loads (so sched2 later fills
   the spilled `my` store's load delay with it); here sched1 prefers the loads
   (a function-unit "potential hazard"), hoists the add to the top of the
   block, and the spill store of `my` falls outside it. No order, spelling or
   type of the branch's statements changed that.

   What got it this far: the frame holds an unused 160-byte array; `x`/`y`
   are the offset plus the slot position in variables of their own; the
   sprite is indexed afresh at every store (`g_sprites[n].f`), rotate stored
   before r/g/b and `n++` after the sort, so loop.c bases the reduced giv on
   `.r`; the columns count down at the top of the loop; and `fade` is a local
   declared after `z`, which puts the spill slots in the image's order. */
#ifdef NON_MATCHING
void SlotsRenderAll(void)
{
    u_char    unused[160];
    int       i;
    int       cols;
    int       n;
    Slot     *s;
    int      *p;
    int       op;
    CelEntry *e;
    CelBlock *b;
    CelCell  *cell;
    u_char    bright;
    u_short   z;
    u_char   *fade;
    u_long    attr;
    int       rows;
    int       c;
    u_short   w;
    u_short   h;
    u_short   sx;
    u_short   sy;
    u_short   mx;
    u_short   my;
    short     x;
    short     y;
    short     x0;
    short     ox;
    short     oy;

    fade = FADE_LEVEL;
    n = 0;
    for (i = 0; i < SLOT_COUNT; i++) {
        s = &g_slots[i];
    step:
        p = (int *)s->script;
        if (s->script != OP_END && s->script != OP_STOP) {
            if (s->delay == 0) {
                op = *p;
                if (op == OP_END) {
                    s->script = OP_END;
                    s->frame = -1;
                } else if (op == OP_STOP) {
                    s->script = op;
                    goto draw;
                } else if (op == OP_JUMP) {
                    s->script = (int)(p + 1);
                    s->script = p[1];
                    goto step;
                } else if (op == OP_WAIT) {
                    s->delay = p[1];
                    p += 2;
                    s->frame = -1;
                    s->script = (int)p;
                } else {
                    s->frame = op;
                    op = p[1];
                    p += 2;
                    s->delay = op & ~DELAY_OFFSET;
                    if (op < 0) {
                        s->unk1C = ((short *)p)[0];
                        s->unk1E = ((short *)p)[2];
                        p += 2;
                    } else {
                        s->unk1C = 0;
                        s->unk1E = 0;
                    }
                    s->script = (int)p;
                }
            }
            s->delay--;
        }
    draw:
        if (s->frame == -1 || (s->attr & SLOT_ATTR_HIDE) || !s->active) {
            continue;
        }
        e = (CelEntry *)s->frame;
        if (s->attr & SLOT_ATTR_FLICKER) {
            bright = g_flicker_ramp[s->flicker & 0x1F];
            s->flicker++;
        } else {
            if (s->attr & SLOT_ATTR_FADE_IN) {
                if (s->brightness < *fade) {
                    s->brightness += s->fade_step;
                    if (s->brightness > *fade) {
                        s->brightness = *fade;
                        s->attr &= ~SLOT_ATTR_FADE_IN;
                    }
                }
            } else if (s->attr & SLOT_ATTR_FADE_OUT) {
                if (s->brightness > 0) {
                    s->brightness -= s->fade_step;
                    if (s->brightness < 0) {
                        s->brightness = 0;
                        s->attr &= ~SLOT_ATTR_FADE_OUT;
                    }
                }
            }
            bright = s->brightness;
        }
        if (bright > *fade || *fade > 0x80) {
            bright = *fade;
        }

        do {
            z = e->z + (s->attr & SLOT_ATTR_Z);
            b = e->block;
            attr = (b->attr << 16) + ((s->attr & 0x6000) << 9) +
                   (s->attr & SLOT_ATTR_SEMITRANS);
            ox = e->dx + s->unk1C;
            oy = e->dy + s->unk1E;
            if ((long)attr < 0) {
                cols = b->cols;
                rows = b->rows;
                w = b->w;
                h = b->h;
                sx = ((u_short *)b)[4];
                sy = ((u_short *)b)[5];
                mx = w >> 1;
                my = h >> 1;
                cell = (CelCell *)((u_char *)b + 12);
            } else {
                cols = b->cols;
                rows = b->rows;
                w = b->w;
                h = b->h;
                sx = s->scale_x;
                sy = s->scale_y;
                mx = s->mx;
                my = s->my;
                cell = (CelCell *)((u_char *)b + 8);
            }
            x = ox + (s->x + s->unk18);
            y = oy + (s->y + s->unk1A);
            for (x0 = x; rows != 0; rows--) {
                for (c = cols; c != 0;) {
                    c--;
                    g_sprites[n].u = cell->u + s->u_add;
                    g_sprites[n].v = cell->v + s->v_add;
                    if (!(g_sprites[n].u == 0xFF && g_sprites[n].v == 0xFF)) {
                        g_sprites[n].x = x;
                        g_sprites[n].y = y;
                        g_sprites[n].w = w;
                        g_sprites[n].h = h;
                        g_sprites[n].tpage = cell->tpage + s->tpage_add;
                        g_sprites[n].cx = cell->cx + s->clut_x;
                        g_sprites[n].cy = cell->cy + s->clut_y;
                        g_sprites[n].attribute = attr & 0x7FFFFFFF;
                        g_sprites[n].scalex = sx;
                        g_sprites[n].scaley = sy;
                        g_sprites[n].mx = mx;
                        g_sprites[n].my = my;
                        g_sprites[n].rotate = s->rotate;
                        g_sprites[n].r = g_sprites[n].g = g_sprites[n].b = bright;
                        GsSortSprite(&g_sprites[n], &g_ot[g_ot_index], z);
                        n++;
                    }
                    x += w;
                    cell++;
                }
                y += h;
                x = x0;
            }
            e++;
        } while ((int)e->block != -1);
    }
}
#endif
