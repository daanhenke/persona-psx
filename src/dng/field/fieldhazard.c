/* Persona 1 (JP) - floors that hurt.  DNG only.
 *   0x8006C2FC FieldDamageFloor
 *   0x8006C51C FieldPoisonFloor
 *
 * Every party member rolls against luck: a byte of rand() above two and a
 * half times the member's luck, and the floor takes effect on them. Neither
 * does anything while a step-limited effect is running or has just run out.
 * Any hit plays the floor's sound and flashes the field twice, with a shake.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include <persona/common/char.h>
#include <persona/common/status.h>
#include <persona/dng/field.h>

#define PARTY_SLOTS 5
#define NO_MEMBER   0xFF

/* A member is hit when this is true. The luck converts as an unsigned char,
   which gcc 2.6 does by converting it signed and adding 256 if negative. */
#define LUCK_ROLL_FAILS(luck) ((double)(rand() % 256) > (luck) * 2.5)

/* Flashes the ambient light to (r, g, b) and back twice, shaking the view
   20, -40, 40, -20. */
#define FLASH(r, g, b)                                                         \
    {                                                                          \
        FieldNudge(1, 20);                                                  \
        GsSetAmbient(r, g, b);                                                 \
        FieldNudge(1, -40);                                                 \
        GsSetAmbient(0x1000, 0x1000, 0x1000);                                  \
        FieldNudge(1, 40);                                                  \
        GsSetAmbient(r, g, b);                                                 \
        FieldNudge(1, -20);                                                 \
        GsSetAmbient(0x1000, 0x1000, 0x1000);                                  \
    }

/* Takes hp / `div` from each member who fails the roll - never below 1, and
   with `div` 1, one hp from anyone above 1. */
/* 98.8%: the luck's conversion to double moves its value into a0 after the
   constant for the unsigned fix-up, so reorg cannot put it in the delay
   slot of the sign test as the image does. Every spelling of the roll gives
   the same order; FieldPoisonFloor shares the row. */
#ifdef NON_MATCHING
void FieldDamageFloor(int div)
{
    int   i;
    int   hit;
    u_int m;
    Char *c;

    hit = 0;
    if (EFFECT_STEPS != 0 || g_effect_over != 0) {
        return;
    }
    for (i = 0; i < PARTY_SLOTS; i++) {
        m = g_party[i];
        if (m != NO_MEMBER) {
            c = &g_chars[m];
            if (LUCK_ROLL_FAILS(g_chars[m].stat[STAT_LUCK])) {
                hit = 1;
                if (div == 1 && c->hp != 1) {
                    c->hp--;
                } else {
                    c->hp -= c->hp / div;
                    if (c->hp <= 0) {
                        c->hp = 1;
                    }
                }
            }
        }
    }
    if (hit) {
        FieldPlayJingle(0x1B, 1);
        FLASH(0x1000, 0, 0);
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldhazard", FieldDamageFloor);
#endif

/* Poisons each member who fails the roll. */
/* 96.6%: the conversion row above, the record's address summed base first,
   and the sound's first argument loaded after the flag's value. */
#ifdef NON_MATCHING
void FieldPoisonFloor(void)
{
    int   i;
    int   hit;
    u_int m;
    Char *c;
    Char *base;

    hit = 0;
    if (EFFECT_STEPS != 0) {
        return;
    }
    base = g_chars;
    for (i = 0; i < PARTY_SLOTS; i++) {
        m = g_party[i];
        if (m != NO_MEMBER) {
            if (LUCK_ROLL_FAILS((c = &base[m])->stat[STAT_LUCK])) {
                hit = 1;
                c->status = STATUS_POISON;
            }
        }
    }
    if (hit) {
        D_800993C6 = 0x80;
        FieldPlayJingle(0x1C, 1);
        FLASH(0, 0x1000, 0);
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldhazard", FieldPoisonFloor);
#endif
