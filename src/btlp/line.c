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
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>

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

/* 88.62%, and the shape is the image's: the counter reset at the head of each
   pass (reorg copies that into the back branch's slot, which makes it look
   like the tail), the fresh number written through the record rather than a
   `next` local, and nothing lifted out of the walks - the 3 and the scratch
   array's address are rebuilt each pass, which goto loops give and for loops
   do not. What is left says the outer walk was a real loop after all: the
   image works out m + kind * 6 once ahead of it and adds the 0xE inside,
   where real loops here lift the whole address, 0xE included, the other way
   round. */
#ifdef NON_MATCHING
short BtlPickLine(int kind)
{
    BtlMember *m;
    short     *used;
    short      pick;
    int        i;
    int        n;
    int        v;
    short      fresh[BTL_LINES];

    m = &g_btl_member[g_btl_actor_slot];
    if (m->cursor[kind] == -1) {
        n = 0;
        v = 0;
    next_value:
        i = 0;
        used = m->used[kind];
    next_place:
        if (*used == v) {
            goto found;
        }
        i++;
        used++;
        if (i < BTL_LINES) {
            goto next_place;
        }
    found:
        if (i == BTL_LINES) {
            fresh[n] = v;
            n++;
        }
        v++;
        if (v < BTL_KINDS) {
            goto next_value;
        }
        pick = fresh[rand() % n];
        i = 0;
        used = m->used[kind];
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
        return pick;
    }
    m->cursor[kind] = (m->cursor[kind] + 1) % BTL_LINES;
    return m->used[kind][m->cursor[kind]];
}
#else
INCLUDE_ASM("btlp/nonmatchings/line", BtlPickLine);
#endif

