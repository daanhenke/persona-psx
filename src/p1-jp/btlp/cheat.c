/* Persona 1 (JP) - the hidden debug display.  BTLP only.
 *   0x8008E254 BtlCheatWatch
 *
 * Two gates guard it, and the first is the party leader's name: unless the
 * eight bytes of g_btl_actors[0].c.name match g_btl_cheat_name exactly, the
 * button sequence is never even looked at. With the right name entered, the
 * masks in g_btl_cheat_pad are matched one per step against the second pad's
 * fresh presses; a wrong press drops the count back to the start. The zero
 * entry that ends the table is the reward rather than a step - reaching it
 * resets the count and flips g_btl_debug_hud, which unhides the four
 * indicators the frame drawer otherwise keeps invisible.
 *
 * The name it wants is eight glyph codes:
 *
 *     0C 2E 09 10 2F 29 41 26   shi n ke ta ga ru be yo
 *                               しんけたがるべよ
 *
 * read through the kana half of the font, whose order comes off the NAME
 * overlay's keyboard - 0x01 is A, and the voiced forms continue the same run.
 * It is not a word, which is presumably the point. (FONT.BIN holds the glyph
 * images themselves and has not been decoded, so the reading rests on the
 * keyboard order alone.)
 *
 * The sequence itself, on the second controller:
 *
 *     1000 1000 4000 4000 8000 2000 8000 2000 0040 0020 0000
 *     tri  tri  x    x    sq   cir  sq   cir  down right  end
 *
 * Polled from BtlDrawFrame, so it only advances while the battle is drawing.
 */
#include <types.h>
#include <persona/btlp/actor.h>

/* Characters of the name that have to match. */
#define BTL_CHEAT_NAME_LEN 8

extern const long  g_btl_cheat_name[];
extern u_short     g_btl_cheat_pad[];
extern int         g_btl_cheat_step;
extern u_char      g_btl_debug_hud;
extern u_short     g_btl_pad2_edge;

void BtlCheatWatch(void)
{
    int          i;
    int          named;
    const long  *want;
    int          step;

    i = 0;
    named = 1;
    want = g_btl_cheat_name;
    while (i < BTL_CHEAT_NAME_LEN) {
        if (g_btl_actors[0].c.name[i] != *want) {
            named = 0;
            break;
        }
        i++;
        want++;
    }

    if (named) {
        step = g_btl_cheat_step;
        if (g_btl_cheat_pad[step] == 0) {
            g_btl_cheat_step = 0;
            g_btl_debug_hud ^= 1;
        } else if (g_btl_pad2_edge != 0) {
            if ((g_btl_cheat_pad[step] & g_btl_pad2_edge) != 0) {
                g_btl_cheat_step = step + 1;
            } else {
                g_btl_cheat_step = 0;
            }
        }
    }
}
