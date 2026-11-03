/* Persona 1 (JP) - dispatch the blackjack play state. */
#include <persona/casino/blackjack.h>
void func_80079AD8();                                  /* extern */
void func_80079C28();                                  /* extern */
void func_8007A10C();                                  /* extern */
void func_8007A27C();                                  /* extern */
void func_8007A39C();                                  /* extern */
void func_8007A654();                                  /* extern */
void func_8007A77C();                                  /* extern */
void func_8007A8B4();                                  /* extern */
void func_8007A9F8();                                  /* extern */
extern u8 g_bj_play_step;

void func_800799F0(void) {
    switch (g_bj_play_step) {
    case 0x1B:
        func_80079AD8();
        return;
    case 0x70:
        func_80079C28();
        return;
    case 0x71:
        func_8007A10C();
        return;
    case 0x74:
        func_8007D3CC(&g_bj_play_step);
        return;
    case 0x25:
        func_8007A27C();
        return;
    case 0x16:
        func_8007A39C();
        return;
    case 0x17:
        func_8007A654();
        return;
    case 0x19:
        func_8007A77C();
        return;
    case 0x13:
        func_8007A8B4();
        return;
    case 0x30:
        func_8007A9F8();
        /* fallthrough */
    default:
        return;
    }
}
