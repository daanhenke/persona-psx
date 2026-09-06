/* Persona 1 (JP) - the contact box.  BTLP only.
 *   0x8007B5D0 BtlTalkChoice  0x8007B5E0 BtlTalkLive
 *   0x8007B61C BtlTalkIdle    0x8007B648 BtlTalkHide
 *
 * Four things to say, laid out two by two. The labels are the persuasion verbs
 * the game is built on and each character has their own set, so BtlTalkOpen
 * picks the set with the actor's id and captions the box with their name.
 *
 * BtlTalkUpdate moves the highlight - up and down by two cells, left and right
 * by one, each with a parity test that keeps the move inside the grid - and
 * latches the answer into g_btl_talk_choice on confirm, or -1 on cancel. Until
 * one of those it holds BTL_TALK_WAITING, which is what callers poll for.
 *
 * The box can be left up without taking the pad, which is how the party's turn
 * shows whose box it is while somebody else's animation plays.
 */
#include <types.h>

#define BTL_TALK_GONE 0
#define BTL_TALK_IDLE 1   /* up, ignoring the pad */
#define BTL_TALK_LIVE 2   /* up, taking the pad   */

extern int g_btl_talk_choice;
extern int g_btl_talk_index;
extern int g_btl_talk_state;

extern void BtlCursorInitPrims(void);
extern void BtlCursorShow(int on);

int BtlTalkChoice(void)
{
    return g_btl_talk_choice;
}

void BtlTalkLive(int cell)
{
    g_btl_talk_state = BTL_TALK_LIVE;
    g_btl_talk_index = cell;
    BtlCursorInitPrims();
    BtlCursorShow(1);
}

void BtlTalkIdle(void)
{
    g_btl_talk_state = BTL_TALK_IDLE;
    BtlCursorShow(0);
}

void BtlTalkHide(void)
{
    g_btl_talk_state = BTL_TALK_GONE;
    BtlCursorShow(0);
}
