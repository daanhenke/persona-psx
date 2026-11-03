extern unsigned char g_bj_shoe_pos, g_casino_deck[], g_bj_card_obj, g_bj_reshuffle;
extern struct
{
    short rot[4];
    int pos[4], scale[4];
} g_casino_xforms[];
extern char g_casino_objs[][16];
extern short func_80082454(unsigned char);
extern void func_800824CC(int, int, short, int), func_800825A4(int, int), CasinoSetXform(void *),
    CasinoDrawObj(void *);
void func_8007E878(unsigned char side, unsigned char face, unsigned char *cards,
                   unsigned char *count)
{
    short rot[4];
    int pos[3];
    cards[*count] = g_casino_deck[g_bj_shoe_pos];
    func_800824CC(g_bj_card_obj, cards[*count], func_80082454(cards[*count]),
                  g_bj_card_obj * 20 + 140);
    func_800825A4(cards[*count], g_bj_card_obj * 20 + 140);
    if (face == 1)
        rot[1] = 0;
    else
        rot[1] = 2048;
    switch (side)
    {
    case 0:
        pos[0] = *count * 4 + 312;
        pos[1] = 64;
        break;
    case 1:
        pos[0] = *count * 4 + 240;
        pos[1] = 64;
        break;
    case 2:
        pos[0] = *count * 4 + 400;
        pos[1] = 64;
        break;
    case 3:
        pos[0] = *count * 4 + 312;
        pos[1] = -64;
        break;
    }
    g_casino_xforms[g_bj_card_obj].rot[0] = 0;
    g_casino_xforms[g_bj_card_obj].rot[1] = rot[1];
    g_casino_xforms[g_bj_card_obj].rot[2] = 0;
    g_casino_xforms[g_bj_card_obj].pos[0] = pos[0];
    g_casino_xforms[g_bj_card_obj].pos[1] = pos[1];
    g_casino_xforms[g_bj_card_obj].pos[2] = 512;
    g_casino_xforms[g_bj_card_obj].scale[0] = 4096;
    g_casino_xforms[g_bj_card_obj].scale[1] = 4096;
    g_casino_xforms[g_bj_card_obj].scale[2] = 4096;
    CasinoSetXform(&g_casino_xforms[g_bj_card_obj]);
    CasinoDrawObj(g_casino_objs[g_bj_card_obj]);
    g_bj_card_obj--;
    g_bj_shoe_pos++;
    (*count)++;
    if (g_bj_shoe_pos >= 207)
    {
        g_bj_shoe_pos = 0;
        g_bj_reshuffle = 1;
    }
}
