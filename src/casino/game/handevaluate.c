/* Persona 1 (JP) - update both card-hand evaluations. */
#include <decomp/types.h>
#include <persona/casino/rotation.h>
extern short D_800B0720[];
extern int D_800B0724[];
extern int D_800B0814[];
extern u8 first_hand_state __asm__("D_800B0800+0x2");
extern u8 second_hand_state __asm__("D_800B08F0+0x2");
extern int func_800917F0(short *);
extern short func_8009199C(short *);
extern short func_80091B00(short *);
extern void func_80091EB8(void);
u_char func_8008EAA4(void)
{
    short *second;
    D_800B0724[3] = func_800917F0(D_800B0720);
    D_800B0724[2] = func_8009199C(D_800B0720);
    D_800B0724[1] = func_80091B00(D_800B0720);
    second = D_800B0720 + 120;
    D_800B0720[0] = func_80091B78((CasinoRotationResponse *)D_800B0720);
    D_800B0814[3] = func_800917F0(second);
    D_800B0814[2] = func_8009199C(second);
    D_800B0814[1] = func_80091B00(second);
    D_800B0720[120] = func_80091B78((CasinoRotationResponse *)second);
    func_80091EB8();
    if (first_hand_state == 2) {
        if (second_hand_state == 2) return 0;
    }
    return 1;
}
