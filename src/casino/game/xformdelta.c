typedef struct
{
    short r[4];
    int p[4], s[4];
} X;
extern X D_80065D54;
extern void CasinoStartAnim(void *, short, short, short, short, int, int, int, int, int, int);
void func_80092F14(void **obj, X *x)
{
    X d = D_80065D54;
    int n;
    d.r[0] = x[0].r[0] - x[1].r[0];
    d.r[1] = x[0].r[1] - x[1].r[1];
    d.r[2] = x[0].r[2] - x[1].r[2];
    d.p[0] = x[0].p[0] - x[1].p[0];
    d.p[1] = x[0].p[1] - x[1].p[1];
    d.p[2] = x[0].p[2] - x[1].p[2];
    d.s[0] = x[0].s[0] - x[1].s[0];
    d.s[1] = x[0].s[1] - x[1].s[1];
    d.s[2] = x[0].s[2] - x[1].s[2];
    n = 0;
    if (d.r[0])
    {
        x[1].r[0] = x[0].r[0];
        n++;
    }
    if (d.r[1])
    {
        x[1].r[1] = x[0].r[1];
        n++;
    }
    if (d.r[2])
    {
        x[1].r[2] = x[0].r[2];
        n++;
    }
    if (d.p[0])
    {
        x[1].p[0] = x[0].p[0];
        n++;
    }
    if (d.p[1])
    {
        x[1].p[1] = x[0].p[1];
        n++;
    }
    if (d.p[2])
    {
        x[1].p[2] = x[0].p[2];
        n++;
    }
    if (d.s[0])
    {
        n++;
    }
    if (d.s[1])
    {
        n++;
    }
    if (d.s[2])
    {
        n++;
    }
    if (n)
        CasinoStartAnim(obj[4], 1, d.r[0], d.r[1], d.r[2], d.p[0], d.p[1], d.p[2], d.s[0], d.s[1],
                        d.s[2]);
}
