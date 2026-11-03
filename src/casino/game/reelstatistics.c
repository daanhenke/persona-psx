/* Persona 1 (JP) - sample reel results and accumulate the debug statistics. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern unsigned short D_800B09A0[7][6];
extern short          D_8009E8E8[7][6];
extern unsigned char  D_800A85F8[];
extern signed char    D_800A8620;
extern unsigned char* D_800B3E28[];
extern CasinoLayout * D_800A1768, *D_800A17BC, *D_800A1624;
extern CasinoObj      D_800A16F8;
extern void           CasinoTween(CasinoLayout*, int, short, short, short, short);
extern void           CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern int            rand(void);
extern void           func_8008749C(unsigned char*, unsigned char), func_80087B08(unsigned char*), func_80087C98(void*);
extern unsigned char  stats_bytes[] __asm__("D_800B09A0");

void func_8008802C(void)
{
    unsigned char* results;
    signed char    positions[21];
    signed char    previous[21];
    unsigned char  symbols[21];
    int            i;
    unsigned char* stats_base;
    int            j;
    unsigned char  stop = 0;
    if (g_casino_timer == 0)
    {

        {

            int col;
            for (i = 0; i < 7; i++)
            {
                for (col = 5; col >= 0; col--)
                {
                    D_800B09A0[i][col] = 0;
                }
            }
        }

        CasinoTween(D_800A1768, -320, 0, 0, 0, 32);
        CasinoTween(D_800A17BC, 320, 0, 0, 0, 32);
        CasinoTween(D_800A1624, -320, 0, 0, 0, 32);
        CasinoStartAnim(&D_800A16F8, 32, 0, 0, 0, 320, 0, 0, 0, 0, 0);
        g_casino_money = 0;
    }
    if (g_casino_timer >= 33)
    {
        for (i = 0; i < 21; i++)
        {
            positions[i] = (rand() % 28) - 3;
            if (positions[i] < 0)
            {
                positions[i] += 28;
            }
        }

        {
            int index;
            for (index = 5; index < 9; index++)
            {
                if (positions[index] == 0)
                {
                    previous[index] = 27;
                }
                else
                {
                    previous[index] = positions[index] - 1;
                }
                symbols[index] = D_800B3E28[index][previous[index]];
            }
        }
        {
            int index;
            for (index = 9; index < 13; index++)
            {
                if (positions[index] == 0)
                {
                    previous[index] = 27;
                }
                else
                {
                    previous[index] = positions[index] - 1;
                }
                symbols[index] = D_800B3E28[index][previous[index]];
            }
        }
        {
            int index;
            for (index = 13; index < 17; index++)
            {
                if (positions[index] == 0)
                {
                    previous[index] = 27;
                }
                else
                {
                    previous[index] = positions[index] - 1;
                }
                symbols[index] = D_800B3E28[index][previous[index]];
            }
        }
        {
            int index;
            for (index = 17; index < 21; index++)
            {
                if (positions[index] == 0)
                {
                    previous[index] = 27;
                }
                else
                {
                    previous[index] = positions[index] - 1;
                }
                symbols[index] = D_800B3E28[index][previous[index]];
            }
        }
        {
            int index;
            for (index = 0; index < 5; index++)
            {
                if (positions[index] == 0)
                {
                    previous[index] = 27;
                }
                else
                {
                    previous[index] = positions[index] - 1;
                }
                symbols[index] = D_800B3E28[index][previous[index]];
            }
        }
        func_8008749C(symbols, 8);
        func_80087B08(symbols);
        for (i = 0; i < 11; i++)
        {
            if (((signed char)D_800A85F8[i]) >= 3)
            {
                unsigned char count = ((signed char)D_800A85F8[i]) - 3;
                int           rank  = D_800A85F8[i + 13];
                D_800B09A0[rank][count]++;
                g_casino_money += D_8009E8E8[rank][count];
            }
        }

        if (((signed char)D_800A85F8[39]) >= 3)
        {

            stats_base = stats_bytes;
            results    = D_800A85F8;
            {
                unsigned long offset;
                unsigned long rowbase;
                rowbase = (unsigned long)(stats_base + D_800A8620 * 12);
                offset  = (signed char)results[39] * 2;
                offset += rowbase;
                (*(u_short*)offset)++;
            }

            g_casino_money += D_8009E8E8[D_800A8620][(signed char)D_800A85F8[39]] * 8;
        }
        func_80087C98(D_800B09A0);
        if ((((g_casino_win == 100) || (g_casino_win == 1000)) || (g_casino_win == 10000)) || (g_casino_win > 9999998U))
        {
            stop = 1;
        }
        g_casino_win++;
    }
    if (g_casino_pad_trig & 128)
    {
        stop = 1;
    }
    if (stop)
    {
        g_casino_step  = 19;
        g_casino_timer = -1;
    }
}
