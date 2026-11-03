extern unsigned int g_casino_money;
extern unsigned char g_casino_bet, D_800A8674, g_bj_count[], g_bj_play_step;
extern int g_casino_timer;
extern void func_8007EF28(unsigned char), func_8007F00C(unsigned char);
extern void func_8007F160(unsigned char, unsigned char, int);

void func_8007B8B0(void)
{
    unsigned char enough = g_casino_money >= g_casino_bet;

    switch (D_800A8674)
    {
    case 0:
        if (g_bj_count[0] == 2 && enough)
        {
            if (g_casino_timer == 0)
            {
                func_8007EF28(2);
                func_8007F00C(114);
            }
            func_8007F160(114, 1, g_casino_timer);
        }
        else
        {
            if (g_casino_timer == 0)
            {
                func_8007EF28(2);
                func_8007F00C(112);
            }
            func_8007F160(112, 1, g_casino_timer);
        }
        break;
    case 1:
        if (g_bj_count[1] == 2 && enough)
        {
            if (g_casino_timer == 0)
            {
                func_8007EF28(1);
                func_8007F00C(114);
            }
            func_8007F160(114, 1, g_casino_timer);
        }
        else
        {
            if (g_casino_timer == 0)
            {
                func_8007EF28(1);
                func_8007F00C(112);
            }
            func_8007F160(112, 1, g_casino_timer);
        }
        break;
    }
    if (g_casino_timer >= 9)
    {
        g_bj_play_step = 27;
        g_casino_timer = -1;
    }
}
