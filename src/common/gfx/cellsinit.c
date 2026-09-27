/* Persona 1 (JP) - the menu screens' cell tables.
 *
 * Compiled into ADV and DNG rather than called across the boundary:
 *                  DNG         ADV
 *   TileMapCapRow  0x80091CB4  0x8008DBE4
 *   CellsInit      0x80091CD4  0x8008DC04
 *
 * The character maps draw through GsCELL tables in the work area; this fills
 * them. The first is the text atlas once per CLUT bank - 31 glyphs a row of
 * 8x12 cells, four banks - and the rest are the menus' fixed pieces: frames,
 * bars, marks and icons, each a strip across the texture page with one CLUT.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define CELLS(addr) ((GsCELL *)((addr) + WORK_BIAS))

#define TEXT_CELLS  0xD7
#define TEXT_ATLAS  ((GsCELL (*)[TEXT_CELLS])CELLS(0x800E3E4C))
#define TEXT_BANKS  4
#define TEXT_COLS   31
#define TEXT_TPAGE  0x1E

/* The icon strip's CLUT row, one per cell. */
extern u_char D_800B9278[];

/* Puts the frame's end piece at both ends of a row `w` cells wide. */
void TileMapCapRow(short *dst, u_char w)
{
    *dst = 0x3A0;
    dst += w - 1;
    *dst = 0x3A0;
}

/* 98.6%: in the image loop.c leaves the atlas base inside the bank loop
   (it is rebuilt every pass) and keeps only i * 8 outside; this build
   hoists the base with it. Every other loop matches. The RTL shows why:
   &TEXT_ATLAS[j][i] expands as (i * 8 + base) + j * 1720, and loop.c moves
   the i * 8 + base sum; the image adds i * 8 to (base + j * 1720). A row
   pointer (TEXT_ATLAS[j] + i) gets that association, but then loop.c moves
   the base and leaves i * 8 inside. Short col/row locals, an inline cell
   setter and byte offsets all score lower. */
#ifdef NON_MATCHING
void CellsInit(void)
{
    short   i;
    short   j;
    GsCELL *c;

    for (i = 0; i < TEXT_CELLS; i++) {
        for (j = 0; j < TEXT_BANKS; j++) {
            c = &TEXT_ATLAS[j][i];
            c->u = (i % TEXT_COLS) * 8;
            c->v = (i / TEXT_COLS) * 12;
            c->cba = ((j + 0x1A0) << 6) + 0x3C;
            c->flag = 0;
            c->tpage = TEXT_TPAGE;
        }
    }
    for (i = 0; i < 6; i++) {
        c = &CELLS(0x800E592C)[i];
        c->u = i * 8;
        c->v = 0x60;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0x40; i++) {
        c = &CELLS(0x800E595C)[i];
        c->u = (i % 32) * 8;
        c->v = (i / 32) * 12 + 0x6C;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0xF; i++) {
        c = &CELLS(0x800E5B5C)[i];
        c->u = i * 8;
        c->v = 0x84;
        c->cba = 0x693C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0xE; i++) {
        c = &CELLS(0x800E5BD4)[i];
        c->u = i * 8;
        c->v = 0x9C;
        c->cba = 0x6A3C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0x19; i++) {
        c = &CELLS(0x800E5C44)[i];
        c->u = (i + 0xE) * 8;
        c->v = (short)(i / 17) * 12 - 0x64;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 4; i++) {
        c = &CELLS(0x800E5D0C)[i];
        c->u = i * 8 + 0x30;
        c->v = 0x60;
        c->cba = 0x693C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0x27; i++) {
        c = &CELLS(0x800E5D2C)[i];
        c->u = (i & 0xF) * 16;
        c->v = (i / 16) * 16;
        c->cba = ((D_800B9278[i] + 0x1B0) << 6) + 0x3C;
        c->flag = 0;
        c->tpage = 0x1C;
    }
    for (i = 0; i < 0x15; i++) {
        c = &CELLS(0x800E5E64)[i];
        c->u = (i & 0xF) * 16;
        c->v = (i / 16) * 16 - 0x40;
        c->cba = 0x7D40;
        c->flag = 0;
        c->tpage = 0x1A;
    }
    for (i = 0; i < 0x10; i++) {
        c = &CELLS(0x800E5F0C)[i];
        c->u = i * 8 + 0x50;
        c->v = 0x60;
        c->cba = 0x69BC;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0xC; i++) {
        c = &CELLS(0x800E5F8C)[i];
        c->u = i * 8 + 0x78;
        c->v = 0x84;
        c->cba = 0x69FC;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0x14; i++) {
        c = &CELLS(0x800E5FEC)[i];
        c->u = i * 8;
        c->v = 0x90;
        c->cba = 0x69FC;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0xF; i++) {
        c = &CELLS(0x800E608C)[i];
        c->u = i * 8 + 0x38;
        c->v = 0xA8;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0xA; i++) {
        c = &CELLS(0x800E6104)[i];
        c->u = i * 8 - 0x60;
        c->v = 0x90;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 9; i++) {
        c = &CELLS(0x800E6154)[i];
        c->u = i * 8 - 0x50;
        c->v = 0xA8;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 0x12; i++) {
        c = &CELLS(0x800E619C)[i];
        c->u = i * 8 + 0x50;
        c->v = 0xB4;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 2; i++) {
        c = &CELLS(0x800E622C)[i];
        c->u = i * 8 - 0x18;
        c->v = 0x9C;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
    for (i = 0; i < 4; i++) {
        c = &CELLS(0x800E623C)[i];
        c->u = i * 8 - 0x28;
        c->v = 0x84;
        c->cba = 0x683C;
        c->flag = 0;
        c->tpage = TEXT_TPAGE;
    }
}
#endif
