#include <persona/casino/blackjack.h>
extern unsigned char D_800A8818, D_800A967F[], D_800AAEA8[];
extern void          CasinoObjSetOn(CasinoObj*, short, short, unsigned char), CasinoPlaySeq(short*, u_long*, short), CasinoSpritesSetOn(short, short, int);
extern void          CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern void          func_8007E878(unsigned char, unsigned char, unsigned char*, unsigned char*);
extern void          func_8007EB20(unsigned char, unsigned char, CasinoLayout*, unsigned char*, CasinoFrame*, unsigned char, short);
extern unsigned char func_80081F80(unsigned char*);
extern int           func_80082034(int), func_8008230C(unsigned char*);
extern void          func_800821B8(unsigned char*, unsigned char);
void                 func_80079C28(void)
{
    short i;
    if (g_casino_timer == 0)
    {
        func_8007E878(0, 0, g_bj_cards[0], &g_bj_count[0]);
        CasinoStartAnim(&g_casino_objs[g_bj_card_obj + 1], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, (u_long*)0x138830, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[g_bj_card_obj + 1], 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        if (g_casino_timer == 32)
        {
            CasinoStartAnim(&g_casino_objs[24], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
            CasinoStartAnim(&g_casino_objs[22], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
            for (i = 20; i > g_bj_card_obj + 1; i--)
                CasinoStartAnim(&g_casino_objs[i], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer >= 65)
    {
        if (g_bj_count[0] == 3)
        {
            CasinoObjSetOn(&g_casino_objs[22], 2, 17, 0);
            CasinoSpritesSetOn(63, 3, 0);
            CasinoSpritesSetOn(73, 3, 0);
            CasinoSpritesSetOn(83, 3, 0);
            CasinoSpritesSetOn(93, 3, 0);
        }
        else
            CasinoObjSetOn(&g_casino_objs[24 - g_bj_count[0]], 2, 17, 0);
        func_800821B8(g_bj_total, D_800A967F[g_bj_count[0]]);
        g_bj_play_step = 0x1B;
        g_casino_timer = -1;
        if (func_80081F80(g_bj_cards[0]) == 3)
        {
            g_bj_play_step  = 0x71;
            g_bj_natural[0] = 3;
        }
        if (!D_800A8818)
        {
            if ((unsigned char)func_80082034(g_bj_count[0]) == 10)
            {
                if (g_bj_total[0] == 21 && g_bj_total[1] == g_bj_total[0])
                {
                    g_bj_play_step  = 0x71;
                    g_bj_natural[0] = 10;
                }
                else
                {
                    g_bj_play_step  = 0x1B;
                    g_bj_natural[0] = 10;
                    D_800AAEA8[4]   = 1;
                }
            }
            else if ((unsigned char)func_80082034(g_bj_count[0]) == 9)
            {
                g_bj_play_step  = 0x71;
                g_bj_natural[0] = 9;
            }
            else if (g_bj_total[0] == 21 && g_bj_total[1] == g_bj_total[0] && g_bj_natural[0] == 1)
                g_bj_play_step = 0x71;
        }
        if ((unsigned char)func_8008230C(g_bj_total) == 2)
        {
            g_bj_play_step  = 0x71;
            g_bj_natural[0] = 2;
        }
        if (D_800A8818 == 1 && g_bj_natural[0] == D_800A8818)
            g_bj_play_step = 0x71;
        func_8007EB20(g_bj_natural[0], g_bj_total[1], D_8009B2B4[1].l, &g_bj_label_n[1], (CasinoFrame*)D_8009AD88, 1, 8);
    }
}
