/* Return the selected bet to the casino balance when its refund flag is set.
 * func_80090448 @ 0x80090448, JP1 CASINO.
 */
extern signed char D_800AF73C[], g_casino_cursor[];
extern unsigned int g_casino_money, D_800A96A4[];
extern short g_casino_seqs[];
extern void SsSeqStop(short), SsSeqPlay(short, short, short);

void func_80090448(void)
{
    unsigned int *entry;
    unsigned int value, total, remaining;
    int index;

    index = g_casino_cursor[1];
    if (D_800AF73C[1]) {
        /* Retaining the table base separately preserves GCC 2.6's original
           register assignment for the scaled index and selected entry. */
        {
            unsigned int *table = D_800A96A4;
            entry = table + index;
        }
        D_800AF73C[1] = 0;
        value = *entry;
        if (value >= 10) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            /* Reload after the sound calls, as the original does. */
            remaining = *entry - value;
            total = value + g_casino_money;
            *entry = remaining;
            g_casino_money = total;
        }
    }
}
