#include <decomp/types.h>

extern s8 cursor_y __asm__("g_casino_cursor+0x1");
extern s8 cursor_prev_y __asm__("g_casino_cursor+0x3");
extern short g_casino_seqs[];
extern void *cursor_object __asm__("D_800A2BC8+0x31E0");
extern u8 cursor_busy __asm__("D_800AAE9C");
extern u8 tween_flag0 __asm__("D_800AAE94+0x2");
extern u8 tween_flag1 __asm__("D_800AAE98+0x2");
extern short tween_counter __asm__("D_800AAE98");
extern void SsSeqStop(short);
extern void SsSeqPlay(short, short, short);
extern void CasinoStartAnim();
void func_80090DD4(void)
{
    int delta = cursor_y - cursor_prev_y;
    if (delta != 0)
    {
        SsSeqStop(g_casino_seqs[11]);
        SsSeqPlay(g_casino_seqs[11], 1, 1);
        CasinoStartAnim(cursor_object, 1, 0, 0, 0, 0, delta << 4, 0, 0, 0, 0);
        if (cursor_busy == 0)
        {
            tween_flag0 = 1;
            tween_flag1 = 1;
            tween_counter = 0;
        }
    }
}
