/* Persona 1 (JP) - CASINO's grid cursors.
 *   0x80066904 CasinoCursorStep
 *   0x80066AD8 CasinoCursorRepeat
 *   0x80066DC4 CasinoCursorClear
 *   0x80066E00 CasinoKeyRepeat
 *
 * A cursor moves a cell per press, or (CasinoCursorRepeat) per press and
 * then at a steady rate while the button is held. `mode` says which way
 * round left and right go: 2 is the usual way, 1 mirrors them.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

/* Per direction: set when the key fires this frame, and how long it has
   been held. */
extern s8 g_casino_key_fire[4];
extern s8 g_casino_key_hold[4];

/* How long a key is held before it starts repeating. */
#define REPEAT_DELAY 31

void CasinoKeyRepeat(s8 *fire, s8 *hold, int held, short rate);

void CasinoCursorStep(CasinoCursor *c, u_char mode)
{
    int left;
    int right;

    left = 0xFF;
    right = 0xFF;
    if (mode == 2) {
        left = PAD_LEFT;
        right = PAD_RIGHT;
    } else if (mode == 1) {
        left = PAD_RIGHT;
        right = PAD_LEFT;
    }

    c->prev_x = c->x;
    if (g_casino_pad_trig & left) {
        if (--c->x < 0) {
            if (c->wrap_x == 1) {
                c->x = c->w - 1;
            } else {
                c->x = 0;
            }
        }
    }
    if (g_casino_pad_trig & right) {
        if (++c->x >= c->w) {
            if (c->wrap_x == 1) {
                c->x = 0;
            } else {
                c->x = c->w - 1;
            }
        }
    }

    c->prev_y = c->y;
    if ((g_casino_pad_trig & PAD_UP) || (g_casino_pad_trig & PAD_DOWN)) {
        if (g_casino_pad_trig & PAD_DOWN) {
            if (++c->y >= c->h) {
                if (c->wrap_y == 1) {
                    c->y = 0;
                } else {
                    c->y = c->h - 1;
                }
            }
        }
        if (g_casino_pad_trig & PAD_UP) {
            if (--c->y < 0) {
                if (c->wrap_y == 1) {
                    c->y = c->h - 1;
                } else {
                    c->y = 0;
                }
            }
        }
    }
}

void CasinoCursorRepeat(CasinoCursor *c, u_char mode, short rate)
{
    if (mode == 2) {
        CasinoKeyRepeat(&g_casino_key_fire[0], &g_casino_key_hold[0], g_casino_pad & PAD_RIGHT, rate);
        CasinoKeyRepeat(&g_casino_key_fire[1], &g_casino_key_hold[1], g_casino_pad & PAD_LEFT, rate);
    } else if (mode == 1) {
        CasinoKeyRepeat(&g_casino_key_fire[0], &g_casino_key_hold[1], g_casino_pad & PAD_LEFT, rate);
        CasinoKeyRepeat(&g_casino_key_fire[1], &g_casino_key_hold[0], g_casino_pad & PAD_RIGHT, rate);
    }
    CasinoKeyRepeat(&g_casino_key_fire[2], &g_casino_key_hold[3], g_casino_pad & PAD_DOWN, rate);
    CasinoKeyRepeat(&g_casino_key_fire[3], &g_casino_key_hold[2], g_casino_pad & PAD_UP, rate);

    c->prev_x = c->x;
    if (g_casino_key_fire[1]) {
        if (--c->x < 0) {
            if (c->wrap_x == 1) {
                c->x = c->w - 1;
            } else {
                c->x = 0;
            }
        }
        g_casino_key_fire[1] = 0;
        g_casino_key_hold[1] = 0;
    }
    if (g_casino_key_fire[0]) {
        if (++c->x == c->w) {
            if (c->wrap_x == 1) {
                c->x = 0;
            } else {
                c->x = c->w - 1;
            }
        }
        g_casino_key_fire[0] = 0;
        g_casino_key_hold[0] = 0;
    }

    c->prev_y = c->y;
    if (g_casino_key_fire[2]) {
        if (++c->y >= c->h) {
            if (c->wrap_y == 1) {
                c->y = 0;
            } else {
                c->y = c->h - 1;
            }
        }
        g_casino_key_fire[2] = 0;
    }
    if (g_casino_key_fire[3]) {
        if (--c->y < 0) {
            if (c->wrap_y == 1) {
                c->y = c->h - 1;
            } else {
                c->y = 0;
            }
        }
        g_casino_key_fire[3] = 0;
    }
}

void CasinoCursorClear(CasinoCursor *c)
{
    c->x = 0;
    c->y = 0;
    c->prev_x = 0;
    c->prev_y = 0;
    c->wrap_x = 0;
    c->wrap_y = 0;
    c->unk8 = 0;
    c->unk9 = 0;
    c->w = 0;
    c->h = 0;
    c->unkA = 0;
    c->unkB = 0;
    c->unkC = 0;
    c->unkD = 0;
}

/* Fires on the first frame a key is down, then every `rate` frames once it
   has been held for REPEAT_DELAY. */
void CasinoKeyRepeat(s8 *fire, s8 *hold, int held, short rate)
{
    if (held) {
        (*hold)++;
    } else {
        *hold = 0;
    }
    if (*hold == 1 || (*hold >= REPEAT_DELAY && g_casino_timer % rate == 0)) {
        if (*hold >= REPEAT_DELAY) {
            *hold = REPEAT_DELAY;
        }
        (*fire)++;
    }
}
