/* Persona 1 (JP) - drawing the effect slots.  BTLP only.
 *   0x80079220 BtlDrawEffects
 *
 * Once a frame, with the ordering table the rest of the 2D layer goes into.
 * Every slot has a two-entry ordering table of its own for each of the two
 * display buffers, cleared at the top of the pass and left in g_btl_effect_ot
 * for the drawers.
 *
 * A slot whose record is being drawn has its offset and cursor settled, then
 * its motion handler run; a motion that has finished takes the running bit
 * away and frees the slot, though the record is still drawn this once. The
 * record's own transform goes into the GTE, its text is put up, and - while
 * it has them - its rows are drawn at eight-pixel steps from the shift, and
 * its frame. The record's texture mode goes in last, so it is the first thing
 * the slot's table draws.
 *
 * The slots' tables are then linked into the caller's: the one the pad is
 * talking to first, and every other after it, so the others are drawn
 * beneath it. The page flips on the way out.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/effect.h>

void BtlDrawEffects(u_long *ot)
{
    MATRIX        m;
    long          ofx;
    long          ofy;
    BtlEffect    *e;
    BtlEffectRow *row;
    int           i;
    u_short       flags;

    ReadGeomOffset(&ofx, &ofy);
    for (i = 0; i < BTL_EFFECT_SLOTS; i++) {
        g_btl_effect_ot = g_btl_effect_ots[g_btl_effect_page][i];
        ClearOTag(g_btl_effect_ot, BTL_EFFECT_OT_LEN);
        if (g_btl_effect[i] == (BtlEffect *)BTL_EFFECT_FREE) {
            continue;
        }
        e = g_btl_effect[i];
        if (((short)e->flags & BTL_EFFECT_DRAWN) == 0) {
            continue;
        }
        BtlEffectOffset(e);
        BtlEffectMoveCursor(i);
        if (g_btl_effect_motions[e->kind & 0xF](e) == 0) {
            e->flags &= ~BTL_EFFECT_RUNNING;
            g_btl_effect[i] = (BtlEffect *)BTL_EFFECT_FREE;
        }
        SetGeomOffset(e->curx, e->cury);
        RotMatrix((SVECTOR *)&e->unk30, &m);
        TransMatrix(&m, (VECTOR *)&e->unk20);
        ScaleMatrix(&m, (VECTOR *)&e->scale_x);
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        BtlDrawEffectText((u_char *)e);
        if ((e->flags & BTL_EFFECT_ROWS) != 0) {
            row = (BtlEffectRow *)e;
            while (row->next != (BtlEffectRow *)BTL_EFFECT_FREE) {
                row = row->next;
                g_btl_glyph_x = g_btl_effect_ox + row->x * 8;
                g_btl_glyph_y = g_btl_effect_oy + row->y * 8;
                g_btl_effect_drawers[(row->kind & 0xF) - 1](row);
            }
        }
        flags = e->flags;
        if ((flags & BTL_EFFECT_FRAMED) != 0) {
            g_btl_effect_frames[(flags >> 9) & 3](e);
        }
        addPrim(g_btl_effect_ot, &((DR_MODE *)e->mode)[g_btl_effect_page]);
    }

    for (i = 0; i < BTL_EFFECT_SLOTS; i++) {
        if (i == g_btl_effect_cur && g_btl_effect[i] != (BtlEffect *)BTL_EFFECT_FREE) {
            g_btl_effect_ot = g_btl_effect_ots[g_btl_effect_page][i];
            AddPrims(ot, g_btl_effect_ot, g_btl_effect_ot + 1);
        }
    }
    for (i = 0; i < BTL_EFFECT_SLOTS; i++) {
        if (i != g_btl_effect_cur && g_btl_effect[i] != (BtlEffect *)BTL_EFFECT_FREE) {
            g_btl_effect_ot = g_btl_effect_ots[g_btl_effect_page][i];
            AddPrims(ot, g_btl_effect_ot, g_btl_effect_ot + 1);
        }
    }
    SetGeomOffset(ofx, ofy);
    g_btl_effect_page ^= 1;
}
