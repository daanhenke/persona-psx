/* Persona JP1: matching casino state handler. */
extern unsigned char g_bj_natural[], g_bj_label_n[], g_bj_total[], g_bj_split_total[], D_800AAEA8[],
    D_8009AE2C[], D_8009AED0[];
extern void *D_8009B2B4[];
extern int g_casino_timer;
extern void func_8007EB20(int, int, void *, void *, void *, int, int);
void func_8008174C(unsigned char hand)
{
    switch (hand)
    {
    case 0:
        if (g_bj_natural[0] == 10 && g_casino_timer >= 64)
        {
            if (!(g_casino_timer & 127))
            {
                func_8007EB20(10, g_bj_total[1], D_8009B2B4[5], g_bj_label_n + 1, D_8009AE2C, 0, 1);
                D_800AAEA8[4] = 1;
            }
            if (g_casino_timer % 128 == 64)
            {
                func_8007EB20(1, g_bj_total[1], D_8009B2B4[5], g_bj_label_n + 1, D_8009AE2C, 0, 1);
                D_800AAEA8[4] = 0;
            }
        }
        break;
    case 1:
        if (g_bj_natural[1] == 10 && g_casino_timer >= 64)
        {
            if (!(g_casino_timer & 127))
            {
                func_8007EB20(10, g_bj_split_total[1], D_8009B2B4[7], g_bj_label_n + 2, D_8009AED0,
                              0, 1);
                D_800AAEA8[4] = 1;
            }
            if (g_casino_timer % 128 == 64)
            {
                func_8007EB20(1, g_bj_split_total[1], D_8009B2B4[7], g_bj_label_n + 2, D_8009AED0,
                              0, 1);
                D_800AAEA8[4] = 0;
            }
        }
        break;
    }
}
