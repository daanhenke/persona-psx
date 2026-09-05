/* Persona 1 (JP) - the name-entry screen.  NAME @ 0x80066134.
 *
 * Three fields are entered in turn. Each has eight cells of room, but only the
 * last is used that far - the first two stop at five.
 */
#include <types.h>
#include <persona/name/entry.h>

/* Every field has to hold something before the screen will accept the name. */
u_char NameEntryComplete(void)
{
    int i;

    for (i = 0; i < NAME_SHORT; i++) {
        if (g_name_entry.text[0][i] != 0) {
            break;
        }
    }
    if (i == NAME_SHORT) {
        return 0;
    }
    for (i = 0; i < NAME_SHORT; i++) {
        if (g_name_entry.text[1][i] != 0) {
            break;
        }
    }
    if (i == NAME_SHORT) {
        return 0;
    }
    for (i = 0; i < NAME_CELLS; i++) {
        if (g_name_entry.text[2][i] != 0) {
            break;
        }
    }
    return i != NAME_CELLS;
}
