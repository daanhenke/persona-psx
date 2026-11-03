#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "../../src/casino/game/betreturn.c"
signed char D_800AF73C[4], g_casino_cursor[4];
unsigned int g_casino_money, D_800A96A4[4];
short g_casino_seqs[8];
static int calls, mutate;
void SsSeqStop(short seq) {
    assert(seq == 42 && calls == 0 && D_800AF73C[1] == 0);
    calls++;
}
void SsSeqPlay(short seq, short mode, short repeats) {
    assert(seq == 42 && mode == 1 && repeats == 1 && calls == 1);
    calls++;
    if (mutate) { D_800A96A4[1] += 7; g_casino_money += 3; }
}
static void check(int pending, unsigned bet, unsigned money, int change,
                  unsigned expected_bet, unsigned expected_money, int expected_calls) {
    D_800AF73C[1] = pending; g_casino_cursor[1] = 1;
    D_800A96A4[0] = 123; D_800A96A4[1] = bet; D_800A96A4[2] = 456;
    g_casino_money = money; g_casino_seqs[5] = 42; calls = 0; mutate = change;
    func_80090448();
    assert(D_800AF73C[1] == 0 && D_800A96A4[0] == 123 && D_800A96A4[2] == 456);
    assert(D_800A96A4[1] == expected_bet && g_casino_money == expected_money);
    assert(calls == expected_calls);
}
int main(void) {
    check(0, 100, 20, 0, 100, 20, 0);
    check(1, 0, 20, 0, 0, 20, 0);
    check(1, 9, 20, 0, 9, 20, 0);
    check(1, 10, 20, 0, 0, 30, 2);
    check(-1, 100, 20, 0, 0, 120, 2);
    check(1, 10, 20, 1, 7, 33, 2);
    check(1, UINT_MAX, 20, 0, 0, 19, 2);
    puts("7 native bet-refund fixtures passed");
}
