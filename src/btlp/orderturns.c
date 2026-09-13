/* Persona 1 (JP) - putting the round's fighters in turn order.  BTLP only.
 *   0x80096188 BtlOrderTurns
 *
 * A bubble sort of the n actor slots in `order`, highest initiative first;
 * two fighters with the same initiative keep the order they came in.
 *
 * Three encounters, 0x20 to 0x22, then give slot 5 - the first enemy - a
 * second turn straight after its first: everything behind it moves back one
 * place to make room. The answer is how many turns the list now holds.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>

/* The slot that acts twice, and the one encounter it does so in that is not
   one of the two with a shape to change into. */
#define ORDER_TWICE_SLOT 5
#define ORDER_ENCOUNTER_21 0x21

/* 99.29%, one instruction: the inner loop's bound is the image's copy of the
   value its entry test computed (addu t3, v0), where gcc here works it out
   again from the outer loop's counter (addiu t3, t1, -1). The bound spelled
   n - i - 1 is what gives the counter as n - i at all - n - 1 - i folds the
   one into it - and a while loop, the test turned round, or a local for the
   bound all leave the recomputation in place. */
#ifdef NON_MATCHING
int BtlOrderTurns(u_char *order, int n)
{
    int    i;
    int    j;
    u_char t;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (g_btl_actors[order[j]].initiative
                < g_btl_actors[order[j + 1]].initiative) {
                t = order[j];
                order[j] = order[j + 1];
                order[j + 1] = t;
            }
        }
    }
    if (g_btl_encounter == BTL_ENCOUNTER_PAIR
        || g_btl_encounter == BTL_ENCOUNTER_TRIO
        || g_btl_encounter == ORDER_ENCOUNTER_21) {
        for (i = 0; i < n; i++) {
            if (order[i] == ORDER_TWICE_SLOT) {
                for (j = n - 1; j >= i; j--) {
                    order[j + 1] = order[j];
                }
                n++;
                break;
            }
        }
    }
    return n;
}
#else
INCLUDE_ASM("btlp/nonmatchings/orderturns", BtlOrderTurns);
#endif
