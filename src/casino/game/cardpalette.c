/* Select the card-kind palette using the original signed-byte rank conversion. */
#include <persona/casino/casino.h>
extern u_char card_ranks[] __asm__("D_800A85F8");
extern char card_palettes[] __asm__("D_80099A50");
extern char card_alternate[] __asm__("D_80099A98+24");
extern void CasinoQueueCluts(short, short, int);
extern void CasinoStartPalAnim(CasinoPalAnim *);

void func_80084FC4(s8 index, int active)
{
    s8 rank = (int)((u_int)(card_ranks[index] - 3) << 24) >> 24;
    int enabled = active & 255;
    int offset = rank * 24;
    int xoffset;
    int yoffset;
    unsigned long animation;
    int clut_x;
    do { xoffset = offset; } while (0);
    if (enabled == 0) xoffset += 96;
    clut_x = *(short*)(card_palettes + xoffset + 8);
    yoffset = offset;
    if (enabled == 0) yoffset += 96;
    CasinoQueueCluts(rank * 8 + 160, 1,
        GetClut(clut_x,
                *(short*)(card_palettes + yoffset + 10)) & 65535);
    animation = (unsigned long)card_palettes;
    if (enabled == 0) animation = (unsigned long)card_alternate;
    CasinoStartPalAnim((CasinoPalAnim*)(offset + animation));
}
