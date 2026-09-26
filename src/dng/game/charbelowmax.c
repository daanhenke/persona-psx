/* Persona 1 (JP) - whether a party member's value is short of its cap.
 *   DNG 0x8008F3E0
 *
 * The last of the item-test unit (common/game/status.c, itemusableany.c,
 * common/game/partystatus.c); PartyAnyBelowMax loops it over the party. ADV
 * carries the same routine at 0x80094B0C, still in asm; once it takes this
 * source the unit belongs in src/common.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* The caps the maxima and the five base stats stop at. */
#define CAP_MAX  999
#define CAP_STAT 99

/* 0 and 1 test HP and SP against their maxima, 2 and 3 the maxima against
   999, 4..8 the base stats against 99; 9 tests HP itself against 999. */
u_char CharBelowMax(u_char slot, u_char kind)
{
    Char  *c;
    u_char below;

    below = 0;
    c = &g_chars[g_party[slot]];
    switch (kind) {
    case 0:
        if (c->hp != c->hp_max) {
            below = 1;
        }
        break;
    case 1:
        if (c->sp != c->sp_max) {
            below = 1;
        }
        break;
    case 2:
        if (c->hp_max != CAP_MAX) {
            below = 1;
        }
        break;
    case 3:
        if (c->sp_max != CAP_MAX) {
            below = 1;
        }
        break;
    case 4:
        if (c->stat_base[0] != CAP_STAT) {
            below = 1;
        }
        break;
    case 5:
        if (c->stat_base[1] != CAP_STAT) {
            below = 1;
        }
        break;
    case 6:
        if (c->stat_base[2] != CAP_STAT) {
            below = 1;
        }
        break;
    case 7:
        if (c->stat_base[3] != CAP_STAT) {
            below = 1;
        }
        break;
    case 8:
        if (c->stat_base[4] != CAP_STAT) {
            below = 1;
        }
        break;
    case 9:
        if (c->hp != CAP_MAX) {
            below = 1;
        }
        break;
    }
    return below;
}
