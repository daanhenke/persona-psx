/* Persona 1 (JP) - the pad the battle's menus read, and the cursor.  BTLP only.
 *   0x8007A2C0 BtlInputKeys   0x8007A2F0 BtlCursorShow
 *
 * The cursor is a textured quad rather than an arrow - BtlCursorPlace writes
 * its four corners 0x10 apart around whatever is being pointed at - and
 * BtlCursorDraw only copies it into the ordering table while bit 0 of
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
