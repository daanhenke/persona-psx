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
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
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

/* Not matched: 98.62% in ADV and DNG, src and cell trade s0/s1.
   The head now schedules as the image's: sched1 fills backwards and only
   boosts an insn that sets a register set once, so the table address has to
   go through `cells`, set once, with the loop stepping its own `cell`; and
   `cells` is set after `src` so it wins the boosted tie by position. What is
   left is global-alloc priority (refs * log2 refs / live): cell 8 refs over
   13 insns outranks src's 9 over 16 and takes s0. Moving cell's set before
   bzero fixes the registers but lets cse fold cells away, and the head goes
   back to the old order; splitting src's sum loads it straight into s0 but
   moves the base; do-while weighting changes nothing. */
#ifdef NON_MATCHING
void MapDrawName(short map)
{
    u_char *src;
    u_char *cell;
    u_char *second;
    u_char *cells;
    int     i;

    src  = (u_char *)(*(int *)(MAP_NAMES_AT + MAP_NAMES_TABLE + map * 4) +
                      MAP_NAMES_AT);
    cells = g_map_name_cells;
    bzero(cells, 0x22);
    i      = 0;
    cell   = cells;
    second = cells + 1;
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
#endif
