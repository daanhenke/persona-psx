#include <decomp/types.h>

typedef struct
{
    short          u, v, w, h;
    unsigned short clut, tpage;
} CasinoStripUV;
extern short **D_8009E8E0;
extern CasinoStripUV *D_8009E8D8;
extern void CasinoQueueUV(short, short, short, short, unsigned short, unsigned short, int);
void func_800825A4(unsigned char index, short first)
{
    int i = 0;
    short *entries = D_8009E8E0[index];
    do
    {
        CasinoStripUV *uv = &D_8009E8D8[*entries];
        unsigned short tpage;
        entries++;
        tpage = uv->tpage;
        CasinoQueueUV(uv->u, uv->v, uv->w, uv->h, uv->clut, tpage, i + first);
        i++;
    } while (i < 20);
}
