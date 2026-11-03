/* Test the real C with GPU calls observed, without emulating PS1 hardware. */
#include "../../src/s2d/game/mapclose.c"
extern int printf(const char*, ...);
extern void* memset(void*, int, unsigned long);
extern void abort(void);
static MapCloseWorkspace workspace;
MapCloseWorkspace* D_800B85BC = &workspace;
int D_800A4D1C, D_800A4D20, g_draw_side;
DRAWENV g_s2d_drawenv;
S2dFade g_fade;
static int stage, frame, firstSide, sidePattern, mutateFade, polyIndex, modeIndex;
static int checks;
#define CHECK(c) do { checks++; if (!(c)) { printf("map close check failed at line %d, stage %d, frame %d\n", __LINE__, stage, frame); abort(); } } while (0)
static void advance(int expected) { CHECK(stage == expected); stage++; }
static int side(void) { return sidePattern ? firstSide : (firstSide ^ (frame & 1)); }
void S2dDraw(void) { advance(0); CHECK(D_800A4D1C == 0 && D_800A4D20 == 0); }
int DrawSync(int mode) { CHECK(mode == 0); if (!frame && stage == 1) advance(1); else advance(49); return 0; }
void S2dMsgClose(void) { advance(2); }
void S2dLeaveToDng(void) { advance(3); }
int MoveImage(RECT* r, int x, int y)
{
    advance(4);
    CHECK(r->x == (short)(512 - g_s2d_drawenv.clip.x));
    CHECK(r->y == g_s2d_drawenv.clip.y && r->w == g_s2d_drawenv.clip.w && r->h == g_s2d_drawenv.clip.h);
    CHECK(x == 128 && y == 256);
    return 0;
}
void SetPolyFT4(POLY_FT4* p)
{
    int i;
    advance(5 + 4 * polyIndex); CHECK(p == workspace.poly + polyIndex);
    for (i = 0; i < 2; i++) CHECK(workspace.ot[i].length == 2 && workspace.ot[i].org == workspace.tags[i]);
}
void SetPolyG4(POLY_G4* p) { CHECK(p == workspace.shade + (stage == 21 ? 0 : 1)); CHECK(stage == 21 || stage == 24); stage++; }
void SetSemiTrans(void* p, int value)
{
    if (stage < 21) { CHECK(stage == 6 + 4 * polyIndex && p == workspace.poly + polyIndex && value == 0); }
    else { CHECK((stage == 22 || stage == 25) && p == workspace.shade + (stage == 22 ? 0 : 1) && value == 1); }
    stage++;
}
void SetShadeTex(void* p, int value)
{
    CHECK(value == 0);
    if (stage < 21) CHECK(stage == 7 + 4 * polyIndex && p == workspace.poly + polyIndex);
    else CHECK((stage == 23 || stage == 26) && p == workspace.shade + (stage == 23 ? 0 : 1));
    stage++;
}
unsigned short GetTPage(int depth, int blend, int x, int y)
{ advance(8 + 4 * polyIndex); CHECK(depth == 2 && blend == 0 && x == (polyIndex + 1) * 128 && y == 256); return 0x120 + polyIndex++; }
void SetDrawMode(DR_MODE* p, int dfe, int dtd, int page, RECT* rect)
{ advance(27 + modeIndex); CHECK(p == workspace.mode + modeIndex++ && dfe == 1 && dtd == 1 && page == 64 && rect == 0); }
int GsGetActiveBuff(void)
{ CHECK(stage == 29); stage = 40; CHECK(frame < 83); return side(); }
void GsSetWorkBase(PACKET* p)
{ advance(40); CHECK((unsigned long)p == 0x80100000UL + ((unsigned long)side() << 16)); }
void GsClearOt(unsigned short offset, unsigned short point, GsOT* ot)
{ advance(41); CHECK(offset == 0 && point == 0 && ot == workspace.ot + side()); }
static void checkPoly(int i)
{
    POLY_FT4* p = workspace.poly + i;
    CHECK(p->r0 == 128 && p->g0 == 128 && p->b0 == 128);
    CHECK(p->x0 == i * 128 - 256 && p->x2 == p->x0 && p->x1 == i * 128 - 128 && p->x3 == p->x1);
    CHECK(p->y0 == -120 && p->y1 == -120 && p->y2 == 120 && p->y3 == 120);
    CHECK(p->u0 == 0 && p->u2 == 0 && p->u1 == 128 && p->u3 == 128);
    CHECK(p->v0 == 0 && p->v1 == 0 && p->v2 == 240 && p->v3 == 240 && p->tpage == 0x120 + i);
}
static void checkShade(int i)
{
    POLY_G4* p = workspace.shade + i;
    int edge = frame < 19 ? 120 - (frame + 1) * 6 : 0;
    int color = frame < 19 ? 0 : (frame - 19) * 4;
    CHECK(p->x0 == -256 && p->x2 == -256 && p->x1 == 256 && p->x3 == 256);
    if (!i) {
        CHECK(p->y0 == -120 && p->y1 == -120 && p->y2 == -edge && p->y3 == -edge);
        CHECK(p->r0 == 255 && p->g0 == 255 && p->b0 == 255 && p->r1 == 255 && p->g1 == 255 && p->b1 == 255);
        CHECK(p->r2 == color && p->g2 == color && p->b2 == color && p->r3 == color && p->g3 == color && p->b3 == color);
    } else {
        CHECK(p->y0 == edge && p->y1 == edge && p->y2 == 120 && p->y3 == 120);
        CHECK(p->r0 == color && p->g0 == color && p->b0 == color && p->r1 == color && p->g1 == color && p->b1 == color);
        CHECK(p->r2 == 255 && p->g2 == 255 && p->b2 == 255 && p->r3 == 255 && p->g3 == 255 && p->b3 == 255);
    }
}
void GsSortPoly(void* p, GsOT* ot, unsigned short priority)
{
    CHECK(ot == workspace.ot + side());
    if (stage < 44) { int i = stage - 42; CHECK(i >= 0 && p == workspace.shade + i && priority == 0); checkShade(i); }
    else { int i = stage - 44; CHECK(i < 4 && p == workspace.poly + i && priority == 1); checkPoly(i); }
    stage++;
}
void AddPrim(void* ot, void* p) { advance(48); CHECK(ot == workspace.tags[side()] && p == workspace.mode + side()); }
int VSync(int mode)
{
    advance(50); CHECK(mode == 2 && g_fade.rgb[0] == 0 && g_fade.rgb[1] == 0 && g_fade.rgb[2] == 0);
    if (mutateFade) { g_fade.rgb[0] = 7; g_fade.rgb[1] = 19; g_fade.rgb[2] = 31; }
    return 0;
}
int ResetGraph(int mode) { advance(51); CHECK(mode == 1); return 0; }
void GsSwapDispBuff(void) { advance(52); }
void GsSortClear(unsigned char r, unsigned char g, unsigned char b, GsOT* ot)
{ advance(53); CHECK(r == (mutateFade ? 7 : 0) && g == (mutateFade ? 19 : 0) && b == (mutateFade ? 31 : 0) && ot == workspace.ot + side()); }
void GsDrawOt(GsOT* ot) { advance(54); CHECK(ot == workspace.ot + side()); frame++; stage = 29; }
void SetDispMask(int mask) { CHECK(stage == 29 && frame == 83 && mask == 0); stage = 99; }
int main(void)
{
    int x, y, cases = 0;
    for (firstSide = 0; firstSide < 2; firstSide++)
    for (sidePattern = 0; sidePattern < 2; sidePattern++)
    for (mutateFade = 0; mutateFade < 2; mutateFade++)
    for (x = 0; x < 3; x++)
    for (y = 0; y < 3; y++) {
        memset(&workspace, 0xA5, sizeof(workspace));
        memset(&g_s2d_drawenv, 0, sizeof(g_s2d_drawenv));
        g_s2d_drawenv.clip.x = x == 0 ? 0 : x == 1 ? 512 : -128;
        g_s2d_drawenv.clip.y = y == 0 ? 0 : y == 1 ? 240 : -32;
        g_s2d_drawenv.clip.w = 512; g_s2d_drawenv.clip.h = 240;
        stage = frame = polyIndex = modeIndex = 0; D_800A4D1C = 123; D_800A4D20 = 456;
        func_80091D24(); CHECK(stage == 99); cases++;
    }
    printf("map close: %d cases, %d frames, %d checks passed\n", cases, cases * 83, checks);
    return 0;
}
