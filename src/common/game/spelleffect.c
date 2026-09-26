/* Persona 1 (JP) - what a recovery spell or item does outside battle.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x800946C4   DNG 0x8008EF98
 *
 * The skills screen casts with it and the item menu uses items through it
 * (an item names the spell it works like). The healing is the caster's
 * magic attack plus up to fifteen, four times that for the stronger ones,
 * capped at the maximum; the two field spells set the flags the field reads.
 * Defined old-style, so all three numbers are narrowed here.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* The steps left on the walking effect, and the flag the other field spell
   raises. Reached by address. */
#define EFFECT_STEPS (*(u_char *)0x801F29A9)
#define FIELD_SPELL  (*(u_char *)0x801F29A8)
#define EFFECT_STEPS_START 0x10

extern int rand(void);

void SpellApplyField(target, caster, spell)
    short target;
    short caster;
    short spell;
{
    Char *chars = g_chars;
    int   t = g_party[target];
    int   c = g_party[caster];

    switch (spell) {
    case 0x5F:
    case 0x62:
        chars[t].hp += chars[c].mag_atk + rand() % 16;
        goto cap;
    case 0x60:
    case 0x63:
        chars[t].hp += (chars[c].mag_atk + rand() % 16) * 4;
    cap:
        if (chars[t].hp > chars[t].hp_max) {
            chars[t].hp = chars[t].hp_max;
        }
        break;
    case 0x61:
    case 0x64:
        chars[t].hp = chars[t].hp_max;
        break;
    case 0x67:
    case 0x6A:
        chars[t].status = STATUS_GOOD;
        break;
    case 0x73:
        EFFECT_STEPS = EFFECT_STEPS_START;
        /* fall through */
    case 0x6F:
        FIELD_SPELL = 1;
        break;
    }
}
