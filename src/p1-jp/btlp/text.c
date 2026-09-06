/* Persona 1 (JP) - the battle's second message window.  BTLP only.
 *   0x8007CA3C BtlTextState     0x8007CA70 BtlTextReset
 *   0x8007CA4C BtlTextSetState  0x8007CAF4 BtlTextOpen
 *   0x8007D3AC BtlTextWaitDone  0x8007CC30 BtlTextAdvance
 *
 * The overlay runs two message windows. Both are records of the same shape,
 * with the state at +2, and both are driven by the same per-frame routine; the
 * sequencer owns one of them and this is the other. Opening it clears the whole
 * record and sets the state to 1, and the state goes back to zero when the
 * script runs out.
 *
 * Opening the window it is already showing leaves it alone rather than starting
 * it over, which is why a message assembled by substitution has to say so: the
 * buffer's address does not change when its contents do, so BtlSetInsert sets
 * a bit here and this clears it again.
 *
 * Waiting for it runs five more frames afterwards, exactly as BtlSeqWaitDone
 * does, so the last frame of the message is on the screen before whatever
 * follows starts drawing over it.
 */
#include <types.h>
#include <persona/btlp/window.h>

#define BTL_TEXT_TAIL 5

/* Bit 0: the message buffer was rewritten, so reopen it even though its
   address has not changed. */
#define BTL_TEXT_EDITED 1

/* Where a window's glyphs and palettes are staged in VRAM. The two pages are
   three tile columns apart, and a column is 0x40 across in 16-bit terms. */
#define BTL_TEXT_PAGE0   11
#define BTL_TEXT_PAGE_W  3
#define BTL_TEXT_COL     0x40
#define BTL_TEXT_GLYPH_Y 0x140
#define BTL_TEXT_CLUT_Y  0x180
#define BTL_TEXT_CLUT_W  0x10
#define BTL_TEXT_CLUT_H  4

extern BtlWindow      g_btl_text;
extern int            g_btl_text_pause;
extern int            g_btl_text_edited;
extern const u_char  *g_btl_text_script;
extern int            g_btl_text_page;
extern const u_short  g_btl_text_cluts[];

extern void BtlDrawFrame(void);
extern void BtlRunFrames(int frames);
extern void BtlWindowStep(BtlWindow *w, int pause);
extern void BtlQueueVramLoad(const void *src, int x, int y, int w, int h);

int BtlTextState(void)
{
    return g_btl_text.state;
}

void BtlTextWaitDone(void)
{
    while (BtlTextState() != 0) {
        BtlDrawFrame();
    }
    BtlRunFrames(BTL_TEXT_TAIL);
}

/* Puts the window into a state with a timer of its own, and makes it wait at
   the end of a field rather than finish there. */
void BtlTextSetState(short state, int timer)
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

void BtlTextAdvance(void)
{
    BtlWindowStep(&g_btl_text, g_btl_text_pause);
}
