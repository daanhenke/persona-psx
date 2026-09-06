/* Persona 1 (JP) - which script a line of negotiation plays.  BTLP only.
 *   0x8006C884 BtlTalkPickScript
 *
 * Each character has twelve lines - three ways of saying each of four verbs -
 * held as script ids in a table and looked up by (character, verb, variant).
 *
 * One line does not go through the table. Six particular demons answer the
 * third character's second verb with a script of their own, matched on the
 * persona the offer in hand would hand over.
 */
#include <types.h>
#include <persona/btlp/offer.h>

/* The line that has an answer of its own, and how many demons give it. */
#define TALK_SPECIAL_WHO     2
#define TALK_SPECIAL_VERB    1
#define TALK_SPECIAL_VARIANT 0
#define TALK_SPECIAL_DEMONS  6

/* Verbs a character has, and ways of saying each. */
#define TALK_VERBS    4
#define TALK_VARIANTS 3

extern short         g_btl_offer_slot;
extern const u_char  g_btl_talk_script_ids[][TALK_VERBS][TALK_VARIANTS];
extern const u_char *g_btl_talk_scripts[];
extern const u_char  g_btl_talk_special_script[];
extern const u_char  g_btl_talk_special[];

extern void BtlSeqPlay(const u_char *script);

void BtlTalkPickScript(int who, int verb, int variant)
{
    const u_char *script;
    int           i;

    if (who == TALK_SPECIAL_WHO && verb == TALK_SPECIAL_VERB &&
        (i = 0, variant == TALK_SPECIAL_VARIANT)) {
        while (i < TALK_SPECIAL_DEMONS) {
            if (g_btl_talk_special[i] == g_btl_offer[g_btl_offer_slot].persona) {
                break;
            }
            i++;
        }
        if (i != TALK_SPECIAL_DEMONS) {
            script = g_btl_talk_special_script;
            goto play;
        }
    }
    script = g_btl_talk_scripts[
        g_btl_talk_script_ids[who][verb][variant]];
play:
    BtlSeqPlay(script);
}
