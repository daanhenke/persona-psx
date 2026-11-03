/* Blink the selected dice reward, hiding leading zeroes at the bright phase. */
#include <persona/casino/reward.h>
extern u_char g_casino_reward_palette_on;
extern CasinoPalAnim D_80099CA8;
extern void CasinoStartPalAnim(CasinoPalAnim *);
extern void CasinoQueueCluts(short, short, int);
extern void CasinoSpritesSetOn(short, short, int);

void func_800915FC(void)
{
    if (!g_casino_reward_palette_on) CasinoStartPalAnim(&D_80099CA8);
    if (!(g_casino_timer & 7))
        CasinoQueueCluts(g_casino_reward_data.dice[2] + 62, 1, 0x7CA5);
    if (g_casino_timer % 8 == 4)
        CasinoQueueCluts(g_casino_reward_data.dice[2] + 62, 1, 0x7CE5);
    if (!(g_casino_timer & 31)) {
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 67, 4, 0);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 115, 2, 0);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 137, 2, 0);
    }
    if (g_casino_timer % 32 == 16) {
        if (g_casino_reward_data.values[(int)((u_int)g_casino_reward_data.dice[2] << 16) >> 16] < 10)
            CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 70, 1, 1);
        else if (g_casino_reward_data.values[(int)((u_int)g_casino_reward_data.dice[2] << 16) >> 16] < 100)
            CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 69, 2, 1);
        else if (g_casino_reward_data.values[(int)((u_int)g_casino_reward_data.dice[2] << 16) >> 16] < 1000)
            CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 68, 3, 1);
        else if (g_casino_reward_data.values[(int)((u_int)g_casino_reward_data.dice[2] << 16) >> 16] < 10000)
            CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 67, 4, 1);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 115, 2, 1);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 137, 2, 1);
    }
}
