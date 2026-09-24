/* Persona 1 (JP) - finding the floor's entry and loading its pictures.
 * DNG only.
 *   0x8006DC7C FieldFindEntry
 *   0x8006DD68 FieldLoadGfx
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* The flag bits that say what a tile is, and the kind the floor is entered
   on. The tile definitions end with a definition whose flags are all set. */
#define TILE_WHAT        0x7E1F
#define TILE_KIND_ENTRY  1
#define TILE_DEFS_END    0xFFFF

/* Finds the first tile of the entry kind on the floor and notes where it
   is. Returns 1 if there is one. The music is marked as started from
   nowhere either way. */
int FieldFindEntry(void)
{
    int x, y, t;

    g_music_y = g_music_x = 0xFE;
    g_entry_pos[0] = g_entry_pos[1] = 0xFF;
    for (t = 0; g_tile_defs[t].flags != TILE_DEFS_END; t++) {
        if ((g_tile_defs[t].flags & TILE_WHAT) == TILE_KIND_ENTRY) {
            for (y = 0; y < FLOOR_W; y++) {
                for (x = 0; x < FLOOR_W; x++) {
                    if (g_floor_grid[y][x] == t) {
                        g_entry_pos[0] = x;
                        g_entry_pos[2] = y;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* The floor's textures: the four every floor has, a fifth on the maps that
   need it, the rest of the pack's pictures side by side from x 0x180, and
   the image rows. The map test is an || chain - as a switch it becomes a
   jump table - and the column's x is worked out from the index, which
   loop.c turns into the register the image steps. */
void FieldLoadGfx(void)
{
    int i;

    TimLoad((u_long *)(PACK_BASE + g_pack_tims[0]), 0);
    TimLoad((u_long *)(PACK_BASE + g_pack_tims[1]), 0);
    TimLoad((u_long *)(PACK_BASE + g_pack_tims[2]), 0);
    TimLoad((u_long *)(PACK_BASE + g_pack_tims[3]), 0);
    DrawSync(0);
    if (g_dng->map == 2 || g_dng->map == 8 || g_dng->map == 9 || g_dng->map == 15 ||
        g_dng->map == 20) {
        TimLoad((u_long *)(PACK_BASE + g_pack_tims[4]), 1);
        DrawSync(0);
    }
    for (i = 5; i < PACK_INDEX[g_pack_sel + 6]; i++) {
        TimLoadAt((u_long *)(PACK_BASE + g_pack_tims[i]), (i - 5) * 0x80 + 0x180, 0);
        DrawSync(0);
    }
    UploadImageRows((void *)(PACK_BASE + g_pack_images[0]), 0, 0x1E1,
                    PACK_INDEX[g_pack_sel]);
    DrawSync(0);
}
