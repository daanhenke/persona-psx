#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "../../src/casino/game/betadd.c"
signed char D_800AF738[4], g_casino_cursor[4];
unsigned int g_casino_money, D_800A96A4[4];
short g_casino_seqs[8], g_casino_main_vab;
static int calls, denial, mutate;
void SsSeqStop(short seq) {
    assert(seq == 42 && calls == 0 && D_800AF738[2] == 0);
    calls++;
}
void SsSeqPlay(short seq, short mode, short repeats) {
    assert(seq == 42 && mode == 1 && repeats == 1 && calls == 1);
    calls++;
    if (mutate) { D_800A96A4[1] += 7; g_casino_money += 3; }
}
void CasinoPlaySeq(short *seqs, unsigned long *data, short vab) {
    assert(seqs == g_casino_seqs && (uintptr_t)data == 0x13B1F8 && vab == 73);
    assert(D_800AF738[2] == 0 && calls == 0 && denial == 0);
    denial++;
}
static void check(int pending, unsigned bet, unsigned money, int change,
                  unsigned expected_bet, unsigned expected_money,
                  int expected_calls, int expected_denial) {
    D_800AF738[2] = pending; g_casino_cursor[1] = 1;
    D_800A96A4[0] = 123; D_800A96A4[1] = bet; D_800A96A4[2] = 456;
    g_casino_money = money; g_casino_seqs[6] = 42; g_casino_main_vab = 73;
    calls = denial = 0; mutate = change;
    func_80090248();
    assert(D_800AF738[2] == 0 && D_800A96A4[0] == 123 && D_800A96A4[2] == 456);
    assert(D_800A96A4[1] == expected_bet && g_casino_money == expected_money);
    assert(calls == expected_calls && denial == expected_denial);
}
int main(void) {
    check(0, 0, 20, 0, 0, 20, 0, 0);
    check(1, 0, 0, 0, 0, 0, 0, 1);
    check(1, 0, 9, 0, 0, 9, 0, 1);
    check(1, 0, 10, 0, 10, 0, 2, 0);
    check(-1, 90, 20, 0, 100, 10, 2, 0);
    check(1, 99, 20, 0, 109, 10, 2, 0);
    check(1, 100, 20, 0, 100, 20, 0, 0);
    check(1, UINT_MAX, 20, 0, UINT_MAX, 20, 0, 0);
    check(1, 10, 20, 1, 27, 13, 2, 0);
    puts("9 native bet-add fixtures passed");
}
