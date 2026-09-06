/* Persona 1 (JP) - the automap screen's name banner.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   ADV 0x80095B88   DNG 0x80096504   S2D 0x80086998
 *
 * The banner is ten 16-pixel background cells across the top of the map
 * screen, drawn as a background layer of its own rather than into the
 * character map. Its cells come out of the pack read to 0x801DD000: the entry
 * table starts at +8 and each entry is a byte offset to twenty bytes, ten
 * pairs, one pair a cell.
 *
 * The pairs are stored the other way round from how a background map wants
 * them, so each is swapped on the way in and the byte that ends up first
 * carries 0x80. A 0xFF 0x01 pair closes the map off.
 */
#include <types.h>
#include <libgs.h>

/* The pack the cells come from, and where its entry table starts. */
#define MAP_NAMES_AT    0x801DD000
#define MAP_NAMES_TABLE 8

/* Ten cells, so twenty bytes and a terminator after them. */
#define NAME_CELLS 10
#define NAME_END   (NAME_CELLS * 2)

/* The banner's layer, and where it sits. */
#define NAME_LAYER 4
#define NAME_BIT   0x10
#define NAME_X     0x5A
#define NAME_Y     0x12
#define NAME_W     0xA0
#define NAME_H     0x10

extern GsBG   g_bg_layers[];
extern u_long g_bg_shown;
extern u_char g_map_name_cells[];

extern void bzero(void *dst, int len);
extern void BgMapInit(u_char *map, int arg);

void MapDrawName(short map)
{
    u_char *cell;
    u_char *src;
    u_char *second;
    int     i;

    src  = (u_char *)(*(int *)(MAP_NAMES_AT + MAP_NAMES_TABLE + map * 4) +
                      MAP_NAMES_AT);
    bzero(g_map_name_cells, 0x22);
    i      = 0;
    cell   = g_map_name_cells;
    second = g_map_name_cells + 1;
    do {
        i++;
        *second = src[0];
        second += 2;
        *cell = src[1] | 0x80;
        src += 2;
        cell += 2;
    } while (i < NAME_CELLS);

    g_map_name_cells[NAME_END]     = 0xFF;
    g_map_name_cells[NAME_END + 1] = 1;
    BgMapInit(g_map_name_cells, 0);

    g_bg_layers[NAME_LAYER].x = NAME_X;
    g_bg_layers[NAME_LAYER].y = NAME_Y;
    g_bg_layers[NAME_LAYER].w = NAME_W;
    g_bg_layers[NAME_LAYER].h = NAME_H;
    g_bg_shown |= NAME_BIT;
}
