/* Persona 1 (JP) - the five character stats.  ADV only.
 *   0x800B0708 CharStat
 *   0x800B078C CharStatAdd
 *
 * A character carries the stat and a baseline for it, and every change moves
 * the pair together. What separates them is the status screen, which draws the
 * baseline as the bar and the difference on top of it.
 */
#include <types.h>
#include <persona/common/char.h>

u_char CharStat(u_char chr, u_char stat)
{
    Char   *c;
    u_char  value;

    c = &g_chars[chr];
    switch (stat) {
    case 0:
        value = c->stat[0];
        break;
    case 1:
        value = c->stat[1];
        break;
    case 2:
        value = c->stat[2];
        break;
    case 3:
        value = c->stat[3];
        break;
    case 4:
        value = c->stat[4];
        break;
    }
    return value;
}

void CharStatAdd(u_char chr, u_char stat, u_char amount, u_char take)
{
    Char *c;

    c = &g_chars[chr];
    if (take) {
        switch (stat) {
        case 0:
            c->stat[0] -= amount;
            c->stat_base[0] -= amount;
            break;
        case 1:
            c->stat[1] -= amount;
            c->stat_base[1] -= amount;
            break;
        case 2:
            c->stat[2] -= amount;
            c->stat_base[2] -= amount;
            break;
        case 3:
            c->stat[3] -= amount;
            c->stat_base[3] -= amount;
            break;
        case 4:
            c->stat[4] -= amount;
            c->stat_base[4] -= amount;
            break;
        }
    } else {
        switch (stat) {
        case 0:
            c->stat[0] += amount;
            c->stat_base[0] += amount;
            break;
        case 1:
            c->stat[1] += amount;
            c->stat_base[1] += amount;
            break;
        case 2:
            c->stat[2] += amount;
            c->stat_base[2] += amount;
            break;
        case 3:
            c->stat[3] += amount;
            c->stat_base[3] += amount;
            break;
        case 4:
            c->stat[4] += amount;
            c->stat_base[4] += amount;
            break;
        }
    }
}
