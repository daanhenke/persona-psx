/* Persona 1 (JP) - the field's entry point: loading a floor and walking it.
 * DNG only.
 *   0x80064D2C ovl_dng_entry
 *   0x80065978 func_80065978
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <stdlib.h>
#include <libsnd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
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
        func_80065978();
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
                    func_80065978();
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
        if (D_800993C6 != 0x80) {
            for (i = 0; i < 5; i++) {
                m = g_party[i];
                if (m == 0xFF) {
                    continue;
                }
                c = &g_chars[m];
                if (c->status == STATUS_SICK && !(g_dng->tick_flags & 3)) {
                    FieldPlaySeq(10, 2);
                    D_800993C6 = 6;
                    if ((c->hp -= c->hp_max / 8) <= 0) {
                        c->hp = 1;
                    }
                } else if (c->status == STATUS_POISON && !(g_dng->tick_flags & 1)) {
                    FieldPlaySeq(10, 1);
                    D_800993C6 = 2;
                    if (c->hp != 1) {
                        c->hp--;
                    }
                }
            }
        } else {
            D_800993C6 = 0;
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

INCLUDE_ASM("dng/nonmatchings/field/fieldmain", func_80065978);
