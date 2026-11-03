/* Persona JP1 payout closing animation and palette state. */
extern int g_casino_timer, D_800A94E4[];
extern unsigned char D_800A9724[], D_800AAE9C[], D_800AAEA8[], g_casino_step;
extern short g_casino_seqs[], g_casino_vab, g_casino_main_vab;
extern void *D_800A2BC8[];
typedef struct
{
    int values[11];
} Mask;
extern const Mask D_80065CA8;
extern void func_8009019C(int *), CasinoPlaySeq(short *, unsigned int, short),
    CasinoQueueCluts(int, int, int), func_80091544(unsigned char),
    CasinoTween(void *, short, short, short, short, short), func_80090EEC(unsigned char);
void func_8008E808(void)
{
    if (!g_casino_timer)
    {
        Mask mask = D_80065CA8;
        mask.values[D_800A9724[2] - 2] = 1;
        func_8009019C(mask.values);
        if (D_800AAE9C[2] == 1)
        {
            if (D_800AAEA8[1])
                CasinoPlaySeq(g_casino_seqs + 1, 0x13A970, g_casino_vab);
            else
                CasinoPlaySeq(g_casino_seqs + 1, 0x13AAB0, g_casino_vab);
        }
    }
    if (g_casino_timer < 112)
    {
        if (!(g_casino_timer & 15))
            CasinoQueueCluts(D_800A9724[2] + 62, 1, 0x7CA5);
        if (g_casino_timer % 16 == 8)
            CasinoQueueCluts(D_800A9724[2] + 62, 1, 0x7CE5);
    }
    if (g_casino_timer == 64)
    {
        func_80091544(D_800A9724[2]);
        CasinoTween(D_800A2BC8[3042], 0, -208, 0, 0, 16);
    }
    if (g_casino_timer == 68)
        CasinoPlaySeq(g_casino_seqs, 0x13B2BC, g_casino_main_vab);
    if (g_casino_timer == 112)
    {
        CasinoQueueCluts(D_800A9724[2] + 62, 1, 0x7CA5);
        CasinoTween(D_800A2BC8[3042], 0, 208, 0, 0, 16);
    }
    if ((unsigned int)(g_casino_timer - 112) < 11)
    {
        D_800A94E4[g_casino_timer] = 0;
        func_80090EEC(g_casino_timer - 112);
    }
    if (g_casino_timer == 123)
    {
        g_casino_step = 36;
        g_casino_timer = -1;
    }
}
