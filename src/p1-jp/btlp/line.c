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
#include <types.h>

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
extern short     g_btl_actor_slot;

extern int rand(void);

short BtlPickLine(int kind)
{
    BtlMember *m;
    short     *used;
    short      pick;
    int        i;
    int        n;
    int        v;
    int        next;
    short      fresh[BTL_LINES];

    m = &g_btl_member[g_btl_actor_slot];
    if (m->cursor[kind] == -1) {
        n = 0;
        v = 0;
        do {
            i = 0;
            used = m->used[kind];
            do {
                if (*used == v) {
                    break;
                }
                i++;
                used++;
            } while (i < BTL_LINES);
            if (i == BTL_LINES) {
                fresh[n] = v;
                n++;
            }
            v++;
        } while (v < BTL_KINDS);
        i = 0;
        used = m->used[kind];
        pick = fresh[rand() % n];
        do {
            if (*used == -1) {
                *used = pick;
                if (i == BTL_LINES - 1) {
                    m->cursor[kind] = 0;
                }
                break;
            }
            i++;
            used++;
        } while (i < BTL_LINES);
    } else {
        next = (m->cursor[kind] + 1) % BTL_LINES;
        m->cursor[kind] = next;
        pick = m->used[kind][next];
    }
    return pick;
}
