/* Persona 1 (JP) - putting the tiled background map back to nothing.  S2D.
 *   0x80066250 BgReset  0x80066394 MsgStep (asm)
 *
 * DNG's BgReset (src/dng/gfx/bgreset.c) over S2D's work area. S2D's
 * message interpreter is 0x14 bytes longer than DNG's and stays asm.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/bg.h>
#include <persona/common/font.h>
#include <persona/common/item.h>
#include <persona/common/menulist.h>
#include <persona/common/pad.h>
#include <persona/common/persona.h>
#define SLOT_SETPOS_INT
#include <persona/common/slot.h>
#include <persona/common/spell.h>
#include <persona/common/imageanim.h>
#include <persona/common/tilemap.h>
#include <persona/common/vram.h>

/* The texture page the cells are cut from, and the palette row under it. */
#define PAGE_X   0x3C0
#define PAGE_Y   0x100
#define PAGE_W   0x40
#define PAGE_H   0x100
#define CLUT_Y   0x1FF

/* Every cell the map can point at, plus the blank one ahead of them. */
#define BG_CELLS (BG_MAP_W * BG_MAP_H + 5)

#define CELL_CBA   0x7FFC
#define CELL_TPAGE 0x1F

/* Beside the cells in the work area, reached by hardcoded address. */
#define BG_SLOTS 64
#define g_bg_slots ((u_int *)(0x800EB14C + WORK_BIAS))

/* The palette row the map's cells draw with. */
extern u_long g_bg_clut[];

void BgReset(void)
{
    u_int  *slots;
    GsCELL *cell;
    u_char  n;
    int     q;
    /* Eight bytes of frame nothing reads, as ADV's copy has. */
    int     unused[2];

    slots = g_bg_slots;
    cell = g_bg_cells;
    VramClearRect(PAGE_X, PAGE_Y, PAGE_W, PAGE_H);
    for (n = 0; n < BG_SLOTS; n++) {
        slots[n] = 0;
    }
    for (n = 0; n < BG_CELLS; n++) {
        cell->u = 0;
        cell->v = 0;
        cell->cba = CELL_CBA;
        cell->flag = 0;
        cell->tpage = CELL_TPAGE;
        cell++;
    }

    q = g_image_queue_count;
    g_image_queue[q].data = g_bg_clut;
    g_image_queue[q].rect.x = PAGE_X;
    g_image_queue[q].rect.y = CLUT_Y;
    g_image_queue[q].rect.w = PAGE_W;
    g_image_queue[q].rect.h = 1;
    g_image_queue_count = q + 1;

    BgMapClearRow(0);
    BgMapClearRow(1);
    BgMapClearRow(2);
    BgMapClearRow(3);
}

/* ------------------------------------------------------------------------ */
/* The message interpreter; see ADV's copy for the control codes. Returns 1
 * once the message has ended.
 */

#define SCRIPT_CODE  0xFF

/* Where the glyph for the cell after `n` is drawn in the font page. */
#define GLYPH_X(n)   ((((n) + 1) & 0xF) * 4 | 0x3C0)
#define GLYPH_Y(n)   ((((n) + 1) & ~0xF) + 0x100)

#define WINDOW_CELLS 60
#define ROW_CELLS    15
#define FULL_AT      30

#define CHOICE_SLOT  0x2E
#define COUNT_ITEM   0x23

typedef struct {
    u_char *script;
    u_char  rows;
    u_char  cols;
    u_char  count;
    u_char  pad;
} MsgChoice;

typedef struct {
    u_char  unk0;
    u_char  unk1;
    u_char  pad[6];
} MsgMark;

INCLUDE_ASM("s2d/nonmatchings/gfx/bgreset", MsgStep);
