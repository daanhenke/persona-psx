#include <persona/casino/reels.h>
#include <libsnd.h>
extern union
{
    unsigned int word;
    signed char  bytes[5];
} D_800AF830;
extern signed char   reel_mode4 __asm__("D_800AF834");
extern unsigned char first_stop __asm__("D_800AF738+3");
extern signed char   D_800AF9E0[];
extern unsigned char D_800AF738[], D_800AF73C[];
extern CasinoPalAnim D_80099A20;
extern CasinoLayout* D_800A15CC;
extern CasinoObj     D_800A156C;
extern void          CasinoStartPalAnim(CasinoPalAnim*);
extern void          CasinoTween(CasinoLayout*, int, short, short, short, short);
extern void          CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern void          func_80085A7C(signed char), func_80085B74(signed char), func_80085F08(void);
void                 func_80083398(void)
{
    unsigned char  pending = 1;
    int            i;
    CasinoLayout** exit_layout;
    if (g_casino_timer % 17 == 0)
        SsSeqStop(g_casino_seqs[11]);
    SsSeqPlay(g_casino_seqs[11], 1, 1);
    if (((g_casino_pad_trig & 0x20) || (g_casino_pad_trig & 0x80) || (g_casino_pad_trig & 0x10) || (g_casino_pad_trig & 0x40)) &&
        (D_800AF830.bytes[1] == 4 || D_800AF830.bytes[2] == 4 || D_800AF830.bytes[3] == 4 || reel_mode4 == 4))
    {
        int j;
        for (j = 0; j < 4; j++)
            if (D_800AF830.bytes[j + 1] == 4)
                D_800AF830.bytes[j + 1] = 3;
    }
    if (g_casino_timer >= 0)
        for (i = 0; i < 4; i++)
        {
            func_80085A7C(i);
            func_80085B74(i);
        }
    if (g_casino_timer == 104)
    {
        int j;
        for (j = 0; j < 4; j++)
            if (D_800AF830.bytes[j + 1] == 4)
                D_800AF830.bytes[j + 1] = 3;
    }
    if ((signed char)D_800AF73C[2] == 0)
    {
        if ((D_800AF830.bytes[2] == 0 || D_800AF830.bytes[2] == 6) && (g_casino_pad_trig & 0x80))
            first_stop++;
        if ((D_800AF830.bytes[3] == 0 || D_800AF830.bytes[3] == 6) && (g_casino_pad_trig & 0x20))
            D_800AF738[2]++;
        if ((D_800AF830.bytes[1] == 0 || D_800AF830.bytes[1] == 6) && (g_casino_pad_trig & 0x10))
            D_800AF73C[0]++;
        if ((reel_mode4 == 0 || reel_mode4 == 6) && (g_casino_pad_trig & 0x40))
            D_800AF73C[1]++;
    }
    if (D_80099A20.on == 0)
        CasinoStartPalAnim(&D_80099A20);
    if ((D_800AF830.word & ~255U) == 0x06060600 && reel_mode4 == 6)
    {
        reel_mode4          = 0;
        D_800AF830.bytes[3] = 0;
        D_800AF830.bytes[2] = 0;
        D_800AF830.bytes[1] = 0;
        g_casino_timer      = 0;
        g_casino_step       = 0x71;
    }
    if (g_casino_step == 0x71)
    {
        func_80085F08();
        if (g_casino_timer == 0)
        {
            CasinoTween(D_800A15CC, 0, -16, 0, 0, 16);
            CasinoStartAnim(&D_800A156C, 16, 0, 0, 0, 0, -16, 0, 0, 0, 0);
        }
        if ((signed char)D_800AF738[3] == 0 && (signed char)D_800AF738[2] == 0 && (signed char)D_800AF73C[0] == 0 && (signed char)D_800AF73C[1] == 0)
            pending = 0;
        exit_layout = &D_800A15CC;
        if ((g_casino_pad_trig & 8) || (g_casino_pad_trig & 2))
        {
            if ((signed char)D_800AF73C[2] == 0)
            {
                SsSeqStop(g_casino_seqs[4]);
                SsSeqPlay(g_casino_seqs[4], 1, 1);
                D_800AF73C[2]++;
                CasinoTween(*exit_layout, 0, 16, 0, 0, 16);
                CasinoStartAnim(&D_800A156C, 16, 0, 0, 0, 0, 16, 0, 0, 0, 0);
            }
        }
        if ((signed char)D_800AF73C[2] != 0 && !pending)
        {
            g_casino_timer      = -16;
            g_casino_step       = 0x72;
            D_800AF73C[2]       = 0;
            D_800AF73C[3]       = 0;
            D_800AF9E0[3]       = 0;
            D_800AF9E0[2]       = 0;
            D_800AF9E0[1]       = 0;
            D_800AF9E0[0]       = 0;
            D_800AF830.bytes[0] = 3;
        }
    }
}