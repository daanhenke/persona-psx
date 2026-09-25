/* Persona 1 (JP) - setting the field's graphics up for a floor.  DNG only.
 *   0x8006D33C FieldSetupGfx
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

extern void FieldInitHud(void);
extern void BgFromPack(u_long *pack, u_char *hdr, GsMAP *map, GsBG *bg, short x, short y);

/* The save's message window style: it picks the window frame's cells. */
#define g_win_style (*(u_char *)0x801F2AC6)


#define PACK(tab, n) ((u_long *)(PACK_BASE + (tab)[n]))

/* The first object's vertex list of a TMD in the pack. The table entry is
   read through a cast: as a plain tab[n] the load is an aggregate element,
   and sched lifts the second one above the first store. */
#define TMD_VERTS(n) (*(SVECTOR **)(PACK_BASE + *(int *)(g_pack_tmd_tab + (n)) + 0xC))

/* Builds the field's display for the current floor (`reload` coming back to
   it): the clocks' frame counts rounded up to a multiple of three, libgs
   restarted, every ordering table pointed at its tags, the scene built and
   lit, the background layers read out of the pack - the message window's
   frame in the save's window style - the drawing areas set, the automap
   rebuilt, and the two bobbing TMDs of the maps that have them found. */
void FieldSetupGfx(int reload)
{
    RECT r;
    u_int i;
    u_int j;
    u_short *p;

    g_playtime_frame = (g_playtime_frame + 2) / 3 * 3;
    g_clock_frame = ((u_char)g_clock_frame + 2) / 3 * 3;
    D_800A059C = 0;
    g_field_frames = 0;
    ResetGraph(1);
    GsInitGraph(320, 240, 4, 0, 0);
    GsDefDispBuff(0, 0, 0, 240);
    GsInit3D();

    g_scene->models[0].tmd = (u_long *)-1;
    g_scene->pad_new = 0;
    g_scene->pad_held = 0;
    g_scene->world_ot[0].length = 9;
    g_scene->world_ot[0].org = g_scene->world_tags[0];
    g_scene->world_ot[1].length = 9;
    g_scene->world_ot[1].org = g_scene->world_tags[1];
    for (i = 0; i < CELL_OTS; i++) {
        g_scene->cell_ot[0][i].length = 5;
        g_scene->cell_ot[0][i].org = g_scene->cell_tags[0][i];
        g_scene->cell_ot[1][i].length = 5;
        g_scene->cell_ot[1][i].org = g_scene->cell_tags[1][i];
    }
    g_scene->ot_b[0].length = 1;
    g_scene->ot_b[0].org = g_scene->ot_b_tags[0];
    g_scene->ot_b[1].length = 1;
    g_scene->ot_b[1].org = g_scene->ot_b_tags[1];
    g_scene->ot_a[0].length = 1;
    g_scene->ot_a[0].org = g_scene->ot_a_tags[0];
    g_scene->ot_a[1].length = 1;
    g_scene->ot_a[1].org = g_scene->ot_a_tags[1];
    g_scene->ot[0].length = 2;
    g_scene->ot[0].org = g_scene->ot_tags[0];
    g_scene->ot[1].length = 2;
    g_scene->ot[1].org = g_scene->ot_tags[1];
    g_scene->ot_c[0].length = 2;
    g_scene->ot_c[0].org = g_scene->ot_c_tags[0];
    g_scene->ot_c[1].length = 2;
    g_scene->ot_c[1].org = g_scene->ot_c_tags[1];

    FieldBuildScene(reload);
    FieldFindEntry();
    FieldSetView(reload);
    FieldInitLight();
    FntLoad(0x3C0, 0x100);
    SetDumpFnt(FntOpen(-0x98, 0, 0, 0, 2, 900));

    BgFromPack(PACK(g_pack_msg_tab, 0), (u_char *)PACK(g_pack_cell_tab, 0),
               &g_scene->layers[0].map, &g_scene->layers[0].bg,
               g_minimap_x = 0x1C, g_minimap_y = -0x78);
    g_scene->layers[0].bg.attribute = 0x60000000;
    g_scene->layers[0].bg.w = 0x84;
    g_scene->layers[0].bg.h = 0x84;
    BgFromPack(PACK(g_pack_msg_tab, 1), (u_char *)PACK(g_pack_cell_tab, 1),
               &g_scene->layers[1].map, &g_scene->layers[1].bg, -0x94, 0x24);
    /* The message window frame's cell index, in the save's window style.
       The row counter is the ordering-table loop's i again, which is what
       keeps loop.c from folding it into the row value. */
    p = (u_short *)(PACK_BASE + 8 + g_pack_cell_tab[2]);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            *p++ = i * 16 + g_win_style * 2;
            *p++ = i * 16 + (u_short)(g_win_style * 2 + 1);
        }
        *p++ = i * 16 + g_win_style * 2;
    }
    BgFromPack(PACK(g_pack_msg_tab, 2), (u_char *)PACK(g_pack_cell_tab, 2),
               &g_scene->layers[2].map, &g_scene->layers[2].bg, -0x8C, 0x24);
    g_scene->layers[2].bg.attribute = 0x40000000;
    BgFromPack(PACK(g_pack_msg_tab, 3), (u_char *)PACK(g_pack_cell_tab, 3),
               &g_scene->layers[3].map, &g_scene->layers[3].bg, -0x80, 0x30);
    g_scene->layers[3].bg.attribute = 0;
    BgFromPack(PACK(g_pack_msg_tab, 4), (u_char *)PACK(g_pack_cell_tab, 4),
               &g_scene->layers[4].map, &g_scene->layers[4].bg, -0x50, -0x60);
    g_scene->layers[4].bg.attribute = 0;
    BgFromPack(PACK(g_pack_msg_tab, 4), (u_char *)PACK(g_pack_cell_tab, 5),
               &g_scene->layers[5].map, &g_scene->layers[5].bg, -0x48, -0x5C);
    g_scene->layers[5].bg.attribute = 0;
    if (g_dng->map == 0x11 || g_dng->map == 0x15) {
        BgFromPack(PACK(g_pack_msg_tab, 5), (u_char *)PACK(g_pack_cell_tab, 6),
                   &g_scene->layers[6].map, &g_scene->layers[6].bg, -0xA0, -0x78);
        g_scene->layers[6].bg.w = 320;
        g_scene->layers[6].bg.h = 0x50;
    }
    FieldInitHud();

    r.x = 0xD4;
    r.y = 0x108;
    r.w = 0x54;
    r.h = 0x54;
    SetDrawArea(&g_scene->areas[0], &r);
    r.y = 0x18;
    SetDrawArea(&g_scene->areas[1], &r);
    r.x = 0;
    r.y = 240;
    r.w = 320;
    r.h = 240;
    SetDrawArea(&g_scene->areas[2], &r);
    r.y = 0;
    SetDrawArea(&g_scene->areas[3], &r);
    r.x = 0x20;
    r.y = 0x198;
    r.w = 240;
    r.h = 0x30;
    SetDrawArea(&g_scene->areas[4], &r);
    r.y = 0xA8;
    SetDrawArea(&g_scene->areas[5], &r);
    r.x = 0;
    r.y = 240;
    r.w = 320;
    r.h = 240;
    SetDrawArea(&g_scene->areas[6], &r);
    r.y = 0;
    SetDrawArea(&g_scene->areas[7], &r);
    FieldRebuildMap();

    g_bob_tmd_b = -1;
    g_bob_tmd_a = -1;
    if (g_dng->map == 0x1B || g_dng->map == 0x1E) {
        g_bob_tmd_a = 0x1E;
        g_bob_tmd_b = 0x1F;
    } else if (g_dng->map == 0x16) {
        g_bob_tmd_a = 0x24;
        g_bob_tmd_b = 0x25;
    } else if (g_dng->map == 0x23) {
        g_bob_tmd_a = 0x2A;
        g_bob_tmd_b = 0x2B;
    }
    if (g_bob_tmd_a != -1) {
        g_bob_verts_a = TMD_VERTS(g_bob_tmd_a);
        g_bob_verts_b = TMD_VERTS(g_bob_tmd_b);
    }
    DrawSync(0);
}
