/* Persona 1 (JP) - small card-selection helpers.
 * Keep the original entry-point names until their callers are reconstructed.
 */
#include <decomp/types.h>

int func_80082034(int value)
{
    if (value == 7) {
        return 9;
    }
    if (value == 6) {
        return 10;
    }
    return 0xFF;
}
