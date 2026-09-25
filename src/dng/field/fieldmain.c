/* Persona 1 (JP) - the field's entry point: loading a floor and walking it.
 * DNG only.
 *   0x80064D2C ovl_dng_entry
 *   0x80065978 FieldFrame
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <stdlib.h>
#include <libsnd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include <persona/main/state.h>
#include <persona/common/char.h>
#include <persona/common/status.h>
#include <persona/common/automap.h>
#include <persona/dng/field.h>

extern void LoadFileToAddrAsync(char *name, void *dest);
extern void LoadFileToAddr(char *name, void *dest);
extern void bcopy(void *src, void *dst, int n);
extern void func_80015384(short map, int kind, short *vab);


/* The state block's map and floor, and the party's tile, as words: map 9
   floor 0, and a tile with its x and y bytes masked out of the word. With
   g_dng just set, cse folds the first reads to the fixed address. */
#define MAP_FLOOR (*(int *)&g_dng->map)
#define TILE_WORD (*(int *)g_dng->pos)

/* The pack's image table, at its head. */
#define PACK_IMAGES ((u_long *)PACK_BASE)
/* The index as the disc holds it, copied over PACK_INDEX for a new map. */
#define INDEX_DISC  ((void *)0x801CA000)
#define INDEX_SIZE  0x3000

/* The VAB headers' offsets in the sequence data. */
extern u_long D_801CD080;
extern u_long D_801CD084;
extern u_long D_801CD088;

extern u_char D_8001555C[];

/* Bit 1: on map 9, the ambient sequence plays. */
#define MAP9_FLAGS (*(u_char *)0x801F29ED)
/* Set while the floor's countdown runs out the game. */
#define CLOCK_ARMED ((u_char *)0x801F29C4)

#define SEQ(n) ((u_long *)(SEQ_BASE + SEQ_OFFSETS[n]))


#define HERE (g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]])

/* The map whose tunes stop for the countdown, and the tunes' handles. */
#define MAP_CLOCK 28
#define TUNE_A    13
#define TUNE_B    14

static inline void MarkSeen(u_int x, int y)
{
    *(g_map_seen + (g_map_base[g_dng->area] + g_dng->room) * MAP_BYTES + y * MAP_ROW_BYTES + (x >> 3)) |= 0x80 >> (x & 7);
}

void ovl_dng_entry(void)
{
    u_char  unused[0xB8]; /* the image's frame holds 0xB8 bytes nothing touches */
    int     i;
    u_char  facing;
    u_char *armed;
    short  *h;
    int     tune;
    int     m;
    Char   *c;

    FieldInitGraph();
    g_scene = (DngScene *)0x800C0000;
    g_dng = (DngState *)0x801F0000;
    g_clock_hold = 0;
    g_effect_over = 0;
    if (MAP_FLOOR == 9 && (TILE_WORD & 0xFF00FF) == 0x80000) {
        MAP9_FLAGS |= 2;
    }
    if ((MAP_FLOOR == 9 && (TILE_WORD & 0xFF00FF) == 0x80004) || g_dng->map != 9) {
        MAP9_FLAGS &= ~2;
    }
    if (g_dng->last_map != g_dng->map || g_state_prev == GAME_STATE_S2D) {
        bcopy(INDEX_DISC, PACK_INDEX, INDEX_SIZE);
        g_dng->last_map = g_dng->map;
        g_dng->last_floor = g_dng->floor;
        if (g_state_prev != GAME_STATE_DNG) {
            g_dng->tick_flags = 0;
        }
    }
    g_pack_images = PACK_IMAGES;
    g_pack_tmd_tab = g_pack_images + PACK_INDEX[g_pack_sel];
    g_pack_cell_tab = g_pack_tmd_tab + PACK_INDEX[g_pack_sel + 1];
    g_pack_msg_tab = g_pack_cell_tab + PACK_INDEX[g_pack_sel + 2];
    g_pack_model_tab = g_pack_msg_tab + PACK_INDEX[g_pack_sel + 3];
    g_pack_obj_tab = g_pack_model_tab + 1;
    g_pack_spot_tab = g_pack_obj_tab + PACK_INDEX[g_pack_sel + 4];
    g_pack_event_tab = g_pack_spot_tab + PACK_INDEX[g_pack_sel + 4];
    g_floor_event_tab = g_pack_event_tab + PACK_INDEX[g_pack_sel + 4];
    g_pack_tims = g_floor_event_tab + PACK_INDEX[g_pack_sel + 5];
    g_index_tile_tab = (u_long *)(INDEX_BASE + 4);
    g_index_info_tab = (u_long *)(INDEX_BASE + 8);
    g_index_grid_tab = g_index_info_tab + PACK_INDEX[g_pack_sel + 4];
    FieldSetFloor();
    FieldEnterFrom();
    FieldLoadGfx();
    DrawSync(0);
    if (!SOUND_KEPT) {
        FieldResetSound();
    } else if (g_dng->exit_bits & 0x40) {
        SsSetMarkCallback(g_seq_handles[1], 0, D_8001555C);
    }
    if (!SOUND_KEPT) {
        do {
            g_vab_handles[0] = SsVabOpenHead((u_char *)(SEQ_BASE + D_801CD080), -1);
        } while (g_vab_handles[0] == -1);
        func_80015384(g_dng->map, 0x41, &g_vab_handles[0]);
        do {
            g_vab_handles[1] = SsVabOpenHead((u_char *)(SEQ_BASE + D_801CD084), -1);
        } while (g_vab_handles[1] == -1);
        func_80015384(g_dng->map, 0x42, &g_vab_handles[1]);
    }
    do {
        g_vab_handles[2] = SsVabOpenHead((u_char *)(SEQ_BASE + D_801CD088), -1);
    } while (g_vab_handles[2] == -1);
    func_80015384(g_dng->map, 0x43, &g_vab_handles[2]);
    LoadFileToAddrAsync("\\ADV\\ADVCMD.BIN;1", (void *)0x80180000);
    g_field_lit = 1;
    if (g_state_prev == GAME_STATE_BTL) {
        FieldSetupGfx(1);
    } else {
        FieldSetupGfx(0);
    }
    FieldOpenSound();
    FieldEnterTile();
    FieldFadeIn();
    armed = CLOCK_ARMED;
    h = g_seq_handles;

    for (;;) {
        if (FieldUpdate(0) != 0) {
            break;
        }
        FieldFrame();
        if (*armed != 0 && (u_char)g_clock_hours >= g_clock_limit_hours[g_dng->map]
            && (u_char)g_clock_min >= g_clock_limit_min[g_dng->map]) {
            if (g_dng->map == MAP_CLOCK) {
                *armed = 0;
                SsSeqStop(g_seq_handles[TUNE_B]);
            } else {
                FieldPauseBgm();
                *armed = 0;
                for (i = 0; i < FIELD_SEQS; i++) {
                    if (h[i] != -1 && i != TUNE_A) {
                        SsSeqSetDecrescendo(h[i], 0x7F, 0x3C);
                    }
                }
                for (i = 0; i < 20; i++) {
                    FieldFrame();
                }
                FieldCloseSound();
                LoadFileToAddr("\\D04\\TUP.BIN;1", (void *)SEQ_BASE);
                do {
                    g_vab_handles[0] = SsVabOpenHead((u_char *)SEQ(2), -1);
                } while (g_vab_handles[0] == -1);
                LoadFileToAddr("\\D04\\TUP.VB;1", (void *)0x80170000);
                SsVabTransBody((u_char *)0x80170000, g_vab_handles[0]);
                SsVabTransCompleted(1);
                h[0] = SsSeqOpen(SEQ(0), g_vab_handles[0]);
                g_seq_handles[4] = SsSeqOpen(SEQ(1), g_vab_handles[0]);
                SsSeqSetVol(h[0], 0x7F, 0x7F);
                SsSeqSetVol(g_seq_handles[4], 0x7F, 0x7F);
                SsPlayBack(h[0], 0, 0);
                FieldRunScript(0);
                SsSeqSetDecrescendo(h[0], 0x7F, 0x3C);
                g_state_next = GAME_STATE_NONE;
                break;
            }
        }
        MarkSeen(g_dng->pos[POS_X], g_dng->pos[POS_Y]);
        if (FieldTileEffect() != 0) {
            continue;
        }
        if (g_step_kind == 0 || (u_char)(g_step_kind - 3) < 2) {
            continue;
        }
        g_dng->tick_flags++;
        if (g_dng->tick_flags & 1) {
            for (i = 0; i < 5; i++) {
                m = g_party[i];
                if (m != 0xFF && g_chars[m].sp != g_chars[m].sp_max) {
                    g_chars[m].sp++;
                }
            }
        }
        if (!((g_dng->map == 1 || g_dng->map == 5) && (g_quest_bits & 0x10))) {
            if (g_music_x <= 0xF0
                && (abs(g_music_x - g_dng->pos[POS_X]) > 0
                    || abs(g_music_y - g_dng->pos[POS_Y]) > 0)) {
                if (g_map_music[g_dng->map][0] == 2 && g_seq_handles[TUNE_A] != -1) {
                    SsSeqStop(g_seq_handles[TUNE_A]);
                } else if (g_map_music[g_dng->map][1] == 2 && g_seq_handles[TUNE_B] != -1) {
                    SsSeqStop(g_seq_handles[TUNE_B]);
                }
                g_music_x = 0xFF;
                g_music_y = 0xFF;
            }
            if ((g_tile_defs[HERE].flags & TILE_MUSIC) && g_music_x >= 0xF0) {
                if (g_map_music[g_dng->map][0] == 2) {
                    tune = g_seq_handles[TUNE_A];
                } else {
                    tune = g_seq_handles[TUNE_B];
                }
                SsPlayBack(tune, 0, 0);
                g_music_x = g_dng->pos[POS_X];
                g_music_y = g_dng->pos[POS_Y];
            }
        }
        if (!g_dng->no_enc) {
            facing = g_dng->facing;
            if (FieldRollEncounter()) {
                FieldStartBattle();
                g_dng->facing = facing;
                break;
            }
        }
        if (g_hurt_flash != 0x80) {
            for (i = 0; i < 5; i++) {
                m = g_party[i];
                if (m == 0xFF) {
                    continue;
                }
                c = &g_chars[m];
                if (c->status == STATUS_SICK && !(g_dng->tick_flags & 3)) {
                    FieldPlaySeq(10, 2);
                    g_hurt_flash = 6;
                    if ((c->hp -= c->hp_max / 8) <= 0) {
                        c->hp = 1;
                    }
                } else if (c->status == STATUS_POISON && !(g_dng->tick_flags & 1)) {
                    FieldPlaySeq(10, 1);
                    g_hurt_flash = 2;
                    if (c->hp != 1) {
                        c->hp--;
                    }
                }
            }
        } else {
            g_hurt_flash = 0;
        }
    }
    if (g_field_lit) {
        FieldFadeOut();
    }
    if (g_state_next == GAME_STATE_BTL) {
        SsSetNck(g_seq_handles[18]);
        SsVabClose(g_vab_handles[1]);
    }
}

/* Last frame's pad, kept in the slot before the floor's first TMD. */
#define PAD_PREV (*(int *)&g_scene->models[0])

/* The view's culling tables, per depth band of 75: how far past the
   screen's edge a tile's centre may lie and still be drawn, and what its
   screen x is divided by to push the ordering point back. */
extern u_char g_view_margin[];
extern u_char g_ot_xdiv[];

/* The colour the frame clears to. */
extern u_char g_clear_r;
extern u_char g_clear_g;
extern u_char g_clear_b;

/* The alarm's pulsing red: the ambient level and its step, +1 or -1. The
   level's += is a 16-bit add, which is why the image reads it again. */
extern u_short g_alarm_level;
extern u_short g_alarm_step;

/* The option that keeps the minimap north-up, turning the compass instead. */
extern u_char g_map_north_up;

/* Counts frames for the water's bob, which moves every other one. */
extern u_short g_bob_tick;

extern u_char  D_801F29ED;
extern MATRIX  D_8005E6F0;

#define BUF        g_draw_buf
#define GRID_TILE  (g_tile_defs[g_floor_grid[v[b]][v[a]]])
#define MODEL_DEF  (g_model_defs[GRID_TILE.models[m]])
#define SCRATCH    ((u_long *)0x1F800000)

/* One frame of the field: clears the ordering tables; sets the floor tune's
   volume by the distance to the floor's entry and rings the clock's alarm;
   sorts the HUD (compass, moon, clock and window), the lift's digits and
   boxes and the minimap; then walks the 11 by 11 tiles around the party,
   culling each by depth and screen x and sorting its models into its own
   ordering table. Then it waits out the frame, ticks the clock, uploads the
   window, reads the pad and draws. */
void FieldFrame(void)
{
    MATRIX  lw;
    MATRIX  ls;
    int     v[2];
    SVECTOR sv;
    long    sxy;
    long    p;
    long    flag;
    int     i;
    int     j;
    int     a;
    int     b;
    int     l;
    int     r;
    int     cell;
    int     otz;
    short   sx;
    int     n;
    int     shift;
    int     m;
    int     last;
    int     k;
    int     off;
    int     pt;
    int     d0;
    int     d1;
    int     flags;
    u_int   kind;
    int     o;
    int     first;
    int     rot;
    GsDOBJ2 *obj;

    BUF = GsGetActiveBuff();
    GsSetRefView2(&g_dng->view);
    GsSetWorkBase((PACKET *)g_scene->packets[BUF]);
    GsClearOt(0, 0, &g_scene->world_ot[BUF]);
    GsClearOt(0, 0x1FE, &g_scene->ot_b[BUF]);
    GsClearOt(0, 0x1FE, &g_scene->ot_a[BUF]);
    GsClearOt(0, 0, &g_scene->ot[BUF]);
    GsClearOt(0, 0, &g_scene->ot_c[BUF]);

    if (g_entry_pos[0] == 0xFF && g_dng->map != 0x1B && g_dng->map != 0x1E) {
        goto clock_done;
    }
    if (g_dng->map != 0x1B && g_dng->map != 0x1E) {
        int c = g_dng->facing & 1;

        d0 = abs(g_entry_pos[c * 2] - g_dng->pos[c * 2]);
        d1 = abs(g_dng->pos[(c ^ 1) * 2] - g_entry_pos[(c ^ 1) * 2]);
        l = r = 127.0 - SquareRoot0(d0 * d0 + d1 * d1) * 3.9;
        if (g_map_music[g_dng->map][0] == 1 && !D_800A02E0) {
            SsSeqSetVol(g_floor_tune_a, l, l);
        }
        if (g_map_music[g_dng->map][1] == 1 && !D_800A02E0) {
            if (!g_clock_on) {
                goto clock_done;
            }
            if (g_clock_frame | (g_clock_hours | g_clock_min | g_clock_sec)) {
                SsSeqSetVol(g_floor_tune_b, l, r);
            }
        }
    }
    if (!g_clock_on) {
        goto clock_done;
    }
    /* On the limit's minute, and at the half hour off it, the tune rings. */
    if (g_clock_limit_min[g_dng->map] == (u_char)g_clock_min ||
        abs(g_clock_limit_min[g_dng->map] - (u_char)g_clock_min) == 30) {
        if ((u_char)g_clock_sec + (u_char)g_clock_frame != 0) {
            D_8009FAD0 = 0;
            goto clock_done;
        }
        if (D_8009FAD0) {
            goto clock_done;
        }
        if (g_floor_tune_a != -1) {
            if (!D_800A02E0) {
                if (g_dng->map == 0x1B || g_dng->map == 0x1E) {
                    SsSeqSetVol(g_floor_tune_a, 0x7F, 0x7F);
                } else {
                    SsSeqSetVol(g_floor_tune_a, l, r);
                }
            }
            if ((u_char)g_clock_hours < g_clock_limit_hours[g_dng->map] ||
                (u_char)g_clock_min < g_clock_limit_min[g_dng->map]) {
                SsPlayBack(g_floor_tune_a, 0,
                           g_clock_limit_hours[g_dng->map] * 2 +
                               (u_char)(g_clock_limit_min[g_dng->map] / 30) -
                               ((u_char)((u_char)g_clock_min / 30) + (u_char)g_clock_hours * 2));
            }
        }
        D_8009FAD0 = 1;
    } else {
        D_8009FAD0 = 0;
    }
clock_done:

    if (g_clock_freeze) {
        GsSortFastBg(&g_scene->layers[5].bg, &g_scene->world_ot[BUF], 0);
        GsSortFastBg(&g_scene->layers[4].bg, &g_scene->world_ot[BUF], 0);
    }
    if ((u_char)(g_field_frames % 12) == 0) {
        D_800A059C = (D_800A059C + 1) & 0xF;
        g_scene->sprites[69].u = (u_char)(D_800A059C % 10) * 24;
        g_scene->sprites[69].v = (u_char)(D_800A059C / 10) * 24 - 0x70;
    }

    if ((!g_field_hold || (g_field_hold & 0x80)) && !g_clock_freeze) {
        for (i = 0; i < 5; i++) {
            int s = i + 5;

            GsSortFastSprite(&g_scene->sprites[g_dng->facing * 5 + s], &g_scene->ot[BUF], 0);
        }
        for (i = 0; i < 4; i++) {
            int s = i + 0x19;

            GsSortFastSprite(&g_scene->sprites[g_moon_group[MOON_PHASE] * 4 + s], &g_scene->ot[BUF], 0);
        }
        for (i = 0; i < 4; i++) {
            GsSortFastSprite(&g_scene->sprites[0x3D + i], &g_scene->ot[BUF], 0);
        }
        for (i = 0x41; i < 0x46; i++) {
            if (MOON_PHASE != 8 || i != 0x44) {
                GsSortFastSprite(&g_scene->sprites[i], &g_scene->ot[BUF], 0);
            }
        }
    }

    if (D_8009FDEC) {
        if (D_8009FE3C) {
            for (i = 0; i < D_8009FE3C; i++) {
                GsSortFastSprite(&g_scene->sprites[0x5A + i], &g_scene->ot_c[BUF], 0);
            }
        }
        GsSortFastBg(&g_scene->layers[3].bg, &g_scene->ot_c[BUF], 1);
        AddPrim(g_scene->ot_c[BUF].tag, &g_scene->areas[4 + BUF]);
        AddPrim(g_scene->ot_c[BUF].org, &g_scene->areas[6 + BUF]);
        GsSortOt(&g_scene->ot_c[BUF], &g_scene->world_ot[BUF]);
        GsSortFastBg(&g_scene->layers[1].bg, &g_scene->world_ot[BUF], 0);
        GsSortFastBg(&g_scene->layers[2].bg, &g_scene->world_ot[BUF], 0);
    }
    if (g_dng->map == 0x11 || g_dng->map == 0x15) {
        GsSortFastBg(&g_scene->layers[6].bg, &g_scene->world_ot[BUF], 0x1FF);
    }

    if (g_box_state == 1) {
        FieldSetLiftDigits(g_scene->lift_at);
        for (i = 0x59; i >= 0x46; i--) {
            GsSortFastSprite(&g_scene->sprites[i], &g_scene->ot[BUF], 0);
        }
    } else if (g_box_state == 2) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 4; i++) {
                GsSortLine(&g_scene->boxes[j][i], &g_scene->ot[BUF], 0);
            }
        }
    }

    if (!g_field_hold && !g_clock_freeze) {
        /* The minimap, scrolled to the view; north-up, the compass turns
           instead. */
        if (g_map_north_up) {
            g_scene->layers[0].bg.rotate = 0;
            g_scene->layers[0].bg.scrollx = ((g_scene->cos * 150 >> 12) + g_dng->view.vpx) / 25 - 60;
            g_scene->layers[0].bg.scrolly = -((g_scene->sin * 150 >> 12) + g_dng->view.vpz) / 25 - 60;
            g_scene->layers[0].bg.rotate = 0x168000;
            GsSortBg(&g_scene->layers[0].bg, &g_scene->ot[BUF], 1);
            g_scene->sprites[0].rotate = (0x1400 - g_dng->angle) * 360;
            rot = -0x400;
        } else {
            g_scene->sprites[0].rotate = 0;
            g_scene->layers[0].bg.my = 0x42;
            g_scene->layers[0].bg.mx = 0x42;
            g_scene->layers[0].bg.scrollx = ((g_scene->cos * 150 >> 12) + g_dng->view.vpx) / 25 - 60;
            g_scene->layers[0].bg.scrolly = -((g_scene->sin * 150 >> 12) + g_dng->view.vpz) / 25 - 60;
            g_scene->layers[0].bg.x = g_minimap_x + g_scene->layers[0].bg.mx;
            g_scene->layers[0].bg.y = g_minimap_y + g_scene->layers[0].bg.my;
            {
                int turn = (g_dng->angle - 0x400) * 360;
                int shown;

                g_scene->layers[0].bg.rotate = turn;
                shown = 0x168000;
                if (turn) {
                    shown = turn;
                }
                g_scene->layers[0].bg.rotate = shown;
            }
            GsSortBg(&g_scene->layers[0].bg, &g_scene->ot[BUF], 1);
            rot = g_dng->angle + 0x800;
        }
        for (i = 1; i < 5; i++) {
            g_scene->sprites[i].x = (rcos(rot) * 48 >> 12) + 0x5E;
            g_scene->sprites[i].y = (rsin(rot) * 48 >> 12) - 0x36;
            rot += 0x400;
        }
        for (i = 0; i < 5; i++) {
            GsSortSprite(&g_scene->sprites[i], &g_scene->ot[BUF], 0);
        }
        AddPrim(g_scene->ot[BUF].tag, &g_scene->areas[BUF]);
        AddPrim(g_scene->ot[BUF].org, &g_scene->areas[2 + BUF]);
    }
    GsSortOt(&g_scene->ot[BUF], &g_scene->world_ot[BUF]);

    a = g_dng->facing & 1;
    FieldDoorSlide();
    b = a ^ 1;
    v[b] = g_dng->pos[POS_Y] - 5;
    if (D_801F29ED & 2) {
        /* The alarm: the ambient pulses between black and red. */
        int step = g_alarm_step;
        int rise;

        /* +1 and -1 swap with one xor at either end of the range. */
        if (g_alarm_level == 0x1000 || g_alarm_level == 0) {
            step ^= 0xFFFE;
        }
        rise = (short)step << 8;
        g_alarm_level += rise;
        g_alarm_step = step;
        GsSetAmbient(0x1000, g_alarm_level, g_alarm_level);
    }
    if (g_hurt_flash & 0x7F) {
        if ((u_char)(g_hurt_flash - 3) >= 2) {
            if (g_hurt_flash & 1) {
                GsSetAmbient(0x1000, 0x1000, 0x1000);
            } else {
                GsSetAmbient(0x1000, 0, 0);
            }
        }
        g_hurt_flash--;
    }
    if (g_bob_tmd_a != -1) {
        if (g_bob_tick++ & 1) {
            FieldBobVerts(0, 0, 0x14);
            FieldBobVerts(1, 4, 0x18);
            FieldBobVerts(2, 8, 0x1E);
            FieldBobVerts(3, 10, 0x10);
        }
        FieldFadePrims((u_char *)(g_pack_tmd_tab[g_bob_tmd_a] + PACK_BASE + 0x2C));
        FieldFadePrims((u_char *)(g_pack_tmd_tab[g_bob_tmd_b] + PACK_BASE + 0x2C));
    }

    for (i = 0; i < 11; i++, v[b]++) {
        if ((u_int)v[b] >= FLOOR_W) {
            continue;
        }
        v[a] = g_dng->pos[POS_X] - 5;
        for (j = 0; j < 11; j++, v[a]++) {
            if ((u_int)v[a] >= FLOOR_W || g_floor_grid[v[b]][v[a]] == 0) {
                continue;
            }
            n = (v[b] % 11 * 11 + v[a] % 11) * 8;
            cell = i * 11 + j;
            FieldClockHands(g_tile_defs[g_floor_grid[v[b]][v[a]]].flags, n, j, i);
            GsSetLsMatrix(&D_8005E6F0);
            sv.vx = v[a] * 300 + (g_scene->cos * 150 >> 12);
            sv.vy = -150;
            sv.vz = -v[b] * 300 + (g_scene->sin * 150 >> 12);
            otz = RotTransPers(&sv, &sxy, &p, &flag);
            sx = *(short *)&sxy;
            if (otz >= 450) {
                continue;
            }
            k = (otz - 5) / 75;
            if (flag < 0) {
                continue;
            }
            if (sx <= -160 - g_view_margin[k] || sx >= g_view_margin[k] + 160) {
                continue;
            }
            off = otz - 105;
            pt = (otz & ~7) + (sx < 0 ? -sx : sx) / g_ot_xdiv[k];
            if (off < 0) {
                off = 0;
            }
            GsClearOt(off, pt, &g_scene->cell_ot[BUF][cell]);
            shift = 27 - Lzc(otz + 12);
            if (shift < 0) {
                shift = 0;
            }

            /* Stairs show only their upper or lower half while the party
               is changing floor on them. */
            flags = GRID_TILE.flags;
            if ((flags & TILE_SPECIAL) && ((kind = flags & TILE_KIND) < 2 || kind == 2 || kind == 3)) {
                if (g_stair_floor == 1) {
                    first = 0;
                    last = 4;
                } else if (g_stair_floor == 2) {
                    first = 4;
                    last = 8;
                } else if (kind == 2) {
                    first = 0;
                    last = 4;
                } else if (kind == 3) {
                    first = 4;
                    last = 8;
                } else {
                    first = 0;
                    last = 8;
                }
            } else if (flags == 0x8001 && !g_stair_floor) {
                first = 0;
                last = 4;
            } else {
                first = 0;
                last = 8;
            }
            for (m = first; m < last; m++) {
                if (GRID_TILE.models[m] == 0) {
                    continue;
                }
                o = n + m;
                if (D_800A02E0 && o >= g_lift_objs && o < g_lift_objs + 8 &&
                    o != g_lift_objs + 2 && o != g_lift_objs + 3) {
                    continue;
                }
                obj = &g_scene->objs[o];
                GsGetLws(obj->coord2, &lw, &ls);
                GsSetLightMatrix2(&lw);
                GsSetLsMatrix(&ls);
                obj->attribute &= ~0xE00;
                if (otz < 0x4C) {
                    obj->attribute |= MODEL_DEF.attr & 0xE00;
                } else if (otz < 0x97) {
                    if ((u_short)(sx + 0x50) <= 0xA0) {
                        obj->attribute |= MODEL_DEF.attr & 0xE00;
                    } else if ((u_short)(sx + 0xA0) <= 0x140) {
                        obj->attribute |= MODEL_DEF.attr_far & 0xE00;
                    }
                } else if (otz < 0xE2 && (u_short)(sx + 0x50) <= 0xA0) {
                    obj->attribute |= MODEL_DEF.attr_far & 0xE00;
                }
                if (m == 6) {
                    GsSortObject4(obj, &g_scene->ot_b[BUF], 13, SCRATCH);
                } else if (m == 7) {
                    GsSortObject4(obj, &g_scene->ot_a[BUF], 13, SCRATCH);
                } else {
                    GsSortObject4(obj, &g_scene->cell_ot[BUF][cell], shift, SCRATCH);
                }
            }
            GsSortOt(&g_scene->cell_ot[BUF][cell], &g_scene->world_ot[BUF]);
        }
    }
    GsSortOt(&g_scene->ot_b[BUF], &g_scene->world_ot[BUF]);
    GsSortOt(&g_scene->ot_a[BUF], &g_scene->world_ot[BUF]);

    DrawSync(0);
    VSync(3);
    FieldClockTick(3);
    rand();
    if (g_scene->win.kind == 1) {
        LoadImage((RECT *)&g_scene->win, g_scene->win.src);
        g_scene->win.kind = 0;
    } else if (g_scene->win.kind == 2) {
        ClearImage((RECT *)&g_scene->win, g_scene->win.src[0], g_scene->win.src[1],
                   g_scene->win.src[2]);
        g_scene->win.kind = 0;
    }
    PAD_PREV = g_scene->pad_held;
    g_scene->pad_held = PadRead(1);
    g_scene->pad_new = (g_scene->pad_held & PAD_PREV) ^ g_scene->pad_held;
    if (g_field_lit) {
        GsSwapDispBuff();
    } else {
        SetDispMask(0);
    }
    GsSortClear(g_clear_r, g_clear_g, g_clear_b, &g_scene->world_ot[BUF]);
    GsDrawOt(&g_scene->world_ot[BUF]);
}
