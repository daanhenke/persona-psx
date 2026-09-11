/* Persona 1 (JP) - the move that whites the arena out behind its effect.
 * BTLP only.
 *   0x800B95E4 BtlFxStart2E
 *
 * A start handler out of g_btl_spell_fx, and the only one that writes the
 * arena's own colour. All three channels are put to full and the arena is given
 * a fade to walk back down by, so the field flashes white and then returns
 * while the effect stands; the records themselves are the ordinary ones over
 * everything the move reaches, tinted blue-green rather than white and started
 * four frames in.
 *
 * The three channels are one array. Its second and third elements carried a
 * symbol of their own until this routine needed them written together.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The arena flashed to full on every channel, and how fast it walks back. */
#define ARENA_FULL 0xFF
#define ARENA_FADE 0x60

/* The colour the fighters under the effect are walked to, and how long the
   first record waits. */
#define FX_2E_R     0
#define FX_2E_G     0xFF
#define FX_2E_B     0xC0
#define FX_2E_TIMER 4

extern short g_btl_arena_fade;
extern short g_btl_arena_rgb[];

BtlObj *BtlFxStart2E(void)
{
    g_btl_arena_rgb[0] = ARENA_FULL;
    g_btl_arena_rgb[1] = ARENA_FULL;
    g_btl_arena_rgb[2] = ARENA_FULL;
    g_btl_arena_fade   = ARENA_FADE;
    return BtlOpenFxOnTargets(FX_2E_R, FX_2E_G, FX_2E_B, FX_2E_TIMER);
}
