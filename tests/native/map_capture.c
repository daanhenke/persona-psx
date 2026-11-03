/* Exercise the production C with an observed graphics interface. No game assets. */
#define SKIP_ASM
#include "../../src/s2d/game/mapcapture.c"
extern int printf(const char *, ...);
extern void abort(void);
short D_8005E5A4, D_8005E5A6, D_8005E5A8, D_8005E5AA, D_8005E600, D_8005E602;
int g_draw_side;
GsOT g_ot_obj[2];
DRAWENV g_s2d_drawenv;
static int cases, side, flip, position, clip, clobber, count, projected, sorted, geom, got,
    expected[80], wanted;
static long checks;
static POLY_FT4 *quads[4], *current;
static MATRIX *matrix;
static RECT source;
#define CHECK(c)                                                                                   \
    do {                                                                                           \
        checks++;                                                                                  \
        if (!(c)) {                                                                                \
            printf("map capture failed: line %d case %d event %d\n", __LINE__, cases, got);        \
            abort();                                                                               \
        }                                                                                          \
    } while (0)
enum {
    FT4 = 1,
    SEMI,
    SHADE,
    PAGE,
    READ,
    GEOM,
    ROT,
    TRANS,
    SCALE,
    SETROT,
    SETTRANS,
    PROJECT,
    BEGIN,
    SORT,
    CLEAR,
    DRAW,
    SYNC,
    MOVE,
    VSYNC
};
static void expect(int n) { expected[wanted++] = n; }
static void event(int n) {
    CHECK(got < wanted && expected[got] == n);
    got++;
}
void SetPolyFT4(POLY_FT4 *p) {
    event(FT4);
    CHECK(count < 4);
    if (count)
        CHECK(p == quads[0] + count);
    current = quads[count++] = p;
    p->code = 0x2c;
}
void SetSemiTrans(void *p, int n) {
    event(SEMI);
    CHECK(p == current && n == 0);
}
void SetShadeTex(void *p, int n) {
    event(SHADE);
    CHECK(p == current && n == 0);
}
unsigned short GetTPage(int depth, int blend, int x, int y) {
    event(PAGE);
    CHECK(depth == 2 && blend == 0 && x == (count - 1) * 128 && y == 256);
    return 0x110 + count - 1;
}
void ReadGeomOffset(long *x, long *y) {
    event(READ);
    *x = position * 113 - 32768;
    *y = 32767 - position * 227;
}
void SetGeomOffset(long x, long y) {
    event(GEOM);
    CHECK(x == (geom ? position * 113 - 32768 : -256));
    CHECK(y == (geom ? 32767 - position * 227 : -120));
    geom++;
}
MATRIX *RotMatrix(SVECTOR *r, MATRIX *m) {
    event(ROT);
    CHECK(r->vx == 0 && r->vy == 0 && r->vz == 0);
    matrix = m;
    m->t[0] = 123;
    return m;
}
MATRIX *TransMatrix(MATRIX *m, VECTOR *v) {
    event(TRANS);
    CHECK(m == matrix && m->t[0] == 123);
    CHECK(v->vx == 0 && v->vy == 0 && v->vz == 0);
    m->t[0] = 456;
    return m;
}
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v) {
    event(SCALE);
    CHECK(m == matrix && m->t[0] == 456);
    CHECK(v->vx == 2564 && v->vy == 4096 && v->vz == 4096);
    m->t[0] = 789;
    return m;
}
void SetRotMatrix(MATRIX *m) {
    event(SETROT);
    CHECK(m == matrix && m->t[0] == 789);
}
void SetTransMatrix(MATRIX *m) {
    event(SETTRANS);
    CHECK(m == matrix && m->t[0] == 789);
}
long RotTransPers4(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3, long *x0, long *x1, long *x2,
                   long *x3, long *p, long *flag) {
    SVECTOR *v[4] = {v0, v1, v2, v3};
    long *xy[4] = {x0, x1, x2, x3};
    POLY_FT4 *q;
    int j;
    event(PROJECT);
    CHECK(projected < 4 && p == flag && *p == 4);
    q = quads[projected];
    CHECK(x0 == (long *)&q->x0 && x1 == (long *)&q->x1 && x2 == (long *)&q->x2 &&
          x3 == (long *)&q->x3);
    for (j = 0; j < 4; j++) {
        CHECK(v[j] == v0 + j && v[j]->vx == (projected + (j & 1)) * 128 - 4 &&
              v[j]->vy == (j / 2) * 240 && v[j]->vz == 640);
        /* SDK long is 32 bits; write the packed coordinate as shorts on LP64. */
        ((short *)xy[j])[0] = projected * 11 + j;
        ((short *)xy[j])[1] = projected * 17 + j;
    }
    *p = clobber == 0 ? -123 : clobber == 1 ? 0 : 65535;
    projected++;
    return 0;
}
void S2dBeginFrame(int n) {
    event(BEGIN);
    CHECK(n == 1 && geom == 2 && projected == 4);
    g_draw_side = side ^ flip;
    /* Capture must use the draw environment selected by this call. */
    g_s2d_drawenv.clip = source;
}
void GsSortPoly(void *p, GsOT *ot, unsigned short depth) {
    POLY_FT4 *q = p;
    event(SORT);
    CHECK(sorted < 4 && p == quads[sorted] && ot == g_ot_obj + (side ^ flip) && depth == 63);
    CHECK(q->code == 0x2c && q->r0 == 128 && q->g0 == 128 && q->b0 == 128 &&
          q->tpage == 0x110 + sorted);
    CHECK(q->u0 == 0 && q->u1 == 128 && q->u2 == 0 && q->u3 == 128);
    CHECK(q->v0 == 0 && q->v1 == 0 && q->v2 == 240 && q->v3 == 240);
    CHECK(q->x0 == sorted * 11 && q->x1 == sorted * 11 + 1 && q->x2 == sorted * 11 + 2 &&
          q->x3 == sorted * 11 + 3);
    CHECK(q->y0 == sorted * 17 && q->y1 == sorted * 17 + 1 && q->y2 == sorted * 17 + 2 &&
          q->y3 == sorted * 17 + 3);
    sorted++;
}
void GsSortClear(unsigned char r, unsigned char g, unsigned char b, GsOT *ot) {
    event(CLEAR);
    CHECK(r == 0 && g == 0 && b == 0 && ot == g_ot_obj + (side ^ flip) && sorted == 4);
}
void GsDrawOt(GsOT *ot) {
    event(DRAW);
    CHECK(ot == g_ot_obj + (side ^ flip));
}
int DrawSync(int n) {
    event(SYNC);
    CHECK(n == 0);
    return 0;
}
int MoveImage(RECT *r, int x, int y) {
    event(MOVE);
    CHECK(r != &g_s2d_drawenv.clip);
    CHECK(r->x == source.x && r->y == source.y && r->w == source.w && r->h == source.h && x == 0 &&
          y == 256);
    return 0;
}
int VSync(int n) {
    event(VSYNC);
    CHECK(n == 0);
    CHECK(D_8005E5A4 == -1 && D_8005E5A6 == -2 && D_8005E5A8 == -3 && D_8005E5AA == -4 &&
          D_8005E600 == -5 && D_8005E602 == -6);
    return 0;
}
int main(void) {
    int j;
    for (side = 0; side < 2; side++)
        for (flip = 0; flip < 2; flip++)
            for (position = 0; position < 3; position++)
                for (clip = 0; clip < 4; clip++)
                    for (clobber = 0; clobber < 3; clobber++) {
                        wanted = got = count = projected = sorted = geom = 0;
                        g_draw_side = side;
                        D_8005E5A4 = -1;
                        D_8005E5A6 = -2;
                        D_8005E5A8 = -3;
                        D_8005E5AA = -4;
                        D_8005E600 = -5;
                        D_8005E602 = -6;
                        source.x = (clip & 1) * 512;
                        source.y = (clip >> 1) * 256;
                        source.w = clip % 2 ? 320 : 512;
                        source.h = clip % 2 ? 224 : 240;
                        g_s2d_drawenv.clip.x = -7;
                        g_s2d_drawenv.clip.y = -8;
                        g_s2d_drawenv.clip.w = -9;
                        g_s2d_drawenv.clip.h = -10;
                        for (j = 0; j < 4; j++) {
                            expect(FT4);
                            expect(SEMI);
                            expect(SHADE);
                            expect(PAGE);
                        }
                        expect(READ);
                        expect(GEOM);
                        expect(ROT);
                        expect(TRANS);
                        expect(SCALE);
                        expect(SETROT);
                        expect(SETTRANS);
                        for (j = 0; j < 4; j++)
                            expect(PROJECT);
                        expect(GEOM);
                        expect(BEGIN);
                        for (j = 0; j < 4; j++)
                            expect(SORT);
                        expect(CLEAR);
                        expect(DRAW);
                        expect(SYNC);
                        expect(MOVE);
                        expect(SYNC);
                        expect(VSYNC);
                        func_80095E34();
                        CHECK(got == wanted && count == 4 && projected == 4 && sorted == 4 &&
                              geom == 2);
                        CHECK(D_8005E5A4 == 320 && D_8005E5A6 == 240 && D_8005E5A8 == 0 &&
                              D_8005E5AA == 120 && D_8005E600 == 320 && D_8005E602 == 240);
                        cases++;
                    }
    printf("map capture: %d cases, %ld checks passed\n", cases, checks);
    return 0;
}
