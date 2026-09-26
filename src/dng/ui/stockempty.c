/* Persona 1 (JP) - the empty-stock notice.  DNG only.
 *   0x8008585C StockEmptyNotice
 *
 * Opening the Persona stock with nothing in it puts a message up in the
 * window layer, frees the stock screen's sprites, and waits for a button
 * before the menu goes back. ADV carries the same routine (0x80076CE0), still
 * in asm.
 */
#define SLOT_TAGGED_INTXY
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/slot.h>
#include <persona/common/bg.h>

/* The message, and the sprite that frames it. */
extern u_char g_stock_empty_msg[];
extern u_char D_8009ABFC[];
extern int    g_pad_pressed[];

extern void BgMapInit(void *script, short speed);
extern void RunFrame(void);
/* The field's message stepper. */
extern int  MsgStep(void);

void StockEmptyNotice(void)
{
    BgMapInit(g_stock_empty_msg, 0);
    g_bg_layers[4].x = 0x38;
    g_bg_layers[4].y = 0x7A;
    g_bg_layers[4].w = 0xF0;
    g_bg_layers[4].h = 0x10;
    g_bg_shown |= 0x10;
    SlotInitTagged(D_8009ABFC, 0x2E, 0x24, 0x36, 0x78);
    SlotClear(1);
    SlotClear(2);
    SlotClear(3);
    SlotClear(8);
    SlotClear(9);
    SlotClear(0xA);
    SlotClear(0xB);
    SlotClear(0xC);
    SlotClear(0xD);
    goto wait;
    do {
        MsgStep();
    wait:
        RunFrame();
    } while (g_pad_pressed[0] == 0);
    RunFrame();
}
