typedef struct
{
    int tag;
    unsigned char r, g, b, c;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    short x2, y2;
    unsigned char u2, v2;
    short pad2;
    short x3, y3;
    unsigned char u3, v3;
    short pad3;
} Poly;
typedef struct
{
    short x, y, w, h;
} Rect;
typedef struct
{
    Rect r[2];
} Rects;
extern Rects D_8006507C;
extern unsigned char D_800B5D18, D_800A831C[];
extern Poly first __asm__("D_800B5D18+8"), pair[] __asm__("D_800B5D44+4"),
    third __asm__("D_800B5D44+84"), fourth __asm__("D_800B5DBC+4");
extern struct
{
    short r[4];
    int scale[4], pos[4];
} D_800B5E00;
extern void LoadImage(Rect *, void *), SetPolyFT4(Poly *), SetSemiTrans(Poly *, int),
    SetShadeTex(Poly *, int);
extern unsigned short GetClut(int, int), GetTPage(int, int, int, int);
void func_80098BFC(void)
{
    Rect rect;
    Rects uv = D_8006507C;
    int i;
    Poly *p;
    D_800B5D18 = 0;
    rect.x = 512;
    rect.y = 241;
    rect.w = 256;
    rect.h = 1;
    LoadImage(&rect, D_800A831C);
    SetPolyFT4(&first);
    SetSemiTrans(&first, 0);
    SetShadeTex(&first, 1);
    first.u0 = 0;
    first.v0 = 96;
    first.u1 = 96;
    first.v1 = 96;
    first.u2 = 0;
    first.v2 = 122;
    first.u3 = 96;
    first.v3 = 122;
    first.clut = GetClut(512, 240);
    for (i = 0; i < 2; i++)
    {
        p = &pair[i];
        SetPolyFT4(p);
        SetSemiTrans(p, 0);
        SetShadeTex(p, 1);
        p->u0 = uv.r[i].x;
        p->v0 = uv.r[i].y;
        p->u1 = uv.r[i].x + uv.r[i].w;
        p->v1 = uv.r[i].y;
        p->u2 = uv.r[i].x;
        p->v2 = uv.r[i].y + uv.r[i].h;
        p->u3 = uv.r[i].x + uv.r[i].w;
        p->v3 = uv.r[i].y + uv.r[i].h;
        pair[i].tpage = GetTPage(1, 0, 320, 352);
        pair[i].clut = GetClut(512, 240);
    }
    SetPolyFT4(&third);
    SetSemiTrans(&third, 0);
    SetShadeTex(&third, 1);
    third.clut = GetClut(512, 241);
    SetPolyFT4(&fourth);
    SetSemiTrans(&fourth, 1);
    SetShadeTex(&fourth, 0);
    fourth.r = 128;
    fourth.g = 128;
    fourth.b = 128;
    fourth.tpage = GetTPage(1, 2, 384, 392);
    fourth.clut = GetClut(512, 241);
    D_800B5E00.r[0] = 0;
    D_800B5E00.r[1] = 0;
    D_800B5E00.r[2] = 0;
    D_800B5E00.scale[0] = 6553;
    D_800B5E00.scale[1] = 4096;
    D_800B5E00.scale[2] = 0;
    D_800B5E00.pos[0] = 0;
    D_800B5E00.pos[1] = 0;
    D_800B5E00.pos[2] = 640;
}
