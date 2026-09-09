/* Persona 1 (JP) - closing the message box.  BTLP only.
 *   0x8007F3C8 BtlCloseMessage
 *
 * A unit of its own, well past the rest of the box code in box.c.
 */
#include <decomp/types.h>

extern void BtlBoxClose(void);

/* The counterpart to BtlOpenMessage, and nothing but a call to BtlBoxClose.
   The cursor code uses it where it would otherwise have opened a help line, so
   the box goes away when the help is turned off. */
void BtlCloseMessage(void)
{
    BtlBoxClose();
}
