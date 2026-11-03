#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_800844D4(int), func_80085090(s8, int), func_80084FC4(s8, int);
/* Keep the original 24-byte records; mocks never interpret host pointer fields. */
char D_80099B10[14 * 24] __attribute__((aligned(8)));
char D_80099A50[8 * 24] __attribute__((aligned(8)));
__asm__(".globl D_80099A98\n.set D_80099A98,D_80099A50+72");
static char *bank;
static int routine;
s8 D_800A8620, D_800A85F8[64];
static int expected_rank, expected_slot, mutation, events, bad, index;

static void change_rank(void) {
    D_800A8620 = (expected_rank + 1) % 7;
    D_800A85F8[index + 13] = (expected_rank + 1) % 7;
    if (routine == 2) D_800A85F8[index] = (expected_rank + 1) % 4 + 3;
}
u_short GetClut(int x, int y) {
    if (events != 0 || x != -1024 + expected_slot * 9 || y != 510 - expected_slot * 3) bad = 1;
    events++;
    if (mutation == 1) change_rank();
    return 0xFEDC;
}
void CasinoQueueCluts(short first, short count, int clut) {
    if (events != 1 || first != (routine == 2 ? expected_rank * 8 + 160 : expected_rank + 153) || count != 1 || clut != 0xFEDC) bad = 1;
    events++;
    if (mutation == 2) change_rank();
}
void CasinoStartPalAnim(CasinoPalAnim *p) {
    if (events != 2 || (char *)p != bank + expected_slot * 24) bad = 1;
    events++;
}
int main(void) {
    const int active[] = {-256, -255, -1, 0, 1, 128, 255, 256, 257, 65536};
    int rank, mode, change, i, j, slots, count = 0;
    for (routine = 0; routine < 3; routine++) for (index = 0; index < (routine ? 13 : 1); index++)
    for (rank = 0; rank < (routine == 2 ? 4 : 7); rank++) for (mode = 0; mode < 10; mode++) for (change = 0; change < 3; change++) {
        bank = routine == 2 ? D_80099A50 : D_80099B10;
        slots = routine == 2 ? 8 : 14;
        for (i = 0; i < slots; i++) {
            for (j = 0; j < 24; j++) bank[i * 24 + j] = 17;
            *(short *)(bank + i * 24 + 8) = -1024 + i * 9;
            *(short *)(bank + i * 24 + 10) = 510 - i * 3;
        }
        for (i = 0; i < 64; i++) D_800A85F8[i] = 0;
        D_800A8620 = rank; D_800A85F8[index + 13] = rank;
        if (routine == 2) D_800A85F8[index] = rank + 3;
        expected_rank = rank; expected_slot = rank + ((active[mode] & 255) ? 0 : (routine == 2 ? 4 : 7));
        mutation = change; events = bad = 0;
        if (routine == 2) func_80084FC4(index, active[mode]);
        else if (routine) func_80085090(index, active[mode]); else func_800844D4(active[mode]);
        if (bad || events != 3) return 1;
        for (i = 0; i < slots; i++) for (j = 0; j < 24; j++)
            if ((j < 8 || j >= 12) && bank[i * 24 + j] != 17) return 2;
        count++;
    }
    if (count != 4500) return 3;
    puts("4500 native rank-palette fixtures passed"); return 0;
}
