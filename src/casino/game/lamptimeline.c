/* Persona 1 (JP) - sequence the casino indicator lamps. */
#include <decomp/types.h>
extern s8 D_800A966C;
extern s8 D_800A9670;
extern s16 lamp_frame __asm__("D_800AF9E8+0xD4");
extern short g_casino_seqs[];
extern short g_casino_vab;
extern void CasinoSpritesSetOn(int, int, int);
extern void CasinoPlaySeq(short *, u_int, short);
void func_8008C4D0(void) {
    switch (lamp_frame) {
    case 0x0:
        CasinoSpritesSetOn(0x10, 1, 0);
        CasinoSpritesSetOn(0x11, 1, 1);
        CasinoPlaySeq(g_casino_seqs, 0x12E818, g_casino_vab);
        break;
    case 0x10:
    case 0x30:
        CasinoSpritesSetOn(0x11, 1, 1);
        break;
    case 0x8:
    case 0x18:
    case 0x28:
    case 0x38:
        CasinoSpritesSetOn(0x11, 1, 0);
        break;
    case 0x20:
    case 0x40:
        CasinoSpritesSetOn(0x11, 1, 1);
        CasinoPlaySeq(g_casino_seqs, 0x12E818, g_casino_vab);
        break;
    case 0x48:
        CasinoSpritesSetOn(0x11, 1, 0);
        CasinoSpritesSetOn(0x10, 1, 1);
        D_800A9670 = 0;
        D_800A966C = 1;
        break;
    }
    lamp_frame = (s16) ((u16) lamp_frame + 1);
}
