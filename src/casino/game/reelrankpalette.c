/* Rank palettes: choose active/inactive coordinates and start the matching animation. */
#include <persona/casino/casino.h>
extern char D_80099B10[];
extern char alternate_palette[] __asm__("D_80099B10+0xA8");
extern s8 D_800A8620;
extern s8 result_ranks[] __asm__("D_800A85F8+0xD");
extern void CasinoQueueCluts(short, short, int);
extern void CasinoStartPalAnim(CasinoPalAnim *);

void func_80085090(s8 index, int active)
{
    s8 rank = result_ranks[index];
    int enabled = active & 255;
    int offset = rank * 24;
    int xoffset;
    int yoffset;
    unsigned long animation;
    int clut_x;
    do { xoffset = offset; } while (0);
    if (enabled == 0) xoffset += 168;
    clut_x = *(short *)(D_80099B10 + xoffset + 8);
    yoffset = offset;
    if (enabled == 0) yoffset += 168;
    CasinoQueueCluts(rank + 153, 1,
        GetClut(clut_x,
                *(short *)(D_80099B10 + yoffset + 10)) & 65535);
    animation = (unsigned long)D_80099B10;
    if (enabled == 0) animation = (unsigned long)alternate_palette;
    CasinoStartPalAnim((CasinoPalAnim*)(offset + animation));
}
