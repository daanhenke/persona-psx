/* Persona 1 (JP) - collect qualifying bets and restart their sequence. */
#include <decomp/types.h>
extern u_int D_800A96A4[];
extern u_int g_casino_money;
extern short g_casino_seqs[];
extern s8 collect_pending __asm__("D_800AF738+0x3");
extern void func_80090EEC(u_char);
extern void SsSeqStop(short);
extern void SsSeqPlay(short, short, short);

void func_800904FC(void)
{
    int changed = 0;
    int i;
    u_int *entry;
    u_int value;
    u_int total;
    s8 *pending = &collect_pending;
    if (*pending != 0) {
        *pending = 0;
        i = 0;
        entry = D_800A96A4;
        do {
            value = *entry;
            if (value >= 10) {
                changed++;
                total = g_casino_money;
                *entry = 0;
                g_casino_money = value + total;
                func_80090EEC((u_char)i);
            }
            i++;
            entry++;
        } while (i < 11);
        if ((u_char)changed != 0) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
        }
    }
}
