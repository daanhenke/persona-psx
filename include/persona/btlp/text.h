#ifndef PERSONA_BTLP_TEXT_H
#define PERSONA_BTLP_TEXT_H

/* Persona 1 (JP) - the battle's second message window.
 *
 * The overlay runs two message windows. Both are BtlWindow records driven by
 * the same per-frame routine; the sequencer owns one and this is the other.
 * Opening it clears the record and sets the state to 1, and the state falls
 * back to zero when the script runs out - which is what BtlTextWaitDone
 * watches, five frames past the end so the last frame is on the screen before
 * anything draws over it.
 *
 * Opening the window it is already showing leaves it alone rather than
 * restarting it, so a message assembled by substitution has to announce
 * itself: the buffer's address does not change when its contents do, which is
 * what g_btl_text_edited is for. BtlSetInsert sets it and BtlTextOpen clears
 * it again.
 */
#include <decomp/types.h>
#include <persona/btlp/window.h>

extern BtlWindow      g_btl_text;
extern int            g_btl_text_pause;
extern int            g_btl_text_edited;
extern const u_char  *g_btl_text_script;
extern int            g_btl_text_page;
extern const u_short  g_btl_text_cluts[];

/* Frames BtlTextWaitDone runs on past the end of the script. */
#define BTL_TEXT_TAIL 5

/* Bit 0 of g_btl_text_edited: the message buffer was rewritten, so reopen the
   window even though its address has not changed. */
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

extern int  BtlTextState(void);
extern void BtlTextReset(void);
extern void BtlTextClose(void);
extern void BtlTextAdvance(void);
extern void BtlTextWaitDone(void);

/* Answers how many characters the message came to, so a caller can centre the
   box it is about to put round it. */
extern int BtlTextOpen(const u_char *script, short x, short y);

/* Three arguments, and the third is never read: the body sets the pause
   itself. All six call sites in the overlay pass it, and all six pass 1 - the
   value the body writes - so it is a parameter the code outgrew rather than
   one caller getting the prototype wrong. */
extern void BtlTextSetState(short state, int timer, int pause);

#endif
