/* Reveal the result and hide unused leading digits after the panels arrive. */
#include <persona/casino/reward.h>
extern CasinoLayout *result_layout __asm__("D_800A2BC8+0x1C70");
extern CasinoLayout *result_frame __asm__("D_800A2BC8+0x2F3C");
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoQueueCluts(short, short, int);
extern void CasinoSpritesSetOn(short, short, int);
void func_8008DF68(void)
{
    u_char *choice;
    u_int reward;
    int selected;
    if (g_casino_timer == 0)
    {
        CasinoTween(result_layout, 0, 240, 0, 0, 16);
        CasinoTween(result_frame, 0, 208, 0, 0, 16);
    }
    if (g_casino_timer == 16)
    {
        choice = &g_casino_reward_data.dice[2];
        CasinoQueueCluts(*choice + 62, 1, 0x7CA5);
        CasinoSpritesSetOn(*choice * 4 + 67, 4, 1);
        CasinoSpritesSetOn(*choice * 2 + 115, 2, 1);
        CasinoSpritesSetOn(*choice * 2 + 137, 2, 1);
        CasinoSpritesSetOn(35, 8, 1);
        selected = *choice;
        reward = g_casino_reward_data.values[(int)((u_int)selected << 16) >> 16];
        if (reward < 100U) CasinoSpritesSetOn(selected * 4 + 67, 2, 0);
        else if (reward < 1000U) CasinoSpritesSetOn(selected * 4 + 67, 1, 0);
        g_casino_step = 0x31;
        g_casino_timer = -1;
    }
}
