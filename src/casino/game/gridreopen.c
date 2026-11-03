#include <persona/casino/casino.h>

extern u_char initial_symbols[] __asm__("D_800A2BC8+0x4");
extern CasinoLayout *entry_layout __asm__("D_800A2BC8+0x2B30");
extern CasinoObj *entry_object __asm__("D_800A2BC8+0x2C14");
extern CasinoObj *cursor_object __asm__("D_800A2BC8+0x31E0");
extern CasinoLayoutDef frame_defs[] __asm__("D_800A2BC8+0x2AD8");
extern CasinoLayoutDef left_defs[] __asm__("D_800A2BC8+0x2C64");
extern CasinoLayoutDef middle_defs[] __asm__("D_800A2BC8+0x2CC0");
extern CasinoLayoutDef right_defs[] __asm__("D_800A2BC8+0x2D1C");
extern u_char flag94 __asm__("D_800AAE94");
extern u_char flag95 __asm__("D_800AAE94+0x1");
extern u_char flag96 __asm__("D_800AAE94+0x2");
extern u_short word98 __asm__("D_800AAE98");
extern u_char flag9A __asm__("D_800AAE98+0x2");
extern u_char flag9B __asm__("D_800AAE98+0x3");
extern u_char flag9C __asm__("D_800AAE9C");
extern u_char flag9D __asm__("D_800AAE9C+0x1");
extern u_char flag9E __asm__("D_800AAE9C+0x2");
extern u_char flagA5 __asm__("D_800AAE9C+0x9");
extern u_char flagA7 __asm__("D_800AAE9C+0xB");
extern u_char flagA9 __asm__("D_800AAEA8+0x1");
extern void func_80091154(int, int);
extern void func_80093178(void);
extern void CasinoTween();
extern void CasinoStartAnim();
extern void CasinoSpritesSetOn();
extern void CasinoBuildLayouts(CasinoLayoutDef *, int);

void func_8008EB9C(void)
{
    int i;

    i = 0;
    do
    {
        func_80091154(i & 255, initial_symbols[i]);
        i++;
    } while (i < 11);
    CasinoTween(entry_layout, -320, 0, 0, 0, 1);
    CasinoStartAnim(entry_object, 1, 0, 0, 0, 320, 0, 0, 0, 0, 0);
    CasinoStartAnim(cursor_object, 12, 0, 0, 0, 24, 0, 0, 0, 0, 0);
    CasinoSpritesSetOn(0xED, 15, 1);
    CasinoSpritesSetOn(0xFC, 15, 1);
    CasinoSpritesSetOn(0x10B, 15, 1);
    CasinoSpritesSetOn(0x11A, 16, 1);
    CasinoSpritesSetOn(0x12A, 24, 0);
    CasinoSpritesSetOn(0x142, 7, 0);
    CasinoSpritesSetOn(0x149, 6, 0);
    CasinoSpritesSetOn(0x14F, 12, 0);
    CasinoSpritesSetOn(0x15B, 8, 0);
    CasinoSpritesSetOn(0x163, 6, 0);
    CasinoSpritesSetOn(0x169, 8, 0);
    flag95 = 0;
    flag94 = 0;
    flag9B = 0;
    flag9A = 0;
    flag96 = 0;
    word98 = 0;
    flag9D = 0;
    flag9C = 0;
    CasinoBuildLayouts(frame_defs, 0);
    CasinoBuildLayouts(left_defs, 0);
    CasinoBuildLayouts(middle_defs, 0);
    CasinoBuildLayouts(right_defs, 0);
    flagA9 = 0;
    CASINO_LAMPS.b0 = 1;
    CASINO_LAMPS.b2 = 0;
    func_80093178();
    g_casino_step = 0x12;
    flag9E = 0;
    flagA5 = 0;
    flagA7 = 0;
    g_casino_timer = -1;
}
