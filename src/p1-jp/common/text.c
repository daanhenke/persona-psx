/* Persona 1 (JP) - script text copy.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008BDB0
 *   ADV @ 0x8007D820
 *   S2D @ 0x8007C220
 * Touches no globals, so one source covers all three.
 */
#include <types.h>

/* Copies up to the two-byte marker {0xFF, 0x01} and returns where the
   destination ended, so the caller can keep appending.
 *
 * 0xFF is an escape byte, not a terminator: 0xFF followed by anything other
 * than 0x01 is copied through unchanged. That is why the test is a nested if
 * rather than `src[0] == 0xFF && src[1] == 1` - the && form makes gcc lay the
 * loop out with the test at the bottom and a jump into it, where the original
 * tests at the top and jumps back. */
u_char *TextCopyUntilEnd(u_char *dst, u_char *src)
{
    while (1) {
        if (*src == 0xFF) {
            if (src[1] == 1) {
                break;
            }
        }
        *dst = *src;
        src++;
        dst++;
    }
    return dst;
}
