/* Persona 1 (JP) - CASINO game 1: video poker.
 *
 * Five cards are dealt face up, any of them can be held, the rest are
 * drawn again, and the final hand is paid by its rank times the coins
 * bet (up to ten). A royal flush on a ten-coin bet takes a hundredth of
 * the jackpot instead, which the save keeps between visits.
 */
#ifndef PERSONA_CASINO_POKER_H
#define PERSONA_CASINO_POKER_H

#include <persona/casino/casino.h>

/* g_casino_step values the game runs through. */
#define POKER_OPEN        0x11 /* load, fade in                      */
#define POKER_ENTER       0x12 /* the table's two halves slide in    */
#define POKER_HOLD        0x15 /* pick the cards to keep             */
#define POKER_WIN         0x16 /* the win jingle                     */
#define POKER_LOSE        0x17
#define POKER_DOUBLE_PLAY 0x1A /* the double-up game chosen          */
#define POKER_JUDGE       0x25 /* rank the final hand                */
#define POKER_BET         0x26 /* coins in                           */
#define POKER_DEAL        0x2E
#define POKER_DRAW        0x2F /* turn the replaced cards over       */
#define POKER_CLEAR       0x30 /* the cards swept off, a new round   */
#define POKER_COLLECT     0x31 /* the win counted into the money     */
#define POKER_START_HAND  0x32 /* the halves slide back out          */
#define POKER_DOUBLE_UP   0x33 /* one of three double-up games       */
#define POKER_CHOOSE      0x3B /* after a win: take it, or double up */
#define POKER_PEEK        0x70 /* the hand shown while a key is held */

/* A card is suit * 13 + rank; suit 4 is the joker. */
#define POKER_CARDS 5
#define POKER_RANKS 9

extern CasinoObj g_casino_objs[25]; /* the 3D objects; poker's cards first */

extern u_char g_poker_hand[POKER_CARDS];    /* the cards dealt            */
extern u_char g_poker_held[POKER_CARDS];    /* kept through the draw      */
extern u_char g_poker_marks[POKER_CARDS];   /* the cards that make the rank */
extern u_char g_poker_flipped[POKER_CARDS]; /* turned over in the draw    */
extern u_char g_poker_rank;                 /* 0 nothing, 1 royal flush .. 9 one pair */
extern u_char g_poker_flip_idx;             /* the card the draw is turning */
extern u_char g_poker_jackpot_hit;
extern u_int  g_poker_jackpot;
extern int    g_poker_payout;
extern u_char g_poker_double_game; /* which double-up game was picked */
extern u_char g_poker_double_step; /* its own step                    */

extern s8    g_poker_flip_sound;               /* the turn sound is owed     */
extern int   g_poker_win_seq;                  /* the jingle playing, or -1  */
extern short g_poker_pay[POKER_RANKS];         /* coins per coin bet, by rank */
extern short g_poker_card_spr[POKER_CARDS];    /* each card's first sprite   */
extern short g_poker_hand_spr[POKER_RANKS];    /* each rank's name on the table */

/* Palette animations, each its own one-element array: the image reads
   `on` and passes the address without deriving one from the other, which
   is what an array name gives and a struct variable does not. */
extern CasinoPalAnim g_poker_palanim0[];
extern CasinoPalAnim g_poker_palanim1[];
extern CasinoPalAnim g_poker_palanim2[];
extern CasinoPalAnim g_poker_palanim3[];
extern CasinoPalAnim g_poker_palanim4[];

#endif
