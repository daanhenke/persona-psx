/* Persona 1 (JP) - revealing a scene's block of the automap.  ADV @ 0x80086390.
 *
 * A scene carries its automap position already resolved, which is why
 * MapMarkTile takes a base rather than a map id: the high byte of `map_at` is
 * that base and the low byte the room. Beside it sits the rectangle the scene
 * uncovers on arrival, so walking into a room fills in the part of it the
 * player can see at once.
 *
 * A scene with no automap block carries 0xFFFF there and nothing is marked;
 * neither is a rectangle with a zero side.
 */
#include <types.h>
#include <persona/adv/scene.h>

/* What a scene without an automap block carries. */
#define MAP_NONE 0xFFFF

extern void MapMarkTile(short base, short room, short x, short y);

void MapRevealScene(void)
{
    int row;
    int col;

    if (g_adv_scene->map_at == MAP_NONE) {
        return;
    }
    row = 0;
    while (row < g_adv_scene->seen_h) {
        col = 0;
        while (col < g_adv_scene->seen_w) {
            MapMarkTile(g_adv_scene->map_at >> 8,
                        *(u_char *)&g_adv_scene->map_at,
                        g_adv_scene->seen_x + col,
                        row + g_adv_scene->seen_y);
            col++;
        }
        row++;
    }
}
