/* Persona 1 (JP) - the floor's music, and the events its spots start.
 * DNG only.
 *   0x80070090 FieldSyncMusic
 *   0x8007021C FieldSpotEvent
 *   0x80070660 FieldRollEncounter
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/common/char.h>
#include <persona/common/status.h>
#include <persona/common/eventflag.h>
#include <persona/dng/field.h>

void FieldSyncMusic(int keep)
{
    if (!keep) {
        if (g_ambient_on == 1 && (FLOOR_FLAGS & FLOOR_QUIET)) {
            SsSeqSetDecrescendo(g_ambient_seq, 0x7F, 300);
            g_ambient_on = 0;
        } else if (g_ambient_on == 0 && !(FLOOR_FLAGS & FLOOR_QUIET)) {
            SsSeqSetCrescendo(g_ambient_seq, 0x7F, 300);
            g_ambient_on = 1;
        }
    }
    if (FieldFindEntry() == 0) {
        if (g_floor_tune_b != -1 && g_map_music[g_dng->map][1] == 1) {
            SsSeqStop(g_floor_tune_b);
            SsSeqSetVol(g_floor_tune_b, 0, 0);
        }
    } else if (g_floor_tune_b != -1 && g_map_music[g_dng->map][1] == 1) {
        SsSeqPlay(g_floor_tune_b, 1, 0);
    }
}

#define HERE (g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]])

/* Spot i, read afresh at every use: through a pointer, loop.c would
   strength-reduce the index the image recomputes. */
#define SPOT(i) (((FloorSpot *)g_floor_events)[i])

/* Runs the event of the spot the party stands on, if it answers to the
   direction taken (the facing instead with mode bit 0x80) and its story
   flag is still clear; with mode bit 1, a TILE_QUIET tile starts nothing.
   A locked door's event that succeeds turns the door's tile into its open
   one, and on two floors opens a wall with it. Returns whether an event
   ran. */
int FieldSpotEvent(int mode)
{
    int        dir;
    int        flags;
    int        kind;
    int        i;
    int        r;

    dir = g_dng->walk_dir;
    if (g_clock_hold) {
        return 0;
    }
    if ((mode & 1) && (g_tile_defs[HERE].flags & TILE_QUIET)) {
        return 0;
    }
    flags = g_tile_defs[HERE].flags;
    if (flags & TILE_EVENT) {
        i = 0;
        kind = flags & TILE_KIND;
        for (; ; i++) {
            if (SPOT(i).x == g_dng->pos[POS_X] && SPOT(i).y == g_dng->pos[POS_Y]) {
                if (mode & 0x80) {
                    dir = g_dng->facing;
                }
                if (!(SPOT(i).facings & g_facing_bits[dir])) {
                    return 0;
                }
                if ((g_event_flags[SPOT(i).flag / 8] >> (SPOT(i).flag % 8)) & 1) {
                    return 0;
                }
                FieldPauseBgm();
                if (kind == TILE_KIND_LOCK) {
                    if (g_dng->map == 7) {
                        if (HERE == 0xA0 || HERE == 0xA1) {
                            return 0;
                        }
                    } else if (g_dng->map == 20) {
                        if (HERE >= 0x68 && HERE <= 0x69) {
                            return 0;
                        }
                    }
                }
                FieldPauseBgm();
                r = FieldRunScript(SPOT(i).event);
                if (!(flags & TILE_QUIET) && kind == TILE_KIND_LOCK) {
                    if (r == 0) {
                        FieldPlayJingle(0x1E, 1);
                        HERE += 2;
                        if (g_dng->map == 7) {
                            if (g_dng->pos[POS_X] == 0 && g_dng->pos[POS_Y] == 2) {
                                g_floor_grid[5][11] = 0x8A;
                                g_floor_grid[5][12] = 0x8B;
                                g_floor_grid[5][13] = 0x8C;
                            } else {
                                g_floor_grid[6][5] = 0x8D;
                                g_floor_grid[7][5] = 0x8E;
                                g_floor_grid[8][5] = 0x8F;
                            }
                        }
                        if (g_dng->map == 20) {
                            if (g_dng->pos[POS_X] == 20 && g_dng->pos[POS_Y] == 7) {
                                g_floor_grid[12][11] = 0x1D;
                                g_floor_grid[13][11] = 0x1D;
                                g_floor_grid[14][11] = 1;
                            } else {
                                g_floor_grid[15][11] = 0x2E;
                            }
                        }
                        FieldBuildScene(1);
                        FieldRebuildMap();
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

#define STORY_FLAG(id) ((g_event_flags[(id) >> 3] >> ((id) & 7)) & 1)

/* Rolls for an encounter on the step just taken, once the floor's
   encounters are on and the calm after the last one has run out: one in
   the tile's rate, the rare encounter first (one in 256) on a tile that
   has one, else a random entry of the set. One in eight encounters then
   rolls the hero's surprise against agility, luck, level and the moon;
   a failed roll flashes the screen. Returns whether there is a battle. */
int FieldRollEncounter(void)
{
    int    zone;
    int    set;
    int    i;
    int    m;
    Char  *c;
    u_long chars;
    short  chance;
    u_int  id;

    id = FLOOR_ENC_FLAG;
    if (!STORY_FLAG(id)) {
        return 0;
    }
    if (g_dng->enc_calm) {
        g_dng->enc_calm--;
        return 0;
    }
    zone = ((u_char (*)[FLOOR_W])g_floor_objs)[g_dng->pos[POS_Y]][g_dng->pos[POS_X]];
    i = zone & ENC_RATE; /* i doubles as the rate */
    if (!i) {
        return 0;
    }
    if (rand() % g_enc_rate_div[i] != 0) {
        return 0;
    }
    set = (g_event_flags[FLOOR_INFO->enc_set / 8] >> (FLOOR_INFO->enc_set % 8)) & 1;
    if (zone & ENC_RARE) {
        if (FLOOR_INFO->enc[set][31] != ENC_NONE && (rand() & 0xFF) == 0) {
            g_dng->enc_calm = 7;
            g_enc_id = FLOOR_INFO->enc[set][31];
            g_enc_map = g_dng->map;
            return 1;
        }
        zone &= ~ENC_RARE;
    }
    if (zone & (ENC_RARE | ENC_NO_COMMON)) {
        return; /* v0 left as it is, as the image does */
    }
    do {
    } while ((g_enc_id = FLOOR_INFO->enc[set][rand() % 31]) == ENC_NONE);
    g_dng->enc_calm = 7;
    g_enc_map = g_dng->map;
    if (!(rand() & 7)) {
        /* The records' base as a number, set before the loop: the image
           adds the index to it rather than folding it into each field. */
        chars = (u_long)g_chars;
        for (i = 0; i < 5; i++) {
            m = g_party[i];
            if (m != 0xFF) {
                c = (Char *)(m * sizeof(Char) + chars);
                if (c->key == 1) {
                    chance = (((c->stat[STAT_AGILITY] + c->stat[STAT_LUCK]) >> 1) - (c->level * 3 + 30) / 5);
                    chance = g_enc_moon_base[g_moon_group[MOON_PHASE]] + (chance + 16) * 4;
                    if (chance >= 0x100) {
                        chance = 0xFF;
                    } else if (chance < 0) {
                        chance = 0;
                    }
                    if (rand() % 256 <= chance) {
                        g_enc_surprise = 1;
                    } else {
                        g_enc_surprise = 2;
                        g_scene->pad_new = 0;
                        g_scene->pad_held = 0x8000;
                        FieldUpdate(0);
                        func_80065978();
                        g_scene->pad_held = 0x8000;
                        FieldUpdate(0);
                        func_80065978();
                    }
                }
            }
        }
    } else {
        g_enc_surprise = 0;
    }
    return 1;
}
