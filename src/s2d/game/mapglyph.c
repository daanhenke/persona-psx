/* Project a terminated string using the scene's 31-column glyph atlas. */
#include <persona/s2d/s2d.h>
extern u_short D_800A85D0;

void func_80098F2C(GsOT *ot, u_char *text, int in_x, int in_y, int in_color)
{
    typedef struct { SVECTOR v[6]; POLY_FT4 p; } Glyph;
    Glyph glyph;
    Glyph *g;
    long i;
    int n = 0;
    int x = in_x;
    int y = in_y;
    int color = in_color;

    SetPolyFT4(&glyph.p);
    SetSemiTrans(&glyph.p, 0);
    SetShadeTex(&glyph.p, 1);
    glyph.p.tpage = GetTPage(0, 0, D_800A85D0 + 320, 384);
    g = &glyph;
    while (*text != 255)
    {
        for (i = 0; i < 4; i++)
        {
            g->v[i].vx = x + (i & 1) * 7 + n * 8;
            g->v[i].vy = y + (i / 2) * 12;
            g->v[i].vz = 0;
        }
        RotTransPers4(&g->v[0], &g->v[1], &g->v[2], &g->v[3],
            (long *)&g->p.x0, (long *)&g->p.x1,
            (long *)&g->p.x2, (long *)&g->p.x3, &i, &i);
        g->p.u0 = (u_char)((unsigned)*text % 31) * 8;
        g->p.v0 = (u_char)((unsigned)*text / 31) * 12 + 128;
        g->p.u1 = (u_char)((unsigned)*text % 31) * 8 + 8;
        g->p.v1 = (u_char)((unsigned)*text / 31) * 12 + 128;
        g->p.u2 = (u_char)((unsigned)*text % 31) * 8;
        g->p.v2 = (u_char)((unsigned)*text / 31) * 12 + 140;
        g->p.u3 = (u_char)((unsigned)*text % 31) * 8 + 8;
        g->p.v3 = (u_char)((unsigned)*text / 31) * 12 + 140;
        g->p.clut = GetClut(512, color + 247);
        GsSortPoly(&g->p, ot, 0);
        n++;
        text++;
    }
}
