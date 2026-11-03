#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_800829F4(void);
CasinoLayoutDef D_800A1A54[10], D_800A20EC[32];
u_char *D_800B3E28[5], *D_800B3E3C[16];
s8 D_800AF834[400];
char D_8009E8E8[1];
int g_casino_timer;
u_char g_casino_step;
static CasinoLayout layouts[42];
static short indices[42][3];
static u_char tables[21][256];
static int events[16], used, bad, mutation, replacement;
static void event(int id) {
    if (used >= 16) { bad = 1; return; }
    events[used++] = id;
    if (id == mutation) g_casino_timer = replacement;
}
void CasinoGame3LoadVram0(void) { event(0); }
void CasinoGame3LoadVram1(void) { event(1); }
void CasinoGame3LoadVram2(void) { event(2); }
void CasinoGame3LoadVram3(void) { event(3); }
void CasinoGame3LoadVram4(void) { event(4); }
void CasinoGame3LoadVram5(void) { event(5); }
void CasinoGame3LoadSound(void) { event(6); }
void func_800853F4(void) { event(7); }
void func_8008514C(void) { event(8); }
void CasinoFade(int first, int count, u_char r, u_char g, u_char b, short frames) {
    if (first != 0 || count != 1000 || r != 128 || g != 128 || b != 128 || frames != 32) bad = 1;
    event(9);
}
void func_80087C98(void *p) {
    if (p != D_8009E8E8) bad = 1;
    event(10);
}
void CasinoQueueSemi(short first, short count, u_char mode) {
    if (mode != 1 || (first == 516 ? count != 5 : first != 521 || count != 16)) bad = 1;
    event(first == 516 ? 11 : 12);
}
static void reference_event(int id, int *timer, int *n, int *expected) {
    expected[(*n)++] = id;
    if (id == mutation) *timer = replacement;
}
static int check(int frame, int seed) {
    int timer = frame, expected[16], n = 0, initialized = 0, i, j, k;
    for (i = 0; i < 21; i++) {
        for (j = 0; j < 256; j++) tables[i][j] = i * 13 + j * 7 + seed;
        if (i < 5) D_800B3E28[i] = tables[i] + 128;
        else D_800B3E3C[i - 5] = tables[i] + 128;
    }
    for (i = 0; i < 42; i++) {
        indices[i][0] = 901; indices[i][1] = -1234; indices[i][2] = 902;
        layouts[i].tex_idx = &indices[i][1];
        if (i < 10) D_800A1A54[i].l = &layouts[i];
        else D_800A20EC[i - 10].l = &layouts[i];
    }
    for (i = 0; i < 400; i++) D_800AF834[i] = 17;
    for (i = 0; i < 21; i++) D_800AF834[344 + i] = seed + i * 11;
    for (i = 0; i < 6; i++) if (timer == i) reference_event(i, &timer, &n, expected);
    if (timer == 6) {
        reference_event(6, &timer, &n, expected);
        reference_event(7, &timer, &n, expected);
        initialized = 1;
        for (i = 8; i < 13; i++) reference_event(i, &timer, &n, expected);
    }
    g_casino_timer = frame; g_casino_step = 77; used = bad = 0;
    func_800829F4();
    if (bad || used != n) return 1;
    for (i = 0; i < n; i++) if (events[i] != expected[i]) return 2;
    if (g_casino_timer != (timer >= 39 ? -1 : timer) || g_casino_step != (timer >= 39 ? 18 : 77)) return 3;
    for (i = 0; i < 42; i++) {
        k = i / 2;
        j = initialized ? tables[k][128 + D_800AF834[344 + k]] + (i % 2 ? 0 : 8) : -1234;
        if (indices[i][1] != j || indices[i][0] != 901 || indices[i][2] != 902) return 4;
        if (layouts[i].tex_idx != &indices[i][1]) return 5;
    }
    for (i = 0; i < 400; i++)
        if (D_800AF834[i] != (i >= 344 && i < 365 ? (s8)(seed + (i - 344) * 11) : 17)) return 6;
    return 0;
}
int main(void) {
    const int frames[] = {-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 38, 39, 40, 2147483647};
    const int timers[] = {-1, 6, 39};
    int i, j, seed, count = 0;
    for (i = 0; i < 14; i++) for (mutation = -1; mutation < 13; mutation++) for (j = 0; j < 3; j++) {
        replacement = timers[j];
        if (check(frames[i], i * 31 + mutation)) return 1;
        count++;
    }
    mutation = -1;
    for (seed = 0; seed < 256; seed++) {
        if (check(6, seed)) return 2;
        count++;
    }
    if (count != 844) return 3;
    puts("844 native reel-loading fixtures passed"); return 0;
}
