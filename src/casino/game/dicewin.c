/* Persona JP1 dice result and reward state. */
extern unsigned char D_800A9724[], D_800AAEA8[], D_800AAE9C[], D_80099CA8[],
    g_casino_reward_palette_on, g_casino_step;
extern void *D_800A2BC8[];
extern unsigned int g_casino_win;
extern int g_casino_timer;
extern short g_casino_seqs[], g_casino_vab;
extern void func_8009019C(int *), CasinoTween(void *, int, short, short, short, short),
    CasinoStartPalAnim(void *), CasinoPlaySeq(short *, unsigned int, short);
typedef struct DiceWinMask
{
    int values[11];
} DiceWinMask;

/* Shared zero template also referenced by the payout-finish state. */
const DiceWinMask D_80065CA8 = {{0}};

void func_8008D980(void)
{
    unsigned char won = 1;
    if (!g_casino_timer)
    {
        DiceWinMask mask = D_80065CA8;
        mask.values[D_800A9724[2] - 2] = 1;
        func_8009019C(mask.values);
        if (g_casino_win >= 5000)
            D_800AAEA8[1] = 1;
        if (D_800AAE9C[2] == 0)
            g_casino_win = ((unsigned char *)D_800A2BC8)[D_800A9724[2] + 2] *
                           ((unsigned int *)(D_800A9724 - 136))[D_800A9724[2]] / 10;
        else if (D_800AAE9C[2] == 1)
            g_casino_win = ((unsigned char *)D_800A2BC8)[D_800A9724[2] + 14] *
                           ((unsigned int *)(D_800A9724 - 136))[D_800A9724[2]] / 10;
        if (g_casino_win >= 10000)
            g_casino_win = 10000;
        if (!g_casino_win)
            won = 0;
        if (won)
            CasinoTween(D_800A2BC8[1820], 0, -160, 0, 0, 16);
    }
    if (won && !g_casino_reward_palette_on)
        CasinoStartPalAnim(D_80099CA8);
    if (g_casino_timer == 4 && won)
        CasinoPlaySeq(g_casino_seqs, 0x13AFA4, g_casino_vab);
    if (g_casino_timer >= 16 && g_casino_timer < 32 && won)
    {
        if (!(g_casino_timer & 1))
            CasinoTween(D_800A2BC8[1820], 0, -2, 0, 0, 1);
        else
            CasinoTween(D_800A2BC8[1820], 0, 2, 0, 0, 1);
    }
    if (!won)
    {
        g_casino_step = 115;
        g_casino_timer = -1;
    }
    if (g_casino_timer == 32 && won)
    {
        g_casino_step = 118;
        g_casino_timer = -1;
    }
}
