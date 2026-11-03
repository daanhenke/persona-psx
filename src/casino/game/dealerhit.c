/* Persona JP1 casino animation and confirmation state. */
extern int g_casino_timer;
extern unsigned char g_bj_dealer[], g_bj_dealer_count, g_bj_card_obj, g_bj_count[], D_800A8C7C[],
    g_bj_dealer_total[];
extern char g_casino_objs[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void func_8007E878(int, int, void *, void *),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int),
    CasinoPlaySeq(short *, unsigned int, short), CasinoObjSetOn(void *, int, int, int),
    func_800821B8(void *, unsigned char);
void func_8007D714(void)
{
    int i;
    if (!g_casino_timer)
    {
        func_8007E878(3, 0, g_bj_dealer, &g_bj_dealer_count);
        CasinoStartAnim(g_casino_objs + 16 + g_bj_card_obj * 16, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
        CasinoStartAnim(g_casino_objs + 16 + g_bj_card_obj * 16, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        if (g_casino_timer == 32)
        {
            CasinoStartAnim(g_casino_objs + 368, 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
            CasinoStartAnim(g_casino_objs + 336, 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
            for (i = 22 - g_bj_count[0] - g_bj_count[1]; i > g_bj_card_obj + 1; i--)
                CasinoStartAnim(g_casino_objs + i * 16, 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer == 36)
    {
        if (g_bj_dealer_count == 3)
            CasinoObjSetOn(g_casino_objs + 336, 2, 17, 0);
        else
            CasinoObjSetOn(g_casino_objs +
                               (20 - g_bj_count[0] - g_bj_count[1] - g_bj_dealer_count) * 16,
                           2, 17, 0);
    }
    if (g_casino_timer >= 65)
    {
        func_800821B8(g_bj_dealer_total, D_800A8C7C[3 + g_bj_dealer_count]);
        D_800A8C7C[0] = 20;
        g_casino_timer = -1;
    }
}
