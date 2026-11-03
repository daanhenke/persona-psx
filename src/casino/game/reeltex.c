/* Copy three symbol rows into the first three cells of five-cell texture rows. */
#include <persona/casino/casino.h>
#include <persona/casino/grid.h>

extern CasinoLayout *symbol_layouts[][2] __asm__("D_800A2BC8+0xB68");
extern void CasinoTexLayout(CasinoLayout *);

void func_8008AAF8(int index, CasinoSymbolBlock symbols)
{
    int row = 0;
    do {
        *(short *)((char *)symbol_layouts[index & 255][0]->tex_idx + row * 10) = (s8)symbols.rows[row].a + 37;
        *(short *)((char *)symbol_layouts[index & 255][0]->tex_idx + row * 10 + 2) = (s8)symbols.rows[row].b + 37;
        *(short *)((char *)symbol_layouts[index & 255][0]->tex_idx + row * 10 + 4) = (s8)symbols.rows[row].c + 37;
    } while (++row < 3);
    CasinoTexLayout(symbol_layouts[index & 255][0]);
}
