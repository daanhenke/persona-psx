typedef struct
{
    char bytes[40];
} CasinoXform;
typedef struct
{
    char bytes[16];
} CasinoObj;
extern CasinoObj g_casino_objs[];
extern int g_casino_timer;
extern unsigned int g_casino_pad;
extern unsigned char g_casino_step;

extern CasinoXform g_casino_xforms[];
extern unsigned char func_80082454(unsigned char);
extern void func_800824CC(int, unsigned char, int, int);
extern void func_800825A4(unsigned char, int);
extern void CasinoSetXform(CasinoXform *);
extern void CasinoDrawObj(CasinoObj *);

static inline void Cycle(unsigned char *card, unsigned char slot, unsigned char forward)
{
    unsigned char rank;
    int x;

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
    x = slot * 20 + 140;
    func_800824CC(slot, *card, rank, x);
    func_800825A4(*card, x);
    CasinoSetXform(&g_casino_xforms[slot]);
    CasinoDrawObj(&g_casino_objs[slot]);
}

extern unsigned char g_casino_cursor[], g_bj_cards[], g_bj_count[], g_bj_card_obj, g_bj_shoe_pos,
    D_800A8674, g_casino_deck[];
extern signed char D_800AF738[], D_800AF73C[], D_800A8C72[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void CasinoSpritesSetOn(int, int, int), CasinoStartAnim(),
    func_8007E878(int, int, void *, void *), CasinoPlaySeq(short *, int, short),
    CasinoKeyRepeat(signed char *, signed char *, int, int);
void func_80080A78(void)
{
    if (g_casino_timer == 0)
    {
        CasinoSpritesSetOn(0, 140, 0);
        g_casino_cursor[0] = 0;
        g_casino_cursor[2] = 0;
        CasinoStartAnim(&g_casino_objs[21], 64, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        func_8007E878(0, 1, g_bj_cards, &g_bj_count[D_800A8674]);
        g_bj_card_obj++;
        g_bj_shoe_pos += 255;
        g_bj_count[D_800A8674] += 255;
        CasinoStartAnim(&g_casino_objs[g_bj_card_obj], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
        CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
    if (g_casino_timer > 32)
    {
        CasinoKeyRepeat(&D_800AF738[2], &D_800A8C72[0], g_casino_pad & 32, 4);
        CasinoKeyRepeat(&D_800AF738[5], &D_800A8C72[3], g_casino_pad & 64, 4);
        if (D_800AF738[2])
        {
            D_800AF738[2] = 0;
            Cycle(&g_casino_deck[g_bj_shoe_pos], g_bj_card_obj, 1);
        }
        if (D_800AF73C[1])
        {
            D_800AF73C[1] = 0;
            Cycle(&g_casino_deck[g_bj_shoe_pos], g_bj_card_obj, 0);
        }
        if (g_casino_timer > 32 && !(g_casino_pad & 4))
        {
            g_casino_step = 123;
            g_casino_timer = -1;
        }
    }
}
