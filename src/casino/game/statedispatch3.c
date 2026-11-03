/* Persona 1 (JP) - dispatch the casino state and run its shared update. */
#include <decomp/types.h>
void func_8008D39C();                                  /* extern */
void func_8008D4C4();                                  /* extern */
void func_8008D620();                                  /* extern */
void func_8008D6A4();                                  /* extern */
void func_8008D6E4();                                  /* extern */
void func_8008D6EC();                                  /* extern */
void func_8008D980();                                  /* extern */
void func_8008DC54();                                  /* extern */
void func_8008DD44();                                  /* extern */
void func_8008DF68();                                  /* extern */
void func_8008E0C0();                                  /* extern */
void func_8008E590();                                  /* extern */
void func_8008E6F0();                                  /* extern */
void func_8008E808();                                  /* extern */
void func_8008EB9C();                                  /* extern */
void func_8008EE00();                                  /* extern */
extern u8 g_casino_step;

void func_8008D250(void) {
    switch (g_casino_step) {
    case 0x11:
        func_8008D39C();
        break;
    case 0x12:
        func_8008D4C4();
        break;
    case 0x26:
        func_8008D620();
        break;
    case 0x70:
        func_8008D6A4();
        break;
    case 0x3C:
        func_8008D6E4();
        break;
    case 0x36:
        func_8008D6EC();
        break;
    case 0x72:
        func_8008D980();
        break;
    case 0x76:
        func_8008DC54();
        break;
    case 0x29:
        func_8008E0C0();
        break;
    case 0x75:
        func_8008DD44();
        break;
    case 0x74:
        func_8008DF68();
        break;
    case 0x33:
        func_8008E590();
        break;
    case 0x73:
        func_8008E808();
        break;
    case 0x31:
        func_8008E6F0();
        break;
    case 0x24:
        func_8008EB9C();
        break;
    }
    if (g_casino_step != 0x11) {
        func_8008EE00();
    }
}
