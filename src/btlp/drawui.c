/* Persona 1 (JP) - the battle's whole 2D layer, once a frame.  BTLP only.
 *   0x8007A024 BtlDrawUi
 *
 * The layer is built into an ordering table of its own and linked into the
 * caller's at the end, so everything below can be drawn in one pass and put in
 * front of the 3D in one link. There are two of those tables and two halves of
 * the primitive buffer, one of each per frame, and this is what flips between
 * them; the glyph allocator is reset at the same time.
 *
 * What the frame cost of the primitive buffer is left in g_btl_prim_used, and
 * the largest ever seen in g_btl_prim_peak - a watch on the one buffer
 * everything draws into.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/input.h>
#include <persona/btlp/text.h>

/* The primitive buffer, and how much of it one frame gets. */
#define BTL_PRIM_POOL 0x801AC200
#define BTL_PRIM_HALF 0x2800

/* Three entries: the two the drawers add to and the tail AddPrims links from. */
#define BTL_UI_OT 3

extern int    g_btl_glyph_next;
extern int    g_btl_ot_index;
extern char  *g_btl_prim_next;
extern u_long g_btl_ui_ot[][BTL_UI_OT];
extern int    g_btl_prim_used;
extern int    g_btl_prim_peak;

extern void BtlSeqAdvance(void);
extern void BtlHudTick(void);
extern void BtlBoxTick(void);
extern void BtlMenuDraw(void);
extern void BtlSeqWindowDraw(void);
extern void BtlTextWindowDraw(void);
extern void BtlTalkDrawPanel(void);
extern void BtlHudDraw(void);
extern void BtlBoxDraw(void);
extern void BtlDrawEffects(u_long *ot);

void BtlDrawUi(u_long *ot)
{
    int used;
    int peak;
    /* Forty bytes of locals the code no longer uses; the frame is that much
       bigger than what is left needs. */
    long spare[10];

    g_btl_glyph_next = 0;
    g_btl_ot_index ^= 1;
    g_btl_prim_next = (char *)BTL_PRIM_POOL + g_btl_ot_index * BTL_PRIM_HALF;
    ClearOTag(g_btl_ui_ot[g_btl_ot_index], BTL_UI_OT);

    BtlSeqAdvance();
    BtlTextAdvance();
    BtlHudTick();
    BtlBoxTick();
    BtlCursorDraw();
    BtlMenuDraw();
    BtlSeqWindowDraw();
    BtlTextWindowDraw();
    BtlTalkDrawPanel();
    BtlHudDraw();
    BtlBoxDraw();
    BtlDrawEffects(g_btl_ui_ot[g_btl_ot_index]);

    AddPrims(ot, &g_btl_ui_ot[g_btl_ot_index][0], &g_btl_ui_ot[g_btl_ot_index][2]);
    peak = g_btl_prim_peak;
    used = g_btl_prim_next - ((char *)BTL_PRIM_POOL + g_btl_ot_index * BTL_PRIM_HALF);
    g_btl_prim_used = used;
    if (peak < used) {
        peak = used;
    }
    g_btl_prim_peak = peak;
}
