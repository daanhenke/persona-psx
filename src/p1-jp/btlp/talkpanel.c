/* Persona 1 (JP) - the bar the negotiation's text sits on.  BTLP only.
 *   0x8007BAE4 BtlTalkDrawPanel
 *
 * A single wide sprite across the lower screen, drawn only while a
 * negotiation is running. Both it and the draw mode that selects its texture
 * page are built fresh on the stack every frame and copied into the primitive
 * buffer, rather than being kept and refilled the way the HUD's bar is - which
 * is why nothing here has to be set up in advance.
 */
#include <types.h>
#include <libc.h>
#include <libgpu.h>

/* Where the panel sits and how big it is. */
#define TALK_PANEL_X 0x20
#define TALK_PANEL_Y 0xAB
#define TALK_PANEL_W 0x100
#define TALK_PANEL_H 0x30

/* Its texture: the top of the page is something else, so it starts a row of
   0x50 down, and the palette is the one at 0x6038. */
#define TALK_PANEL_U    0
#define TALK_PANEL_V    0x50
#define TALK_PANEL_CLUT 0x6038

/* The page the panel's texture is in. */
#define TALK_PANEL_PAGE_X 0x380
#define TALK_PANEL_PAGE_Y 0x150

extern int    g_btl_talk_state;
extern char  *g_btl_prim_next;
extern u_long g_btl_ot[][3];
extern int    g_btl_ot_index;

void BtlTalkDrawPanel(void)
{
    SPRT    s;
    DR_MODE mode;

    if (g_btl_talk_state == 0) {
        return;
    }
    SetSprt(&s);
    SetSemiTrans(&s, 0);
    SetShadeTex(&s, 1);
    s.x0 = TALK_PANEL_X;
    s.y0 = TALK_PANEL_Y;
    s.v0 = TALK_PANEL_V;
    s.w = TALK_PANEL_W;
    s.h = TALK_PANEL_H;
    s.u0 = TALK_PANEL_U;
    s.clut = TALK_PANEL_CLUT;
    SetDrawMode(&mode, 0, 0,
                GetTPage(0, 0, TALK_PANEL_PAGE_X, TALK_PANEL_PAGE_Y), 0);

    memcpy(g_btl_prim_next, &s, sizeof(SPRT));
    AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
    g_btl_prim_next += sizeof(SPRT);
    memcpy(g_btl_prim_next, &mode, sizeof(DR_MODE));
    AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
    g_btl_prim_next += sizeof(DR_MODE);
}
