/* Exercise the real C with observed graphics calls, without game assets. */
#define SKIP_ASM
#include "../../src/s2d/game/mappresent.c"
extern int printf(const char *, ...);
extern void abort(void);
CaptureTail D_800B942C;
RECT D_800B9428;
POLY_G4 D_800B8FDC;
u_char D_800B94D4[3], D_800B94DC[3], D_800B94E4[3], D_800B94EC[3];
u_short D_800B8FD4, D_800B8FD8, D_800B91E0, D_800B91E4;
int g_view_zoom, D_800B91C0, D_800A4CEC, D_800B0F18, g_draw_side;
short D_8005E5A4, D_8005E5A6, D_8005E5A8, D_8005E5AA, D_8005E600, D_8005E602;
S2dWin D_800B91EC;
GsOT g_ot_map[2], g_ot_obj[2];
DR_MODE g_scene_drmode[2];
DRAWENV g_s2d_drawenv;
S2dFade g_fade;
static int cases, side, delay, perturb, message, position, frame, poll, quadCount, projection,
    sorted;
static int want[1024], nWant, nGot, values[256], changed[256], frames;
static int moves, stores, loads, vramMoves, geometry;
static void *primitive;
static unsigned long checks, totalFrames;
#define CHECK(c)                                                                                   \
    do {                                                                                           \
        checks++;                                                                                  \
        if (!(c)) {                                                                                \
            printf("map present failed: line %d case %d event %d frame %d\n", __LINE__, cases,     \
                   nGot, frame);                                                                   \
            abort();                                                                               \
        }                                                                                          \
    } while (0)
enum {
    CLOSE = 1,
    BEGIN,
    PARTY,
    MAP,
    FRONT,
    SPECIAL,
    STORE,
    VRAMMOVE,
    VIEW,
    SYNC,
    MOVE,
    G4,
    SEMI,
    SHADE,
    CAPTURE,
    MODE,
    COLORS,
    FRAME,
    POLL,
    FT4,
    PAGE,
    READGEOM,
    GEOM,
    ROTATE,
    TRANSLATE,
    SCALE,
    SETROT,
    SETTRANS,
    PROJECT,
    SORT,
    EXTRA,
    VSYNC,
    LOAD,
    FONTLOAD,
    FONTOPEN,
    FONTSELECT,
    MSG,
    RESTORE
};
static void expect(int event) { want[nWant++] = event; }
static void event(int event)
{
    CHECK(nGot < nWant && want[nGot] == event);
    nGot++;
}
void S2dMsgClose(void)
{
    event(CLOSE);
    CHECK(D_800B91EC.flags == 0 && g_view_zoom == 0);
    CHECK(D_800B8FD4 == D_800B91E0 && D_800B8FD8 == D_800B91E4);
}
void S2dBeginFrame(int n)
{
    event(BEGIN);
    CHECK(n == (moves == 2));
}
void S2dDrawParty(GsOT *ot, int n)
{
    event(PARTY);
    CHECK(ot == g_ot_map + side && n == 3);
}
void S2dDrawMapObjs(void) { event(MAP); }
void func_800971D4(void) { event(FRONT); }
void func_8008AC10(void) { event(SPECIAL); }
void VramStore(int x, int y, int w, int h, u_long *p)
{
    event(STORE);
    CHECK(y == 256 && w == 192);
    CHECK(x == (stores ? 512 : 0) && h == (stores ? 248 : 256));
    CHECK((unsigned long)p == (stores ? 0x800D0000UL : 0x800E8000UL));
    stores++;
}
void VramMove(int x, int y, int w, int h, int dx, int dy)
{
    event(VRAMMOVE);
    CHECK(y == 256 && dy == 256 && h == 248);
    CHECK(x == (vramMoves ? 512 : 320) && dx == (vramMoves ? 320 : 512) &&
          w == (vramMoves ? 128 : 192));
    vramMoves++;
}
void VramLoad(int x, int y, int w, int h, u_long *p)
{
    event(LOAD);
    CHECK(y == 256 && w == 192);
    CHECK(x == (loads ? 512 : 0) && h == (loads ? 248 : 256));
    CHECK((unsigned long)p == (loads ? 0x800D0000UL : 0x800E8000UL));
    CHECK(D_8005E5A4 == 512 && D_8005E5A6 == 240 && D_8005E5A8 == 256 && D_8005E5AA == 120 &&
          D_8005E600 == 512 && D_8005E602 == 240);
    loads++;
}
void func_80099930(int n)
{
    event(VIEW);
    CHECK(n == (loads == 0));
}
int DrawSync(int n)
{
    event(SYNC);
    CHECK(n == 0);
    return 0;
}
int MoveImage(RECT *r, int x, int y)
{
    event(MOVE);
    CHECK(r == &D_800B9428);
    if (!moves) {
        CHECK(x == 0 && y == 256);
        CHECK(r->x == g_s2d_drawenv.clip.x && r->y == g_s2d_drawenv.clip.y && r->w == 512 &&
              r->h == 240);
    } else {
        CHECK(x == side * 512 && y == 0);
        CHECK(r->x == 0 && r->y == 256 && r->w == 512 && r->h == 240);
    }
    moves++;
    return 0;
}
void SetPolyG4(POLY_G4 *p)
{
    event(G4);
    CHECK(p == &D_800B8FDC || p == &D_800B942C.shade);
    primitive = p;
    p->code = 0x38;
}
void SetSemiTrans(void *p, int n)
{
    event(SEMI);
    CHECK(p == primitive && n == (quadCount == 0));
}
void SetShadeTex(void *p, int n)
{
    event(SHADE);
    CHECK(p == primitive && n == 0);
}
void func_80095E34(void)
{
    int i;
    event(CAPTURE);
    CHECK(D_800B8FDC.r0 == 0 && D_800B8FDC.g0 == 0 && D_800B8FDC.b0 == 0);
    CHECK(D_800B8FDC.r1 == 0 && D_800B8FDC.g1 == 0 && D_800B8FDC.b1 == 0);
    CHECK(D_800B8FDC.r2 == 255 && D_800B8FDC.g2 == 255 && D_800B8FDC.b2 == 255);
    CHECK(D_800B8FDC.r3 == 255 && D_800B8FDC.g3 == 255 && D_800B8FDC.b3 == 255);
    for (i = 0; i < 3; i++)
        CHECK(D_800B94D4[i] == 255 && D_800B94DC[i] == 255 && D_800B94E4[i] == 0 &&
              D_800B94EC[i] == 0);
}
void SetDrawMode(DR_MODE *p, int dfe, int dtd, int page, RECT *r)
{
    event(MODE);
    CHECK(p == &g_scene_drmode[0] && dfe == 0 && dtd == 1 && page == 64 && r == 0);
    p->tag = 0x1234;
    p->code[0] = 0xABCD;
    p->code[1] = 0x5678;
}
void S2dLoadWinColors(void)
{
    event(COLORS);
    CHECK(D_800B91C0 == 0);
    CHECK(g_scene_drmode[1].tag == 0x1234 && g_scene_drmode[1].code[0] == 0xABCD &&
          g_scene_drmode[1].code[1] == 0x5678);
    if (perturb)
        D_800B91C0 = perturb == 1 ? 47 : 49;
}
int func_8009C07C(void)
{
    event(POLL);
    return poll++ == delay ? 1 : 0;
}
void func_80093AE0(void)
{
    event(FRAME);
    CHECK(frame < frames && D_800B91C0 == values[frame]);
    if (changed[frame])
        D_800B91C0 = changed[frame];
    frame++;
    totalFrames++;
}
void SetPolyFT4(POLY_FT4 *p)
{
    event(FT4);
    CHECK(quadCount < 4 && p == &D_800B942C.poly[quadCount]);
    primitive = p;
    p->code = 0x2C;
    quadCount++;
}
unsigned short GetTPage(int depth, int blend, int x, int y)
{
    event(PAGE);
    CHECK(depth == 2 && blend == 0 && x == (quadCount - 1) * 128 && y == 256);
    return 0x110 + quadCount - 1;
}
void ReadGeomOffset(long *x, long *y)
{
    event(READGEOM);
    *x = position * 31 - 70;
    *y = position * 19 + 90;
}
void SetGeomOffset(long x, long y)
{
    event(GEOM);
    CHECK(x == (geometry ? position * 31 - 70 : -256) &&
          y == (geometry ? position * 19 + 90 : -120));
    geometry++;
}
MATRIX *RotMatrix(SVECTOR *r, MATRIX *m)
{
    event(ROTATE);
    CHECK(r->vx == 0 && r->vy == 0 && r->vz == 0);
    return m;
}
MATRIX *TransMatrix(MATRIX *m, VECTOR *v)
{
    event(TRANSLATE);
    CHECK(v->vx == 0 && v->vy == 0 && v->vz == 0);
    return m;
}
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v)
{
    event(SCALE);
    CHECK(v->vx == 0x1999 && v->vy == 4096 && v->vz == 4096);
    return m;
}
void SetRotMatrix(MATRIX *m)
{
    event(SETROT);
    CHECK(m != 0);
}
void SetTransMatrix(MATRIX *m)
{
    event(SETTRANS);
    CHECK(m != 0);
}
long RotTransPers4(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3, long *x0, long *x1, long *x2,
                   long *x3, long *p, long *flag)
{
    SVECTOR *v[4] = {v0, v1, v2, v3};
    long *xy[4] = {x0, x1, x2, x3};
    int j;
    POLY_FT4 *q = &D_800B942C.poly[projection];
    event(PROJECT);
    CHECK(projection < 4 && p == flag);
    CHECK(x0 == (long *)&q->x0 && x1 == (long *)&q->x1 && x2 == (long *)&q->x2 &&
          x3 == (long *)&q->x3);
    for (j = 0; j < 4; j++) {
        CHECK(v[j] == v0 + j && v[j]->vx == (projection + (j & 1)) * 128 + 3 &&
              v[j]->vy == (j / 2) * 240 && v[j]->vz == 640);
        /* The SDK has 32-bit long; write the two shorts individually on LP64. */
        ((short *)xy[j])[0] = projection * 11 + j;
        ((short *)xy[j])[1] = projection * 17 + j;
    }
    *p = -123;
    projection++;
    return 0;
}
void GsSortPoly(void *p, GsOT *ot, unsigned short n)
{
    POLY_FT4 *q = p;
    event(SORT);
    CHECK(sorted < 4 && q == &D_800B942C.poly[sorted] && ot == g_ot_obj + side && n == 63);
    CHECK(q->code == 0x2C && q->r0 == 128 && q->g0 == 128 && q->b0 == 128 &&
          q->tpage == 0x110 + sorted);
    CHECK(q->u0 == 0 && q->u1 == 128 && q->u2 == 0 && q->u3 == 128);
    CHECK(q->v0 == 0 && q->v1 == 0 && q->v2 == 240 && q->v3 == 240);
    CHECK(q->x0 == sorted * 11 && q->x3 == sorted * 11 + 3 && q->y0 == sorted * 17 &&
          q->y3 == sorted * 17 + 3);
    sorted++;
}
void func_80089F5C(int n)
{
    event(EXTRA);
    CHECK(n == 2 && g_fade.rgb[0] == 0 && g_fade.rgb[1] == 0 && g_fade.rgb[2] == 0);
}
int VSync(int n)
{
    event(VSYNC);
    CHECK(n == 0);
    return 0;
}
void FntLoad(int x, int y)
{
    event(FONTLOAD);
    CHECK(x == 192 && y == 256);
}
int FntOpen(int x, int y, int w, int h, int bg, int max)
{
    event(FONTOPEN);
    CHECK(x == -240 && y == -104 && w == 512 && h == 240 && bg == 0 && max == 512);
    return 20 + position;
}
void SetDumpFnt(int id)
{
    event(FONTSELECT);
    CHECK(id == 20 + position && D_800B0F18 == id);
}
void S2dMsgOpen(int n)
{
    event(MSG);
    CHECK(message && n == D_800A4CEC);
}
void func_80096904(void) { event(RESTORE); }
int main(void)
{
    const int start[] = {CLOSE, BEGIN,    PARTY, MAP,   FRONT,   SPECIAL, STORE,
                         STORE, VRAMMOVE, VIEW,  SYNC,  MOVE,    G4,      SEMI,
                         SHADE, G4,       SEMI,  SHADE, CAPTURE, MODE,    COLORS};
    const int middle[] = {READGEOM, GEOM,    ROTATE,  TRANSLATE, SCALE,    SETROT,
                          SETTRANS, PROJECT, PROJECT, PROJECT,   PROJECT,  GEOM,
                          MOVE,     SYNC,    BEGIN,   SORT,      SORT,     SORT,
                          SORT,     EXTRA,   SYNC,    VSYNC,     LOAD,     VRAMMOVE,
                          LOAD,     SYNC,    VIEW,    FONTLOAD,  FONTOPEN, FONTSELECT};
    for (side = 0; side < 2; side++)
        for (delay = 0; delay < 3; delay++)
            for (perturb = 0; perturb < 3; perturb++)
                for (message = 0; message < 2; message++)
                    for (position = 0; position < 3; position++) {
                        int j, state = 0, value = perturb ? (perturb == 1 ? 47 : 49) : 0,
                               waited = 0, mutated = 0;
                        cases++;
                        nWant = nGot = frames = frame = poll = quadCount = projection = sorted =
                            moves = stores = loads = vramMoves = geometry = 0;
                        for (j = 0; j < (int)(sizeof(start) / sizeof(*start)); j++)
                            expect(start[j]);
                        do {
                            if (state == 0) {
                                if (value == 48)
                                    state = 1;
                                else {
                                    value += 2;
                                    if (value > 48)
                                        value = 48;
                                }
                            } else if (state == 1) {
                                expect(POLL);
                                if (waited++ == delay)
                                    state = 2;
                            } else if (state == 2) {
                                if (value == 0)
                                    state = 3;
                                else {
                                    value -= 2;
                                    if (value < 0)
                                        value = 0;
                                }
                            }
                            expect(FRAME);
                            values[frames] = value;
                            changed[frames] = 0;
                            if (state == 2 && perturb && !mutated) {
                                value = perturb == 1 ? 1 : -1;
                                changed[frames] = value;
                                mutated = 1;
                            }
                            frames++;
                        } while (state != 3);
                        for (j = 0; j < 4; j++) {
                            expect(FT4);
                            expect(SEMI);
                            expect(SHADE);
                            expect(PAGE);
                        }
                        for (j = 0; j < (int)(sizeof(middle) / sizeof(*middle)); j++)
                            expect(middle[j]);
                        if (message)
                            expect(MSG);
                        expect(RESTORE);
                        expect(SYNC);
                        g_draw_side = side;
                        D_800B91EC.flags = 0xFFFF;
                        g_view_zoom = 99;
                        D_800B91C0 = 999;
                        D_800B91E0 = 0x8000 + position;
                        D_800B91E4 = 0xFFFF - position;
                        D_800B8FD4 = D_800B8FD8 = 0;
                        D_800A4CEC = message ? position * 7 : -1;
                        g_s2d_drawenv.clip.x = position * 256;
                        g_s2d_drawenv.clip.y = position * 8;
                        g_s2d_drawenv.clip.w = 512;
                        g_s2d_drawenv.clip.h = 240;
                        g_fade.rgb[0] = g_fade.rgb[1] = g_fade.rgb[2] = 255;
                        func_80093DF8();
                        CHECK(nGot == nWant && frame == frames && sorted == 4 && projection == 4 &&
                              quadCount == 4 && geometry == 2 && stores == 2 && loads == 2 &&
                              vramMoves == 2);
                    }
    printf("map present: %d cases, %lu frames, %lu checks passed\n", cases, totalFrames, checks);
    return 0;
}
