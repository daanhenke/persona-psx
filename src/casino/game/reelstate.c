/* Persona 1 (JP) - dispatch the reel state and run its shared update. */
#include <decomp/types.h>
void func_800829F4();                                  /* extern */
void func_80082C80();                                  /* extern */
void func_80082E1C();                                  /* extern */
void func_80083224();                                  /* extern */
void func_80083398();                                  /* extern */
void func_80083934();                                  /* extern */
void func_80083B14();                                  /* extern */
void func_80083B1C();                                  /* extern */
void func_80083BFC();                                  /* extern */
void func_80083DA8();                                  /* extern */
void func_80083F8C();                                  /* extern */
void func_8008634C();                                  /* extern */
void func_8008802C();                                  /* extern */
void func_800885C4();                                  /* extern */
extern u8 g_casino_step;

void func_800828DC(void) {
    switch (g_casino_step) {
    case 0x11:
        func_800829F4();
        break;
    case 0x12:
        func_80082C80();
        break;
    case 0x26:
        func_80082E1C();
        break;
    case 0x70:
        func_80083224();
        break;
    case 0x71:
    case 0x73:
        func_80083398();
        break;
    case 0x72:
        func_80083934();
        break;
    case 0x14:
        func_80083B1C();
        break;
    case 0x74:
        func_80083BFC();
        break;
    case 0x75:
        func_80083DA8();
        break;
    case 0x76:
        func_80083F8C();
        break;
    case 0x3C:
        func_80083B14();
        break;
    case 0x77:
        func_8008802C();
        break;
    case 0x13:
        func_800885C4();
        break;
    }
    func_8008634C();
}
