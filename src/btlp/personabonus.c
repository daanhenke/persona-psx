/* Persona 1 (JP) - goodwill for carrying the right Persona.  BTLP only.
 *   0x80066A94 BtlTalkPersonaBonus
 *
 * Six demon species each warm to one kind of Persona, paired off between
 * g_btl_kin_species and g_btl_kin_persona. If the demon being spoken to is one
 * of the six and the speaker has that kind equipped, the mood the line was
 * aimed at goes up by fifteen.
 *
 * Only moods 0 and 3 take the bonus; the other two can only be moved the
 * ordinary way, which is the same pair BtlTalkLikedEquip refuses.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/battle.h>

#define BTL_KIN_PAIRS 6

/* What a match is worth. */
#define BTL_KIN_BONUS 15

/* The two moods the bonus can reach. */
#define BTL_KIN_MOOD_A 0
#define BTL_KIN_MOOD_B 3

extern const u_char g_btl_kin_species[];
extern const u_char g_btl_kin_persona[];

void BtlTalkPersonaBonus(int which)
{
    const BtlStats *persona;
    short            *mood;
    u_char           *list;
    u_char            species;
    int               i;

    i = 0;
    mood = g_btl_offer[g_btl_offer_slot].mood;
    list = &g_btl_actors[g_btl_actor_slot]
                .c.list[g_btl_actors[g_btl_actor_slot].c.entry];
    species = (&g_btl_actors[BTL_PARTY])[g_btl_talk_target].species;
    persona = &g_btl_personas[*list];
    while (i < BTL_KIN_PAIRS) {
        if (species == g_btl_kin_species[i]) {
            break;
        }
        i++;
    }
    if (i != BTL_KIN_PAIRS
        && g_btl_kin_persona[i] == persona->kind
        && (which == BTL_KIN_MOOD_A || which == BTL_KIN_MOOD_B)) {
        mood += which;
        *mood += BTL_KIN_BONUS;
    }
}
