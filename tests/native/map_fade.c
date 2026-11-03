/* Run the real transition C with synthetic GPU state and observed calls. */
#define SKIP_ASM
#include "../../src/s2d/game/mapfade.c"
extern int printf(const char*, ...);
extern void abort(void);
static MapFadeWorkspace workspace;
MapFadeWorkspace* D_800B85BC = &workspace;
int D_800A4D1C, D_800A4D20, g_draw_side;
DRAWENV g_s2d_drawenv;
S2dFade g_fade;
GsOT g_ot_map[2];
static int mode, frame, firstSide, constantSide, flipOnPrim, mutateFade, quadIndex, item, cases;
static int calls[1024], count, expected[1024], expectedCount;
static unsigned long checks;
#define CHECK(c) do { checks++; if (!(c)) { printf("map fade failed: line %d case %d frame %d call %d\n", __LINE__, cases, frame, count); abort(); } } while (0)
enum { BEGIN=1, PARTY, MAP, FRONT, EXTRA, UNLOCK, DRAW, SYNC, LTS, ADV, MOVE, FT4, SEMI, SHADE, PAGE, F4, PRIM, MASK, ACTIVE, CLEAR, ADD, VSYNC, RESET, SWAP, SORT, OT };
static void event(int v) { CHECK(count < 1024); calls[count++] = v; }
static void want(int v) { expected[expectedCount++] = v; }
static int selectedSide(void) { return constantSide ? firstSide : (firstSide ^ (frame & 1)); }
static void checkInitial(void)
{ CHECK(D_800A4D1C == 0 && D_800A4D20 == 0); CHECK(g_fade.rgb[0] == 96 && g_fade.rgb[1] == 96 && g_fade.rgb[2] == 96); }
void S2dBeginFrame(int alt) { event(BEGIN); CHECK(mode == 0 && alt == 0); checkInitial(); }
void S2dDrawParty(GsOT* ot, int shift) { event(PARTY); CHECK(ot == g_ot_map + firstSide && shift == 3); }
void S2dDrawMapObjs(void) { event(MAP); }
void func_800971D4(void) { event(FRONT); }
void func_80089F5C(int n) { event(EXTRA); CHECK(n == 0); }
void S2dFrameUnlocked(void) { event(UNLOCK); CHECK(mode != 0); checkInitial(); }
void S2dDraw(void) { event(DRAW); }
int DrawSync(int n) { event(SYNC); CHECK(n == 0); return 0; }
static void checkOT(void)
{ int i; for (i = 0; i < 2; i++) CHECK(workspace.ot[i].length == 2 && workspace.ot[i].org == workspace.tags[i]); }
void S2dLoadLtsAsync2(void) { event(LTS); CHECK(mode == 0); checkOT(); }
void S2dLoadAdvCmdAsync(void) { event(ADV); CHECK(mode != 0); checkOT(); }
int MoveImage(RECT* r, int x, int y)
{ event(MOVE); CHECK(x == 0 && y == 256); CHECK(r->x == (short)(512 - g_s2d_drawenv.clip.x)); CHECK(r->y == g_s2d_drawenv.clip.y && r->w == g_s2d_drawenv.clip.w && r->h == g_s2d_drawenv.clip.h); return 0; }
void SetPolyFT4(POLY_FT4* p) { event(FT4); CHECK(p == &workspace.poly[0][0]); p->code = 0x2C; }
void SetSemiTrans(void* p, int n) { event(SEMI); CHECK(p == &workspace.poly[0][0] && n == 0); }
void SetShadeTex(void* p, int n) { event(SHADE); CHECK(p == &workspace.poly[0][0] && n == 0); }
unsigned short GetTPage(int depth, int blend, int x, int y)
{ event(PAGE); CHECK(quadIndex < 4 && depth == 2 && blend == 0 && x == quadIndex * 128 && y == 256); return 0x110 + quadIndex++; }
void SetPolyF4(POLY_F4* p) { event(F4); CHECK(mode == 0 && p == &workspace.shade); }
void DrawPrim(void* p)
{
    POLY_F4* q = &workspace.shade; event(PRIM); CHECK(p == q && mode == 0);
    CHECK(q->r0 == 0 && q->g0 == 0 && q->b0 == 0);
    CHECK(q->x0 == -256 && q->x1 == 256 && q->x2 == -256 && q->x3 == 256);
    CHECK(q->y0 == -120 && q->y1 == -120 && q->y2 == 120 && q->y3 == 120);
}
static void checkStrips(void)
{
    int b, i;
    for (b = 0; b < 2; b++) for (i = 0; i < 4; i++) {
        POLY_FT4* p = &workspace.poly[b][i];
        CHECK(p->code == 0x2C && p->tpage == 0x110 + i);
        CHECK(p->x0 == i * 128 - 256 && p->x1 == i * 128 - 128 && p->x2 == p->x0 && p->x3 == p->x1);
        CHECK(p->y0 == -120 && p->y1 == -120 && p->y2 == 120 && p->y3 == 120);
        CHECK(p->u0 == 0 && p->u1 == 128 && p->u2 == 0 && p->u3 == 128);
        CHECK(p->v0 == 0 && p->v1 == 0 && p->v2 == 240 && p->v3 == 240);
    }
}
void SetDispMask(int on) { event(MASK); CHECK(on == 1 && quadIndex == 4); checkStrips(); }
int GsGetActiveBuff(void) { event(ACTIVE); CHECK(frame < (mode ? 21 : 22)); CHECK(item == 0); return selectedSide(); }
void GsClearOt(unsigned short offset, unsigned short point, GsOT* ot)
{ event(CLEAR); CHECK(offset == 0 && point == 0 && ot == workspace.ot + selectedSide()); CHECK(g_draw_side == selectedSide()); }
void AddPrim(void* ot, void* primitive)
{
    POLY_FT4* p = &workspace.poly[g_draw_side][item];
    int color = mode ? 128 - 6 * (frame + 1) : 6 * (frame + 1);
    event(ADD); CHECK(item < 4 && primitive == p && ot == workspace.tags[g_draw_side]);
    CHECK(p->r0 == color && p->g0 == color && p->b0 == color);
    CHECK(p->x0 == item * 128 - 256 && p->tpage == 0x110 + item);
    item++;
    if (flipOnPrim) g_draw_side ^= 1;
}
int VSync(int n)
{ event(VSYNC); CHECK(n == 0 && item == 4); CHECK(g_fade.rgb[0] == 0 && g_fade.rgb[1] == 0 && g_fade.rgb[2] == 0); if (mutateFade) { g_fade.rgb[0] = 3; g_fade.rgb[1] = 11; g_fade.rgb[2] = 27; } return 0; }
int ResetGraph(int n) { event(RESET); CHECK(n == 1); return 0; }
void GsSwapDispBuff(void) { event(SWAP); }
void GsSortClear(unsigned char r, unsigned char g, unsigned char b, GsOT* ot)
{ event(SORT); CHECK(r == (mutateFade ? 3 : 0) && g == (mutateFade ? 11 : 0) && b == (mutateFade ? 27 : 0)); CHECK(ot == &workspace.ot[g_draw_side]); }
void GsDrawOt(GsOT* ot)
{ event(OT); CHECK(ot == &workspace.ot[g_draw_side]); frame++; item = 0; }
int main(void)
{
    int m, x, n;
    const int modes[] = {0, 1, -1, 7};
    for (m = 0; m < 4; m++) for (firstSide = 0; firstSide < 2; firstSide++)
    for (constantSide = 0; constantSide < 2; constantSide++) for (flipOnPrim = 0; flipOnPrim < 2; flipOnPrim++)
    for (mutateFade = 0; mutateFade < 2; mutateFade++) for (x = 0; x < 3; x++) {
        cases++; mode = modes[m]; frame = quadIndex = item = count = expectedCount = 0;
        memset((unsigned char*)&workspace, 0xA5, sizeof(workspace)); D_800A4D1C = D_800A4D20 = 99; g_draw_side = firstSide;
        g_s2d_drawenv.clip.x = x * 256; g_s2d_drawenv.clip.y = 8; g_s2d_drawenv.clip.w = 512; g_s2d_drawenv.clip.h = 240;
        if (!mode) { want(BEGIN); want(PARTY); want(MAP); want(FRONT); want(EXTRA); } else { want(UNLOCK); want(DRAW); }
        want(SYNC); want(mode ? ADV : LTS); want(MOVE); want(SYNC); want(FT4); want(SEMI); want(SHADE);
        for (n = 0; n < 4; n++) want(PAGE);
        if (!mode) { want(F4); want(PRIM); want(SYNC); }
        want(MASK);
        for (n = 0; n < (mode ? 21 : 22); n++) {
            int j; want(ACTIVE); want(CLEAR); for (j = 0; j < 4; j++) want(ADD);
            want(SYNC); want(VSYNC); want(RESET); want(SWAP); want(SORT); want(OT);
        }
        func_80092C6C(mode);
        CHECK(frame == (mode ? 21 : 22) && count == expectedCount);
        for (n = 0; n < count; n++) CHECK(calls[n] == expected[n]);
    }
    printf("map fade: %d cases, %lu checks passed\n", cases, checks); return 0;
}
