/* Persona 1 (JP) - CASINO game 2: blackjack.
 *
 * Four decks dealt from one shoe, reshuffled once it runs out. The player
 * can take insurance against a dealer's ace, double, and split a pair into
 * two hands. An ace with the jack of its own suit pays above a plain
 * blackjack, by suit.
 */
#ifndef PERSONA_CASINO_BLACKJACK_H
#define PERSONA_CASINO_BLACKJACK_H

#include <persona/casino/casino.h>

/* g_casino_step values the game runs through. */
#define BJ_OPEN         0x11 /* load, fade in                        */
#define BJ_ENTER        0x12 /* the table's two halves slide in      */
#define BJ_CHOOSE       0x1B /* hit, stand, double or split          */
#define BJ_BET          0x26 /* coins in                             */
#define BJ_DEAL         0x2E /* two cards each, the dealer's one down */
#define BJ_START_HAND   0x32 /* the halves slide back out            */
#define BJ_PLAY         0x76 /* the hand played out (g_bj_play_step) */
#define BJ_INSURE       0x80 /* the dealer shows an ace: insure?     */
#define BJ_INSURED      0x81 /* the hole card checked, insurance paid */
#define BJ_NO_INSURANCE 0x82

/* What a hand's first two cards make (g_bj_natural). */
#define BJ_HAND_PLAIN   1
#define BJ_HAND_NATURAL 8 /* an ace and a ten                     */
/* 4..7: the ace and the jack of one suit, suit by suit. */

#define BJ_HAND_CARDS 8

extern CasinoPalAnim g_bj_palanim0[];

extern u_char g_bj_cards[2][BJ_HAND_CARDS]; /* the player's hands; the second after a split */
extern u_char g_bj_count[2];                /* cards in each                 */
extern u_char g_bj_total[2];                /* the first hand's total: aces low, one ace high */
extern u_char g_bj_split_total[2];
extern u_char g_bj_natural[2];              /* each hand's two-card kind     */
extern u_char g_bj_dealer[12];
extern u_char g_bj_dealer_count;
extern u_char g_bj_dealer_total[2];
extern u_char g_bj_dealer_bj;  /* the hole card makes the dealer's ace a blackjack */
extern u_char g_bj_insurance;  /* coins put on it                    */
extern u_char g_bj_card_obj;   /* the next card's 3D object, counting down */
extern u_char g_bj_shoe_pos;   /* the next card in g_casino_deck     */
extern u_char g_bj_reshuffle;  /* the shoe ran out: shuffle before the next hand */
extern u_char g_bj_play_step;  /* BJ_PLAY's own step                 */
extern u_char g_bj_label_n[3]; /* pieces each hand's label shows: dealer, hand 1, hand 2 */
extern u_int  g_bj_jackpot;

/* The HUD's digits. */
extern CasinoLayout g_bj_money_digits[8];
extern CasinoLayout g_bj_jackpot_digits[7]; /* six digits and the point */
extern CasinoLayout g_bj_bet_digits[2];
extern CasinoLayout g_bj_win_digits[4];

/* The table's pieces, as the game lists them. */
extern CasinoLayoutDef D_8009ACE4[]; /* the dealer's label */
extern CasinoLayoutDef D_8009AD88[]; /* the player's       */
extern CasinoLayoutDef D_8009B2B4[];
extern CasinoLayoutDef D_8009B6D8[];
extern CasinoLayoutDef D_8009B81C[]; /* the two halves of the betting table */
extern CasinoLayoutDef D_8009B874[];
extern CasinoLayoutDef D_8009B97C[]; /* the insurance prompt */
extern CasinoLayoutDef D_8009B9E4[];
extern CasinoLayoutDef D_8009BA64[];
extern CasinoObj       D_8009B7B0[];
extern CasinoModel     D_8009B7C0;

#endif
