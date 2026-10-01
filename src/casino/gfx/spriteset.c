/* Persona 1 (JP) - CASINO's sprite runs.
 *   0x800695C4 CasinoSpritesSetZ
 *   0x80069618 CasinoSpritesSetOn
 *
 * A game keeps each of its pictures as a run of consecutive sprites; these
 * set a whole run's depths, or switch it on or off.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

void CasinoSpritesSetZ(short first, short n, u_short *z)
{
    int i;

    for (i = 0; i < n; i++) {
        g_casino_sprites.z[i + first] = *z++;
    }
}

void CasinoSpritesSetOn(short first, short n, int on)
{
    int i;

    for (i = 0; i < n; i++) {
        g_casino_sprites.on[i + first] = on;
    }
}
