#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_80082C80(void);
int g_casino_timer;
u_char g_casino_step, g_casino_max_bet, g_casino_pay_digit, g_casino_win_digits[8];
u_int g_casino_lamps;
s8 g_casino_key_fire[16], g_casino_key_hold[16];
u_char D_800AF834[16], D_800A85F8[64];
short D_80099A20[12];
CasinoPalAnim D_800999C0;
extern u_char reel_entry_palette_on __asm__("D_800999C0+0x10");
CasinoLayoutDef D_800A1764, D_800A17B8;
static CasinoLayout panels[3];
static int bad, mutation, count, events[16];
static void event(int e) { if (count < 16) events[count] = e; else bad = 1; count++; }
void CasinoCursorClear(CasinoCursor *c) {
    event(c == (CasinoCursor *)g_casino_key_fire ? 1 : 2);
    if (c != (CasinoCursor *)g_casino_key_fire && c != (CasinoCursor *)g_casino_key_hold) bad = 1;
    if (mutation && count == 1) g_casino_timer = 33;
}
u_short GetClut(int x, int y) { event(3); if (x != -3 || y != 500) bad = 1; return 0xABCD; }
void CasinoQueueCluts(short first, short n, int clut) {
    event(4); if (first != 537 || n != 21 || clut != 0xABCD) bad = 1;
}
void CasinoSpritesSetOn(short first, short n, int on) {
    event(first == 426 ? 5 : first == 439 ? 6 : 10);
    if (on != 1 || !((first == 426 && n == 13) || (first == 439 && n == 12) || (first == 405 && n == 14))) bad = 1;
}
void CasinoTween(CasinoLayout *l, int dx, short dy, short dw, short dh, short frames) {
    event(dx == 320 ? 7 : 8);
    if (dy || dw || dh || frames != 32) bad = 1;
    if (dx == 320) {
        if (l != &panels[0]) bad = 1;
        if (mutation) D_800A17B8.l = &panels[2];
    } else if (dx != -320 || l != &panels[mutation ? 2 : 1]) bad = 1;
}
void CasinoStartPalAnim(CasinoPalAnim *p) { event(9); if (p != &D_800999C0) bad = 1; }

int main(void) {
    const int timers[] = {-100, -1, 0, 1, 16, 32, 33, 34, 2147483647};
    const int flags[] = {0, 1, 255};
    int t, f, change, i, init, done, n, expected[16], total = 0;
    for (t = 0; t < 9; t++) for (f = 0; f < 3; f++) for (change = 0; change < 2; change++) {
        g_casino_timer = timers[t]; reel_entry_palette_on = flags[f]; mutation = change;
        g_casino_step = 99; g_casino_max_bet = 7; g_casino_pay_digit = 8; g_casino_lamps = 0xA5F0;
        for (i = 0; i < 8; i++) g_casino_win_digits[i] = 17;
        for (i = 0; i < 16; i++) D_800AF834[i] = 23;
        for (i = 0; i < 64; i++) D_800A85F8[i] = 29;
        D_80099A20[4] = -3; D_80099A20[5] = 500;
        D_800A1764.l = &panels[0]; D_800A17B8.l = &panels[1];
        count = bad = n = 0; init = timers[t] == 0; done = timers[t] >= 33 || (init && change);
        if (init) for (i = 1; i <= 8; i++) expected[n++] = i;
        if (done) { if (!flags[f]) expected[n++] = 9; expected[n++] = 10; }
        func_80082C80();
        if (bad || count != n || g_casino_timer != (done ? -1 : timers[t]) || g_casino_step != (done ? 38 : 99)) return 1;
        for (i = 0; i < n; i++) if (events[i] != expected[i]) return 2;
        if (g_casino_max_bet != (init ? 0 : 7) || g_casino_pay_digit != (init ? 0 : 8) || g_casino_lamps != (init ? 0xA5F1 : 0xA5F0)) return 3;
        for (i = 0; i < 8; i++) if (g_casino_win_digits[i] != (init ? 0 : 17)) return 4;
        for (i = 0; i < 16; i++) if (D_800AF834[i] != (init && i == 5 ? 0 : 23)) return 5;
        for (i = 0; i < 64; i++) if (D_800A85F8[i] != (init && i >= 26 && i <= 38 ? 0 : 29)) return 6;
        total++;
    }
    if (total != 54) return 7;
    puts("54 native reel-entry fixtures passed"); return 0;
}
