/* Swap unequal active reel rows after the four fixed rows. */
typedef struct
{
    unsigned char cards[12][5];
    signed char   active[13], value[13];
    unsigned char dirty[13];
} ReelRows;

extern ReelRows      D_800A85BC;
extern unsigned char g_casino_bet;

void func_80087964(void)
{
    int count = g_casino_bet;
    int i, j;
    unsigned char first[2][5], second[2][1], third[2][1];

    if (count >= 5U)
        count = (short)(count - 4) + g_casino_bet;
    for (i = 4; i < count; i += 2)
    {
        if (D_800A85BC.active[i] && D_800A85BC.active[i + 1] && D_800A85BC.cards[i + 1][0])
        {
            if (D_800A85BC.value[i] != D_800A85BC.value[i + 1])
            {
                D_800A85BC.dirty[i] = D_800A85BC.dirty[i + 1] = 1;
                for (j = 0; j < 5; j++)
                {
                    first[0][j] = D_800A85BC.cards[i][j];
                    first[1][j] = D_800A85BC.cards[i + 1][j];
                    D_800A85BC.cards[i + 1][j] = first[0][j];
                    D_800A85BC.cards[i][j] = first[1][j];
                }
                second[0][0] = D_800A85BC.active[i];
                second[1][0] = D_800A85BC.active[i + 1];
                D_800A85BC.active[i + 1] = second[0][0];
                D_800A85BC.active[i] = second[1][0];
                third[0][0] = D_800A85BC.value[i];
                third[1][0] = D_800A85BC.value[i + 1];
                D_800A85BC.value[i + 1] = third[0][0];
                D_800A85BC.value[i] = third[1][0];
            }
        }
    }
}
