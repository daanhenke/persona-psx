/* Persona 1 (JP) - dispatch the casino state and run its shared update. */
#include <decomp/types.h>
void func_80088818();                                  /* extern */
void func_80088954();                                  /* extern */
void func_80088A28();                                  /* extern */
void func_80088C8C();                                  /* extern */
void func_80088E64();                                  /* extern */
void func_80089154();                                  /* extern */
void func_8008915C();                                  /* extern */
void func_8008945C();                                  /* extern */
void func_800895EC();                                  /* extern */
void func_800899F8();                                  /* extern */
void func_80089B68();                                  /* extern */
void func_80089C04();                                  /* extern */
void func_80089E7C();                                  /* extern */
void func_8008A010();                                  /* extern */
void func_8008A628();                                  /* extern */
extern u8 g_casino_step;

void func_800886F0(void) {
    switch (g_casino_step) {
    case 0x11:
        func_80088818();
        break;
    case 0x12:
        func_80088954();
        break;
    case 0x26:
        func_80088A28();
        break;
    case 0x1E:
        func_80088C8C();
        break;
    case 0x70:
        func_80088E64();
        break;
    case 0x71:
        func_8008915C();
        break;
    case 0x72:
        func_8008945C();
        break;
    case 0x73:
        func_800895EC();
        break;
    case 0x74:
        func_800899F8();
        break;
    case 0x75:
        func_80089B68();
        break;
    case 0x76:
        func_80089C04();
        break;
    case 0x77:
        func_80089E7C();
        break;
    case 0x78:
        func_8008A010();
        break;
    case 0x3C:
        func_80089154();
        break;
    }
    func_8008A628();
}
