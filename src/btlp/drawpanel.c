/* Persona 1 (JP) - putting the pop-up panel in the frame.  BTLP only.
 *   0x80075B78 BtlDrawPanel
 *
 * One frame of the panel: run whichever of the two animations the state names,
 * settle the state on the answer, and if anything is left on screen, place it
 * and hand its prims to the ordering table.
 *
 * The state machine has no case of its own for a panel that is simply open -
 * it falls straight through to the drawing - and both a finished close and any
 * state the switch does not know put it back to shut. That is what keeps a
 * stray value from leaving the panel up.
 *
 * The body of the panel is drawn two ways. While it is still scaling it goes
 * out as the transformed quad; once it has reached full size the flat sprite
 * is used instead, copied whole out of the one template so both buffers can
 * carry their own.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/panel.h>

extern void BtlHighlightDraw(int buf, u_long *ot);

void BtlDrawPanel(int buf, u_long *ot)
{
    SPRT *flat;
    int   i;

    switch (g_btl_panel_state) {
    case PANEL_OPEN:
        break;
    case PANEL_OPENING:
        if (BtlPanelStepOpen() == 0) {
            g_btl_panel_state = PANEL_OPEN;
        }
        break;
    case PANEL_CLOSING:
        if (BtlPanelStepClose() != 0) {
            break;
        }
        /* fall through: a close that has run out is a shut panel */
    default:
        g_btl_panel_state = PANEL_SHUT;
        break;
    }

    if (g_btl_panel_state != 0) {
        i = 0;
        BtlPlacePanel();
        BtlDrawPanelBox(buf);
        BtlHighlightDraw(buf, ot);
        do {
            AddPrim(ot, &g_btl_panel_corner[buf][i]);
            i++;
        } while (i < BTL_PANEL_CORNERS);

        if (g_btl_panel_scale.vx < PANEL_FULL) {
            AddPrim(ot, &g_btl_panel_poly[buf]);
        } else {
            flat = &g_btl_panel_flat[buf];
            *flat = g_btl_panel_sprite;
            AddPrim(ot, flat);
        }
        AddPrim(ot, &g_btl_panel_mode[buf]);
    }
}
