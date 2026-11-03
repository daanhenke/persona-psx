/* Add ten coins to the selected bet when its input flag is set.
 * func_80090248 @ 0x80090248, JP1 CASINO.
 */
extern signed char D_800AF738[], g_casino_cursor[];
extern unsigned int g_casino_money, D_800A96A4[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void SsSeqStop(short), SsSeqPlay(short, short, short);
extern void CasinoPlaySeq(short *, unsigned long *, short);

void func_80090248(void)
{
    unsigned int *entry;
    unsigned int value, total;
    int selected = g_casino_cursor[1];

    if (D_800AF738[2]) {
        D_800AF738[2] = 0;
        if (g_casino_money >= 10) {
            /* The separate base retains the original index register. */
            {
                unsigned int *base = D_800A96A4;
                entry = base + selected;
            }
            if (*entry < 100) {
                SsSeqStop(g_casino_seqs[6]);
                SsSeqPlay(g_casino_seqs[6], 1, 1);
                /* Both balances are read after the sound calls. */
                value = *entry;
                total = g_casino_money;
                *entry = value + 10;
                g_casino_money = total - 10;
            }
        } else {
            CasinoPlaySeq(g_casino_seqs, (unsigned long *)0x13B1F8,
                          g_casino_main_vab);
        }
    }
}
