/* Persona 1 (JP) - blackjack card-test screen and animated card selection. */
#include <persona/casino/blackjack.h>
extern CasinoXform   g_casino_xforms[];
extern unsigned char func_80082454(unsigned char);
extern void          func_800824CC(int, unsigned char, int, int);
extern void          func_800825A4(unsigned char, int);
extern void          CasinoSetXform(CasinoXform*);
extern void          CasinoDrawObj(CasinoObj*);
extern void          CasinoStartAnim();
extern void          CasinoSpritesSetOn(int, int, int), CasinoPlaySeq(short*, int, short);
extern void          CasinoCursorStep(CasinoCursor*, int), CasinoKeyRepeat(signed char*, signed char*, int, int);
extern void          func_8007E878(int, int, void*, void*), SsSeqStop(short), SsSeqPlay(short, int, int);
extern signed char   D_800AF738[], D_800AF73C[], D_800A8C72[];
extern unsigned char D_800B467C[];
static inline void   Cycle(unsigned char* card, unsigned char slot, unsigned char forward)
{
    unsigned char rank;
    int           x;
    if (forward)
    {
        (*card)++;
        if (*card >= 52)
            *card = 0;
    }
    else
    {
        (*card)--;
        if ((signed char)*card < 0)
            *card = 51;
    }
    rank = func_80082454(*card);
    x    = slot * 20 + 140;
    func_800824CC(slot, *card, rank, x);
    func_800825A4(*card, x);
    CasinoSetXform(&g_casino_xforms[slot]);
    CasinoDrawObj(&g_casino_objs[slot]);
}
void func_80080270(void)
{
    int i;
    int j;
    if (g_casino_timer == 0)
    {
        CasinoSpritesSetOn(0, 140, 0);
        g_casino_cursor.x      = 0;
        g_casino_cursor.prev_x = 0;
        func_8007E878(0, 1, g_bj_cards, g_bj_count);
        func_8007E878(3, 1, g_bj_dealer, &g_bj_dealer_count);
        func_8007E878(0, 1, g_bj_cards, g_bj_count);
        func_8007E878(3, 1, g_bj_dealer, &g_bj_dealer_count);
        for (j = 0; j < 8; j++)
            func_8007E878(0, 1, g_bj_cards, g_bj_count);
        for (i = 24; i >= 13; i--)
            CasinoStartAnim(&g_casino_objs[i], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[23], 2, 0, 0, 0, -8, 0, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[24], 18, 0, 0, 0, -72, 0, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[22], 16, 0, 0, 0, -64, 0, 0, 0, 0, 0);
        for (i = 20; i >= 14; i--)
            CasinoStartAnim(&g_casino_objs[i], (i - 13) * 2, 0, 0, 0, i * (-8) + 104, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 50)
        CasinoStartAnim(&g_casino_objs[24], 4, 0, 0, 0, 0, -4, 0, 0, 0, 0);
    if (g_casino_timer >= 57)
    {
        CasinoCursorStep(&g_casino_cursor, 2);
        if (g_casino_cursor.x != g_casino_cursor.prev_x)
        {
            CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
            CasinoStartAnim(&g_casino_objs[24 - g_casino_cursor.x], 4, 0, 0, 0, 0, -4, 0, 0, 0, 0);
            CasinoStartAnim(&g_casino_objs[24 - g_casino_cursor.prev_x], 4, 0, 0, 0, 0, 4, 0, 0, 0, 0);
        }
        CasinoKeyRepeat(&D_800AF738[2], &D_800A8C72[0], g_casino_pad & 32, 4);
        CasinoKeyRepeat(&D_800AF738[5], &D_800A8C72[3], g_casino_pad & 64, 4);
        if (D_800AF738[2])
        {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            D_800AF738[2] = 0;
            {
                unsigned char* card = D_800B467C + g_bj_shoe_pos;
                card               += g_casino_cursor.prev_x;
                Cycle(card, 24 - g_casino_cursor.prev_x, 1);
            }
        }
        if (D_800AF73C[1])
        {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            D_800AF73C[1] = 0;
            {
                unsigned char* card = D_800B467C + g_bj_shoe_pos;
                card               += g_casino_cursor.prev_x;
                Cycle(card, 24 - g_casino_cursor.prev_x, 0);
            }
        }
        if (g_casino_timer >= 57 && !(g_casino_pad & 4))
        {
            CasinoStartAnim(&g_casino_objs[24 - g_casino_cursor.prev_x], 4, 0, 0, 0, 0, 4, 0, 0, 0, 0);
            g_casino_step  = 61;
            g_casino_timer = -1;
        }
    }
}
