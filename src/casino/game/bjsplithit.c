#include <persona/casino/blackjack.h>
extern unsigned char D_800A967F[];
extern void          CasinoObjSetOn(CasinoObj*, short, short, unsigned char), CasinoPlaySeq(short*, u_long*, short), CasinoSpritesSetOn(short, short, int);
extern void          CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern void          func_8007E878(unsigned char, unsigned char, unsigned char*, unsigned char*);
extern void          func_8007EB20(unsigned char, unsigned char, CasinoLayout*, unsigned char*, CasinoFrame*, unsigned char, short);
extern void          func_800821B8(unsigned char*, unsigned char);
extern unsigned char D_800A8674;
extern CasinoFrame   D_8009AE2C, D_8009AED0;
extern void          func_8007F960(unsigned char, void*, unsigned char*);
void                 func_8007BA60(void)
{
    short i;
    int   hand;
    if (g_casino_timer == 0)
    {
        switch (D_800A8674)
        {
            case 0:
                func_8007E878(1, 0, g_bj_cards[0], &g_bj_count[0]);
                break;
            case 1:
                func_8007E878(2, 0, g_bj_cards[1], &g_bj_count[1]);
                break;
        }
        CasinoStartAnim(&g_casino_objs[g_bj_card_obj + 1], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, (u_long*)0x138830, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[g_bj_card_obj + 1], 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        if (g_casino_timer == 32)
        {
            switch (D_800A8674)
            {
                case 0:
                    CasinoStartAnim(&g_casino_objs[24], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    CasinoStartAnim(&g_casino_objs[20], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    for (i = 18; i > g_bj_card_obj + 1; i--)
                        CasinoStartAnim(&g_casino_objs[i], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    break;
                case 1:
                    CasinoStartAnim(&g_casino_objs[22], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    CasinoStartAnim(&g_casino_objs[19], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    for (i = 20 - g_bj_count[0]; i > g_bj_card_obj + 1; i--)
                        CasinoStartAnim(&g_casino_objs[i], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
                    break;
            }
        }
    }
    if (g_casino_timer == 36)
    {
        switch (D_800A8674)
        {
            case 0:
                if (g_bj_count[0] == 3)
                {
                    CasinoObjSetOn(&g_casino_objs[20], 2, 17, 0);
                    CasinoSpritesSetOn(60, 3, 0);
                    CasinoSpritesSetOn(70, 3, 0);
                    CasinoSpritesSetOn(80, 3, 0);
                    CasinoSpritesSetOn(90, 3, 0);
                }
                else
                    CasinoObjSetOn(&g_casino_objs[22 - g_bj_count[0]], 2, 17, 0);
            /* The first hand intentionally falls through to hide the second. */
            case 1:
                if (g_bj_count[1] == 3)
                {
                    CasinoObjSetOn(&g_casino_objs[22], 2, 17, 0);
                    CasinoSpritesSetOn(67, 3, 0);
                    CasinoSpritesSetOn(77, 3, 0);
                    CasinoSpritesSetOn(87, 3, 0);
                    CasinoSpritesSetOn(97, 3, 0);
                }
                else
                    CasinoObjSetOn(&g_casino_objs[22 - g_bj_count[0] - g_bj_count[1]], 2, 17, 0);
                break;
        }
    }
    if (g_casino_timer >= 65)
    {
        hand           = D_800A8674;
        g_bj_play_step = 0x1B;
        g_casino_timer = -1;
        switch (hand)
        {
            case 0:
                func_800821B8(g_bj_total, D_800A967F[g_bj_count[0]]);
                func_8007F960(D_800A8674, g_bj_cards[0], g_bj_total);
                func_8007EB20(g_bj_natural[0], g_bj_total[1], D_8009B2B4[2].l, &g_bj_label_n[1], &D_8009AE2C, 1, 8);
                break;
            case 1:
                func_800821B8(g_bj_split_total, g_bj_cards[1][g_bj_count[1] - 1]);
                func_8007F960(D_800A8674, g_bj_cards[1], g_bj_split_total);
                func_8007EB20(g_bj_natural[1], g_bj_split_total[1], D_8009B2B4[3].l, &g_bj_label_n[2], &D_8009AED0, 1, 8);
                break;
        }
        g_bj_play_step = 0x21;
    }
}
