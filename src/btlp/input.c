/* Persona 1 (JP) - the pad the battle's menus read, and the cursor.  BTLP only.
 *   0x8007A2C0 BtlInputKeys    0x8007A2D0 BtlInputClear
 *   0x8007A2E0 BtlCursorFlags  0x8007A2F0 BtlCursorShow
 *
 * Four one-line accessors over the two globals in input.h. BtlInputClear is
 * what a loop calls after drawing a frame so a press made before it opened
 * cannot be read inside it, and BtlCursorFlags exists so a caller can put the
 * cursor back the way it found it - the whole byte goes to BtlCursorShow,
 * which only cares whether it is zero.
 */
#include <decomp/types.h>
#include <persona/btlp/input.h>

u_long BtlInputKeys(void)
{
    return g_btl_input;
}

void BtlInputClear(void)
{
    g_btl_input = 0;
}

u_char BtlCursorFlags(void)
{
    return g_btl_cursor_flags;
}

void BtlCursorShow(int on)
{
    if (on == 0) {
        g_btl_cursor_flags &= ~BTL_CURSOR_VISIBLE;
    } else {
        g_btl_cursor_flags |= BTL_CURSOR_VISIBLE;
    }
}
