/* Persona 1 (JP) - the battle's second message window.  BTLP only.
 *   0x8007CA3C BtlTextState    0x8007CA70 BtlTextReset
 *   0x8007CA4C BtlTextSetState 0x8007CAF4 BtlTextOpen
 *   0x8007CC10 BtlTextClose    0x8007CC30 BtlTextAdvance
 *
 * Opening the window it is already showing leaves it alone rather than
 * starting it over, so a message assembled by substitution has to say so - see
 * BTL_TEXT_EDITED in text.h. Closing it is the same reset opening does, which
 * is why BtlTextClose has nothing of its own to say.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

int BtlTextState(void)
{
    return g_btl_text.state;
}
/* The third argument is taken and never read - the body sets the pause itself.
   Every call site in the overlay passes 1 for it. */
void BtlTextSetState(short state, int timer, int pause)
{
    g_btl_text.state = state;
    g_btl_text.timer = timer;
    g_btl_text_pause = 1;
}

/* Empties the window and points its glyph staging at the current page. */
void BtlTextReset(void)
{
    bzero(&g_btl_text, sizeof(BtlWindow));
    g_btl_text.state = 0;
    g_btl_text.placed = 0;
    g_btl_text.staged = 0;
    g_btl_text.x = 0;
    g_btl_text.y = 0;
    g_btl_text.vram_x = (g_btl_text_page * BTL_TEXT_PAGE_W + BTL_TEXT_PAGE0) *
                        BTL_TEXT_COL;
    g_btl_text.vram_y = BTL_TEXT_GLYPH_Y;
    g_btl_text_script = 0;
}

/* Returns how many characters the message came to, so a caller can centre the
   box it is about to put round it. */
int BtlTextOpen(const u_char *script, short x, short y)
{
    if (g_btl_text_script != script ||
        (g_btl_text_edited & BTL_TEXT_EDITED) != 0) {
        BtlQueueVramLoad(g_btl_text_cluts,
                         (short)(g_btl_text_page * BTL_TEXT_PAGE_W +
                                 BTL_TEXT_PAGE0) * BTL_TEXT_COL,
                         BTL_TEXT_CLUT_Y, BTL_TEXT_CLUT_W, BTL_TEXT_CLUT_H);
        BtlTextReset();
        g_btl_text.state = 1;
        g_btl_text.script = script;
        g_btl_text_script = script;
        g_btl_text_pause = 0;
        g_btl_text_edited &= ~BTL_TEXT_EDITED;
        BtlWindowStep(&g_btl_text, 0);
    }
    g_btl_text.x = x;
    g_btl_text.y = y;
    g_btl_text.dx = 0;
    g_btl_text.dy = 0;
    g_btl_text.slide = 0;
    return g_btl_text.staged;
}

void BtlTextClose(void)
{
    BtlTextReset();
}

void BtlTextAdvance(void)
{
    BtlWindowStep(&g_btl_text, g_btl_text_pause);
}
