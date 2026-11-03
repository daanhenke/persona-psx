/* Reveal the target grid and finish once its digit animation completes. */
#include <persona/casino/casino.h>
#include <persona/casino/grid.h>
extern TargetGrid grid_goal __asm__("D_800A87E0");
extern s8 D_800AFC00[3][3];
extern u_char D_800A8700[],D_800A8704,D_800B4758[4][3];
extern short grid_ticks __asm__("D_800AF9E8+0xD0");
extern CasinoLayoutDef grid_choices[] __asm__("D_800A2BC8+0x468");
extern void func_8008A9DC(int,short,short),CasinoTween(CasinoLayout*,int,short,short,short,short);
extern void CasinoSpritesSetOn(short,short,int),func_8008A974(int,int),func_8008AD00(u_char[4][3],short),func_8008D120(void);
extern u_char func_8008C9DC(void);
extern short func_8008CBF8(s8[3][3],TargetGrid);
void func_80089C04(void)
{
    int x,y;
    short count=0;
    if(!g_casino_timer){
        func_8008A9DC(3,-128,32);
        CasinoTween(grid_choices[!(D_800A8700[1]&1)].l,-160,0,0,0,32);
        grid_ticks=0;D_800A8704=0;
    }
    if(g_casino_timer==32){
        CasinoSpritesSetOn(293,40,0);
        for(y=0;y<3;y++)for(x=0;x<3;x++)func_8008A974(x,y);
    }
    if(g_casino_timer>=32){count=func_8008CBF8(D_800AFC00,grid_goal);func_8008C9DC();}
    if(count==9){if(!grid_ticks)func_8008AD00(D_800B4758,128);grid_ticks--;}
    {
        u_char *done=&D_800A8704;
        if(*done==1){CasinoSpritesSetOn(293,40,0);func_8008D120();*done=0;g_casino_step=119;g_casino_timer=-1;}
    }
}
