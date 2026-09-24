/* Persona 1 (JP) - setting the field up.  DNG only.
 *   0x8006CF40 FieldOpenDoor
 *   0x8006CFD0 FieldInitGraph
 *   0x8006D098 FieldSetFloor
 *   0x8006D300 FieldEnterFrom
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/state.h>
#include <persona/dng/field.h>

/* Starts the door the party has stepped up to sliding open: its halves move
   DOOR_SLIDE a frame for DOOR_FRAMES frames, one each way. Returns int with
   nothing returned but the no-clip case. */
#define DOOR_SLIDE  15
#define DOOR_FRAMES 10

int FieldOpenDoor(void)
{
    if (g_noclip) {
        return 0;
    }
    if ((g_tile_flags & TILE_SPECIAL) && g_tile_kind == TILE_KIND_DOOR) {
        g_door_state = 1;
        g_door_dx = -DOOR_SLIDE;
        g_door_dy = DOOR_SLIDE;
        g_door_frames = DOOR_FRAMES;
        func_80070BF0(0x15, 1);
    }
}

/* The field's display: 320 by 240 interlaced, 3D on, VRAM cleared and the
   debug font loaded beside the map's texture page. */
void FieldInitGraph(void)
{
    RECT r;

    ResetGraph(1);
    GsInitGraph(320, 240, 4, 0, 0);
    GsDefDispBuff(0, 0, 0, 240);
    GsInit3D();
    r.x = 0;
    r.y = 0;
    r.w = 0x3FF;
    r.h = 0x1FF;
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    FntLoad(0x3C0, 0x100);
    SetDumpFnt(FntOpen(-0x98, 0, 0, 0, 2, 900));
    DrawSync(0);
}

/* Points the floor's tables at the current floor's entries in the pack and
   its index, clears what a new floor starts without, and faces the view the
   way the party faces. Both clocks' frame counts are rounded up to a
   multiple of three. */
/* 95.0%: the order is the image's statement for statement, but sched puts
   the division's constant and the D_8009FDEC clear one slot early, and
   keeps g_scene in a0 across the clock store only in the image. */
#ifdef NON_MATCHING
void FieldSetFloor(void)
{
    g_model_defs = (u_char *)(PACK_BASE + *g_pack_model_tab);
    g_tile_defs = (TileDef *)(INDEX_BASE + *g_index_tile_tab);
    g_floor_info = (u_char *)(INDEX_BASE + g_index_info_tab[DNG_FLOOR]);
    g_floor_grid = (void *)(INDEX_BASE + g_index_grid_tab[DNG_FLOOR]);
    g_floor_objs = (u_char *)(PACK_BASE + g_pack_obj_tab[DNG_FLOOR]);
    g_floor_spots = (u_char *)(PACK_BASE + g_pack_spot_tab[DNG_FLOOR]);
    D_8009FDEC = 0;
    g_floor_events = (u_char *)(PACK_BASE + g_pack_event_tab[DNG_FLOOR]);
    g_scene->flag6EA04 = 0;
    D_800993B8 = 0;
    g_bgm_off = 0;
    D_8009FE3C = 0;
    D_8009FAD0 = 0;
    g_playtime_frame = (g_playtime_frame + 2) / 3 * 3;
    g_clock_frame = ((u_char)g_clock_frame + 2) / 3 * 3;
    D_800A059C = 0;
    g_field_frames = 0;
    g_scene->from_y = 0;
    g_scene->from_x = 0;
    g_dng->angle = (g_dng->facing ^ 1) << 10;
    g_field_hold = 0;
    D_800A02E0 = 0;
    g_scene->sin = rsin(g_dng->angle);
    g_scene->cos = rcos(g_dng->angle);
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldinit", FieldSetFloor);
#endif

/* Coming back from the 2D map or an ADV scene clears the state's 0x15B0
   byte; coming back from a battle leaves it. The battle's empty case is what
   puts the lower bound test in the image's case tree, and the two others
   are separate arms - stacked, they fold into one range test. */
void FieldEnterFrom(void)
{
    switch (g_state_prev) {
    case GAME_STATE_BTL:
        break;
    case GAME_STATE_S2D:
        g_dng->flag15B0 = 0;
        break;
    case GAME_STATE_ADV:
        g_dng->flag15B0 = 0;
        break;
    }
}
