/* Persona 1 (JP) - the pad the battle's menus read, and the cursor.  BTLP only.
 *   0x8007A2C0 BtlInputKeys   0x8007A2F0 BtlCursorShow
 *
 * The cursor is four small sprites at the corners of a box rather than an arrow
 * - BtlCursorPlace puts them 0x10 apart around whatever is being pointed at -
 * and the draw pass only copies them into the ordering table while bit 0 of
 * g_btl_cursor_flags is set. So hiding the cursor is not a move off screen, it
 * is this bit.
 */
#include <types.h>

#define BTL_CURSOR_VISIBLE 1

extern u_long  g_btl_input;
extern u_char  g_btl_cursor_flags;

u_long BtlInputKeys(void)
{
    return g_btl_input;
}

void BtlCursorShow(int on)
{
    if (on == 0) {
        g_btl_cursor_flags &= ~BTL_CURSOR_VISIBLE;
    } else {
        g_btl_cursor_flags |= BTL_CURSOR_VISIBLE;
    }
}
