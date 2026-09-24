/* Persona 1 (JP) - which of a character's three lines to use.  BTLP only.
 *   0x800692BC BtlPickLine
 *
 * Every party member has three things to say in each of three situations, and
 * this deals them out so all three are heard before any is heard twice. The
 * first call for a situation finds the numbers 0 to 2 that are not in the used
 * list yet, takes one at random and writes it into the first free place;
 * filling the last place starts the cursor at zero. After that the cursor
 * simply steps round the three in the order they were dealt.
 *
 * The number goes to BtlBgmChange as the base its table nibble is added to, so
 * a track with three variants gets a different one each time round, and the
 * same pair of character and situation picks the text alongside it.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

#define BTL_LINES 3   /* lines in a situation */
#define BTL_KINDS 3   /* situations a member has lines for */

/* One record per party member. Only the rotation is used here; the byte at +0
   and the short at +4 belong to the Persona lookup. */
typedef struct {
    /* 0x00 */ u_char pad00[6];
    /* 0x06 */ short  cursor[BTL_KINDS];            /* -1 until it is dealt */
    /* 0x0C */ u_char pad0C[2];
    /* 0x0E */ short  used[BTL_KINDS][BTL_LINES];   /* -1 for a free place  */
    /* 0x20 */ u_char pad20[6];
} BtlMember;                                        /* 0x26 bytes */

extern BtlMember g_btl_member[];

/* The same deal as BtlSayDemonLine: one variable walks the candidates and
   then holds the line picked, and the routine answers an int, which is how
   talk.h has always declared it (a short answer sign-extends it on the way
   out). */
int BtlPickLine(int kind)
{
    BtlMember *m;
    short      fresh[BTL_LINES];
    int        n;
    int        i;
    int        pick;

    m = &g_btl_member[g_btl_actor_slot];
    if (m->cursor[kind] == -1) {
        n = 0;
        for (pick = 0; pick < BTL_LINES; pick++) {
            for (i = 0; i < BTL_LINES; i++) {
                if (m->used[kind][i] == pick) {
                    break;
                }
            }
            if (i == BTL_LINES) {
                fresh[n] = pick;
                n++;
            }
        }
        pick = fresh[rand() % n];
        for (i = 0; i < BTL_LINES; i++) {
            if (m->used[kind][i] == -1) {
                m->used[kind][i] = pick;
                if (i == BTL_LINES - 1) {
                    m->cursor[kind] = 0;
                }
                break;
            }
        }
        return pick;
    }
    m->cursor[kind] = (m->cursor[kind] + 1) % BTL_LINES;
    return m->used[kind][m->cursor[kind]];
}

