/* Execute the production C with host pointers in place of PS1 linker aliases. */
#define __asm__(symbol)
#include "../../src/casino/game/reeltex.c"
#undef __asm__
extern int printf(const char *, ...);
CasinoLayout *symbol_layouts[256][2];
static CasinoLayout layouts[256];
static short cells[17];
static int chosen, calls, bad;
static CasinoSymbolBlock expected;

void CasinoTexLayout(CasinoLayout *layout)
{
    int i, row, col, value;
    calls++;
    if (layout != &layouts[chosen]) bad = 1;
    for (i = 0; i < 17; i++) {
        value = 12345;
        if (i >= 1 && i <= 13) {
            row = (i - 1) / 5; col = (i - 1) % 5;
            if (col < 3) {
                u_char symbol = col == 0 ? expected.rows[row].a :
                    col == 1 ? expected.rows[row].b : expected.rows[row].c;
                value = (symbol < 128 ? symbol : (int)symbol - 256) + 37;
            }
        }
        if (cells[i] != value) bad = 1;
    }
}

int main(void)
{
    int i, row, value, high, count = 0;
    const int upper[] = {0, 256, -256, 0x123400};
    for (i = 0; i < 256; i++) {
        layouts[i].tex_idx = 0;
        symbol_layouts[i][0] = &layouts[i];
        symbol_layouts[i][1] = 0;
    }
    for (chosen = 0; chosen < 256; chosen += 17)
        for (value = 0; value < 256; value++) for (high = 0; high < 4; high++) {
            for (row = 0; row < 3; row++) {
                expected.rows[row].a = (u_char)(value + row * 73);
                expected.rows[row].b = (u_char)(value + row * 31 + 1);
                expected.rows[row].c = (u_char)(value + row * 19 + 127);
            }
            for (i = 0; i < 17; i++) cells[i] = 12345;
            layouts[chosen].tex_idx = cells + 1;
            calls = bad = 0;
            func_8008AAF8(chosen + upper[high], expected);
            if (bad || calls != 1) return 1;
            for (i = 0; i < 256; i++) {
                if (symbol_layouts[i][0] != &layouts[i] || symbol_layouts[i][1]) return 2;
                if (i != chosen && layouts[i].tex_idx) return 3;
            }
            layouts[chosen].tex_idx = 0;
            count++;
        }
    if (count != 16384 || sizeof(CasinoSymbolBlock) != 9 || sizeof(CasinoDigitRows) != 12) return 4;
    printf("%d native reel-texture fixtures passed\n", count);
    return 0;
}
