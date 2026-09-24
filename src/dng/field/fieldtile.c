/* Persona 1 (JP) - what the tile the party lands on does.  DNG only.
 *   0x8006BBC0 FieldTileEffect
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Save flags the floors keep: bit 4 of the first marks the switch puzzle
   solved, bit 3 whether map 0x21's puzzle is live; bit 6 of the second
   retires map 0x14's. */
#define PUZZLE_FLAGS  (*(u_char *)0x801F29EF)
#define PUZZLE_FLAGS2 (*(u_char *)0x801F29F2)
/* Picks which of the two over-the-clock events runs. */
#define CLOCK_EVENT_ALT (*(u_char *)0x801F29A8)

/* The two switch puzzles (map 0x21, then any other): the rectangle of
   tiles each covers and, per tile, whether it must be on. */
extern u_char g_puzzle_w[2];
extern u_char g_puzzle_h[2];
extern u_char g_puzzle_x[2];
extern u_char g_puzzle_y[2];
extern u_char g_puzzle_goal[2][5][5];

#define HERE (g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]])

/* The tile's kind with the event and music bits masked out. */
#define TILE_EFFECT_MASK 0xFE1F
#define TILE_PIT         4
#define TILE_DAMAGE1     5
#define TILE_SWITCH      6
#define TILE_DAMAGE2     7
#define TILE_DAMAGE4     8
#define TILE_DAMAGE8     9
#define TILE_POISON      10

#define WINDOW 11

/* After a step: a pit drops the party a floor, in the dark, and reloads
   the floor below; a finished effect that lasts so many steps runs its
   event; a spot event on the tile runs; then, unless the step was a turn,
   a switch tile flips and checks its puzzle, and damage and poison floors
   hurt the party - but not on the step an effect ran out. Returns 1 when a
   spot event ran. The fall's angle and the light level, and then every
   counter, are two variables reused throughout, and the clamps carry their
   step as a side effect - without it fold turns them into MIN/MAX. */
/* 98.35%: the only difference is which copy of the puzzle's jingle call
   cross-jumping keeps. Both arms end in FieldPlayJingle; loop.c moves the
   "bit set" arm out of the loop, and the later jump pass merges the two
   calls - keeping the in-loop one here, the moved one in the image. Every
   ordering of the arms, a shared call, a default number and a goto to the
   function's end leave it the same or worse. */
#ifdef NON_MATCHING
int FieldTileEffect(void)
{
    int      effect_ran;
    int      i;   /* the fall's angle, then a counter */
    int      j;   /* the light level, then a counter */
    int      eye;
    int      k;
    int      x, y, n;
    int      flags;
    int      base;
    u_char   cx, cy;
    TileDef *t;

    effect_ran = 0;
    if (g_noclip) {
        return 0;
    }
    if ((g_tile_defs[HERE].flags & TILE_EFFECT_MASK) == TILE_PIT) {
        FieldPauseBgm();
        i = 0;
        FieldPlayJingle(0x1D, 1);
        eye = -150;
        j = 0x1000;
        do {
            i = (i -= 0x6E) < -0x3FF ? -0x400 : i;
            g_dng->view.vpy = eye - (rsin(i) * 50 >> 12);
            g_dng->view.vry = eye + (rsin(i) * 8000 >> 12);
            eye += 12;
            j = (j -= 0x100) > 0 ? j : 0;
            GsSetAmbient(j, j, j);
            func_80065978();
        } while (j != 0);
        GsSetAmbient(0, 0, 0);
        g_field_lit = 0;
        func_80065978();
        g_dng->floor = g_dng->floor != 0 ? g_dng->floor - 1 : 0;
        FieldSetFloor();
        FieldSetupGfx(0);
        for (i = 0; i < 5; i++) {
            func_80065978();
        }
        j = 0;
        g_field_lit = 1;
        g_dng->view.vpy = -270;
        do {
            j = (j += 0x100) < 0x1000 ? j : 0x1000;
            GsSetAmbient(j, j, j);
            func_80065978();
        } while ((g_dng->view.vpy += 30) != -60);
        while ((g_dng->view.vpy -= 5) != -150) {
            j = (j += 0x100) < 0x1000 ? j : 0x1000;
            GsSetAmbient(j, j, j);
            func_80065978();
        }
        g_scene->pad_new = 0;
        g_scene->pad_held = 0;
    }

    if (g_step_kind == 0) {
        return 0;
    }
    if (g_effect_over) {
        g_effect_over = 0;
        FieldPauseBgm();
        effect_ran = 1;
        func_80073A64(CLOCK_EVENT_ALT ? 0x101 : 0x100);
    }
    if (FieldSpotEvent(0x81)) {
        return 1;
    }
    if ((u_char)(g_step_kind - 3) < 2) {
        return 0;
    }
    flags = g_tile_defs[HERE].flags & TILE_EFFECT_MASK;
    if (flags == TILE_SWITCH) {
        if (!((g_dng->map == 0x21 && !(PUZZLE_FLAGS & 8)) ||
              (g_dng->map == 0x14 && !(PUZZLE_FLAGS2 & 0x40)))) {
            return 0;
        }
        HERE ^= 1;
        cy = g_dng->pos[POS_Y] % WINDOW;
        cx = g_dng->pos[POS_X] % WINDOW;
        t = &g_tile_defs[HERE];
        base = (cy * WINDOW + cx) * 8;
        for (i = 0; i < 8; i++) {
            FieldPlaceObject(base + i, t->models[i], g_dng->pos[POS_X], g_dng->pos[POS_Y]);
        }
        k = g_dng->map != 0x21;
        y = g_puzzle_y[k];
        for (i = 0; i < g_puzzle_h[k]; y++, i++) {
            x = g_puzzle_x[k];
            for (j = 0; j < g_puzzle_w[k]; x++, j++) {
                if (g_puzzle_goal[k][i][j] != (g_floor_grid[y][x] & 1)) {
                    if (PUZZLE_FLAGS & 0x10) {
                        PUZZLE_FLAGS &= ~0x10;
                        FieldPlayJingle(0x1C, 1);
                        return 0;
                    }
                    FieldPlayJingle(0x1F, 1);
                    return 0;
                }
            }
        }
        PUZZLE_FLAGS |= 0x10;
        FieldPlayJingle(0x1B, 1);
    } else if (flags == TILE_DAMAGE1) {
        if (!effect_ran) {
            FieldDamageFloor(1);
        }
    } else if (flags == TILE_DAMAGE2) {
        if (!effect_ran) {
            FieldDamageFloor(2);
        }
    } else if (flags == TILE_DAMAGE4) {
        if (!effect_ran) {
            FieldDamageFloor(4);
        }
    } else if (flags == TILE_DAMAGE8) {
        if (!effect_ran) {
            FieldDamageFloor(8);
        }
    } else if (flags == TILE_POISON) {
        if (!effect_ran) {
            FieldPoisonFloor();
        }
    }
    return 0;
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldtile", FieldTileEffect);
#endif
