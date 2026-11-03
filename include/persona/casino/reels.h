#ifndef PERSONA_CASINO_REELS_H
#define PERSONA_CASINO_REELS_H

#include <persona/casino/casino.h>

/* The 21 reel entries, two layout banks, timing and byte-state tables. */
typedef struct {
 u_char header[8]; CasinoLayout *layouts[2][21];
 u_char state[21],next[21]; unsigned short ticks[21]; u_char frames[21];
 signed char delta[21]; u_char flags[21],symbols[21],expected[21],unused[21],reset[21];
 signed char previous[21]; u_char trailing[4];
} ReelState;

#endif
