/* Execute the production frame dispatcher with synthetic state and graphics adapters.
 * Fixed PS1 RAM addresses are mapped locally; no game data is needed. */
#define _FNTPRINT_
int FntPrint(char*, ...);
#include "../../src/s2d/game/framefinish.c"
/* Use the host ABI; the PS1 SDK stdarg macros describe MIPS stack arguments. */
typedef __builtin_va_list HostArgs;
extern int                printf(const char*, ...);
extern void               abort(void);
extern void*              mmap(void*, unsigned long, int, int, int, long);
extern int                munmap(void*, unsigned long);
extern void*              memset(void*, int, unsigned long);
extern int                strcmp(const char*, const char*);
extern char*              strcpy(char*, const char*);
char                      D_80064C8C[] = "D_80064C8C";
char                      D_80064C98[] = "D_80064C98";
char                      D_80064C9C[] = "D_80064C9C";
char                      D_80064CAC[] = "D_80064CAC";
char                      D_80064CBC[] = "D_80064CBC";
char                      D_80064CD8[] = "D_80064CD8";
char                      D_80064CEC[] = "D_80064CEC";
char                      D_80064CF8[] = "D_80064CF8";
char                      D_80064D04[] = "D_80064D04";
char                      D_80064D10[] = "D_80064D10";
char                      D_80064D14[] = "D_80064D14";
char                      D_80064D18[] = "D_80064D18";
char                      D_80064D24[] = "D_80064D24";
char                      D_80064D34[] = "D_80064D34";
char                      D_80064D54[] = "D_80064D54";
char                      D_80064D7C[] = "D_80064D7C";
char                      D_80064D8C[] = "D_80064D8C";
char                      D_80064DA0[] = "D_80064DA0";
char                      D_80064DB4[] = "D_80064DB4";
char                      D_80064DBC[] = "D_80064DBC";
char                      D_80064DC4[] = "D_80064DC4";
char                      D_80064DD0[] = "D_80064DD0";
char                      D_80064DD8[] = "D_80064DD8";
char                      D_80064DE0[] = "D_80064DE0";
char                      D_80064DE8[] = "D_80064DE8";
char                      D_80064DEC[] = "D_80064DEC";
char                      D_80064DF8[] = "D_80064DF8";
char                      D_80064E04[] = "D_80064E04";
char                      D_80064E18[] = "D_80064E18";
char                      D_80064E1C[] = "D_80064E1C";
char                      D_80064E24[] = "D_80064E24";
char                      D_80064E34[] = "D_80064E34";
char                      D_80064E3C[] = "D_80064E3C";
char                      D_80064E4C[] = "D_80064E4C";
char                      D_80064E50[] = "D_80064E50";
char                      D_80064E5C[] = "D_80064E5C";
char                      D_80064E68[] = "D_80064E68";
int                       D_800A4D08, D_800A4D0C[4], D_800A4D1C, D_800A4D20, D_800A4D34, D_800A4D3C, D_800A4D40;
int                       D_800B0EF0, D_800B0F18, D_800B8650, g_cd_busy, g_key_lock, g_draw_side, g_map_obj_count;
short                     D_800B8FD4, D_800B8FD8, g_s2d_selected_event_flag, g_s2d_exit;
u_short                   g_map_id;
u_char                    g_map_pos_x, g_map_pos_y;
u_int                     g_s2d_frame;
char*                     g_s2d_workbase;
GsOT                      g_ot_back[2], g_ot_obj[2], g_ot_map[2], g_ot_front[2], g_ot_layer[3][2];
DR_MODE                   g_menu_drmode[2], g_scene_drmode[2];
DRAWENV                   g_s2d_drawenv;
S2dFade                   g_fade;
POLY_F4                   g_fade_poly;
char*                     D_800A4D24[] = { "exit0", "exit1", "exit2", "exit3" };
char*                     D_800A4E8C[] = { "map0", "map1", "map2", "map3", "map4", "map5", "map6", "map7", "map8", "map9", "map10", "map11" };
char*                     D_800A4EBC[] = { "sound0", "sound1", "sound2", "sound3", "sound4" };
char*                     D_800A4ED0[] = { "tile0", "tile1", "tile2", "tile3", "tile4", "tile5", "tile6", "tile7", "tile8" };
Labels4                   D_80064E70   = { { { 'a', 0 }, { 'b', 0 }, { 'c', 0 }, { 'd', 0 } } };
Labels2                   D_80064E80   = { { { 'e', 0 }, { 'f', 0 } } };
static int                cases, mode, selected, side, swap, flushChange, syncMode, frame, eventFlag, packetSize;
static int                oldMaxObjects, oldMaxPacket, initialBlank, afterBlank, drawn, adds, cleared, sorted, flagsTested;
static int                events[100], wantEvents[100], eventCount, wantEventCount;
static long               checks;
static char               packets[1024];
static u_short            tileValue;
typedef struct
{
    char* format;
    int   a, b;
    char  text[32];
} Print;
static Print prints[100], wantPrints[100];
static int   printed, wanted;
#define CHECK(c)                                                                         \
    do                                                                                   \
    {                                                                                    \
        checks++;                                                                        \
        if (!(c))                                                                        \
        {                                                                                \
            printf("map frame failed line %d case %d mode %d\n", __LINE__, cases, mode); \
            abort();                                                                     \
        }                                                                                \
    } while (0)
enum
{
    ADD = 1,
    ENV,
    BLANK,
    SHOW,
    SYNC,
    VRAM,
    PACKETS,
    POLY,
    PRINT,
    FLUSH,
    TILES,
    VSYNC,
    RESET,
    PAGES,
    EXTRA,
    SWAP,
    CLEAR,
    DRAW,
    FINISH,
    FLAG
};
static void event(int n)
{
    CHECK(eventCount < 100);
    events[eventCount++] = n;
}
static void expect(int n)
{
    CHECK(wantEventCount < 100);
    wantEvents[wantEventCount++] = n;
}
static void putPrint(Print* p, char* fmt, int a, int b, const char* text)
{
    p->format  = fmt;
    p->a       = a;
    p->b       = b;
    p->text[0] = 0;
    if (text)
    {
        CHECK(__builtin_strlen(text) < sizeof p->text);
        strcpy(p->text, text);
    }
}
static void out(char* fmt, int a, int b, const char* text)
{
    CHECK(wanted < 100);
    putPrint(&wantPrints[wanted++], fmt, a, b, text);
    expect(PRINT);
}
int FntPrint(char* fmt, ...)
{
    HostArgs ap;
    int      a = 0, b = 0;
    char*    text = 0;
    __builtin_va_start(ap, fmt);
    if (fmt == D_80064C98 || fmt == D_80064CEC || fmt == D_80064D04 || fmt == D_80064DA0 || fmt == D_80064DE8 || fmt == D_80064E34)
        text = __builtin_va_arg(ap, char*);
    else if (fmt == D_80064CBC || fmt == D_80064CD8 || fmt == D_80064D34 || fmt == D_80064D54 || fmt == D_80064DEC || fmt == D_80064DF8 || fmt == D_80064E3C || fmt == D_80064E5C)
    {
        a = __builtin_va_arg(ap, int);
        b = __builtin_va_arg(ap, int);
    }
    else if (fmt == D_80064E04)
    {
        a    = __builtin_va_arg(ap, int);
        b    = __builtin_va_arg(ap, int);
        text = __builtin_va_arg(ap, char*);
    }
    else if (fmt == D_80064CF8 || fmt == D_80064D18 || fmt == D_80064D24 || fmt == D_80064D7C || fmt == D_80064D8C || fmt == D_80064E68)
        a = __builtin_va_arg(ap, int);
    __builtin_va_end(ap);
    CHECK(printed < 100);
    putPrint(&prints[printed++], fmt, a, b, text);
    event(PRINT);
    return 0;
}
void AddPrim(void* ot, void* p)
{
    CHECK(adds < 2);
    CHECK(ot == (adds ? g_ot_obj[side].tag : g_ot_back[side].tag));
    CHECK(p == (void*)(adds ? &g_scene_drmode[side] : &g_menu_drmode[side]));
    adds++;
    event(ADD);
}
DRAWENV* GetDrawEnv(DRAWENV* p)
{
    CHECK(p == &g_s2d_drawenv);
    p->clip.x = 123;
    p->clip.y = 47;
    event(ENV);
    return p;
}
void SetDispMask(int n)
{
    CHECK(n == 0 || n == 1);
    event(n ? SHOW : BLANK);
}
int DrawSync(int n)
{
    CHECK(n == 0);
    event(SYNC);
    return 0;
}
void VramFlushQueues(void)
{
    D_800A4D08 = afterBlank;
    event(VRAM);
}
char* func_80034860(void)
{
    event(PACKETS);
    return packets + packetSize;
}
void GsSortPoly(void* p, GsOT* ot, u_short depth)
{
    CHECK(p == &g_fade_poly);
    CHECK(ot == &g_ot_back[side]);
    CHECK(depth == 0);
    CHECK(g_fade_poly.x0 == -256 && g_fade_poly.x2 == -256);
    CHECK(g_fade_poly.y0 == -100 && g_fade_poly.y1 == -100);
    CHECK(g_fade_poly.x1 == (mode == 1 ? -64 : mode == 2 ? -32
                                                         : -56));
    CHECK(g_fade_poly.x3 == g_fade_poly.x1);
    CHECK(g_fade_poly.y2 == (mode == 2 ? 4 : mode == 3 ? -52
                                                       : 12));
    CHECK(g_fade_poly.y3 == g_fade_poly.y2);
    sorted++;
    event(POLY);
}
int EventFlagTest(u_short* p)
{
    CHECK(p == (u_short*)&g_s2d_selected_event_flag);
    flagsTested++;
    event(FLAG);
    return eventFlag;
}
u_long* FntFlush(int id)
{
    CHECK(id == D_800B0F18);
    event(FLUSH);
    return 0;
}
void FadeTilesDraw(void)
{
    CHECK(mode == 4);
    event(TILES);
}
int VSync(int n)
{
    CHECK(n == syncMode);
    event(VSYNC);
    return 100 + n;
}
int ResetGraph(int n)
{
    CHECK(n == 1);
    CHECK(D_800B0EF0 == 100 + syncMode);
    event(RESET);
    return 0;
}
void S2dMapPagesFlip(void)
{
    event(PAGES);
}
void func_8008E1C0(void)
{
    event(EXTRA);
}
void GsSwapDispBuff(void)
{
    g_draw_side = side ^ swap;
    event(SWAP);
}
void GsSortClear(u_char r, u_char g, u_char b, GsOT* ot)
{
    CHECK(r == 17 && g == 39 && b == 201);
    CHECK(ot == &g_ot_obj[side ^ swap]);
    cleared++;
    event(CLEAR);
}
void GsDrawOt(GsOT* p)
{
    GsOT* order[] = { &g_ot_obj[side ^ swap], &g_ot_map[side ^ swap], &g_ot_front[side ^ swap], &g_ot_back[side ^ swap], &g_ot_layer[0][side ^ swap], &g_ot_layer[1][side ^ swap], &g_ot_layer[2][side ^ swap] };
    CHECK(drawn < 7 && p == order[drawn]);
    drawn++;
    event(DRAW);
}
void func_80097288(int n)
{
    CHECK(n == syncMode);
    CHECK(g_s2d_frame == (u_int)frame + 1);
    CHECK(g_fade.rgb[0] == 96 && g_fade.rgb[1] == 96 && g_fade.rgb[2] == 96);
    event(FINISH);
}
static void marker(int row)
{
    out(row != selected ? D_80064DD0 : (frame % 2 ? D_80064DD8 : D_80064DE0), 0, 0, 0);
}
static void reference(void)
{
    int i, j, maxObjects = g_map_obj_count > oldMaxObjects ? g_map_obj_count : oldMaxObjects;
    int maxPacket = packetSize > oldMaxPacket ? packetSize : oldMaxPacket;
    expect(ADD);
    expect(ADD);
    expect(ENV);
    if (initialBlank)
        expect(BLANK);
    expect(SYNC);
    expect(VRAM);
    if (afterBlank)
        expect(SHOW);
    expect(PACKETS);
    if (mode >= 1 && mode <= 4)
        expect(POLY);
    if (mode == 1)
    {
        out(D_80064C8C, 0, 0, 0);
        out(D_80064C98, 0, 0, g_cd_busy != -1 ? D_80064C9C : D_80064CAC);
        out(D_80064CBC, D_800B8FD4, D_800B8FD4, 0);
        out(D_80064CD8, 199 - D_800B8FD8, 199 - D_800B8FD8, 0);
        out(D_80064CEC, 0, 0, D_800A4D24[g_s2d_exit]);
        out(D_80064CF8, D_800B0EF0, 0, 0);
        out(D_80064D04, 0, 0, g_fade.unk04 ? D_80064D10 : D_80064D14);
        out(D_80064D18, tileValue, 0, 0);
        out(D_80064D24, 0x1080, 0, 0);
        out(D_80064D34, g_map_obj_count, maxObjects, 0);
        out(D_80064D54, packetSize, maxPacket, 0);
        out(D_80064D7C, *(u_char*)0x801F2B30, 0, 0);
        out(D_80064D8C, D_800B8650, 0, 0);
        out(D_80064DA0, 0, 0, g_key_lock == 1 ? D_80064DB4 : D_80064DBC);
        expect(FLUSH);
    }
    else if (mode == 2)
    {
        out(D_80064DC4, 0, 0, 0);
        for (i = 0; i < 12; i++)
        {
            marker(i);
            out(D_80064DE8, 0, 0, D_800A4E8C[i]);
            if (i >= 4 && i <= 8)
            {
                int value = i == 4 ? g_map_id : i == 5 ? *(u_char*)0x801F5354
                                            : i == 6   ? *(u_char*)0x801F5355
                                            : i == 7   ? g_map_pos_x
                                                       : g_map_pos_y;
                out(i == 4 ? D_80064DEC : D_80064DF8, value, value, 0);
            }
            else if (i == 9)
            {
                expect(FLAG);
                out(D_80064E04, g_s2d_selected_event_flag, g_s2d_selected_event_flag, eventFlag == 1 ? D_80064D10 : D_80064D14);
            }
            else if (i == 10)
                out(D_80064DE8, 0, 0, g_fade.unk08 == 1 ? D_80064D10 : D_80064D14);
            out(D_80064E18, 0, 0, 0);
        }
        out(D_80064E1C, 0, 0, 0);
        expect(FLUSH);
    }
    else if (mode == 3)
    {
        out(D_80064E24, 0, 0, 0);
        for (i = 0; i < 5; i++)
        {
            marker(i);
            out(D_80064DE8, 0, 0, D_800A4EBC[i]);
            if (i == 0)
            {
                out(D_80064E34, 0, 0, D_800A4D0C[0] ? D_80064D10 : D_80064D14);
                continue;
            }
            if (i < 4)
            {
                out(D_80064E3C, D_800A4D0C[i], D_800A4D0C[i], 0);
                if (((short*)0x801F538E)[i] != -1)
                    for (j = 0; j < frame % 5; j++)
                        out(D_80064E4C, 0, 0, 0);
            }
            out(D_80064E18, 0, 0, 0);
        }
        out(D_80064E1C, 0, 0, 0);
        expect(FLUSH);
    }
    else if (mode == 4)
    {
        out(D_80064E50, 0, 0, 0);
        for (i = 0; i < 9; i++)
        {
            out(D_80064DE8, 0, 0, D_800A4ED0[i]);
            if (i == 1 || i == 2 || i == 4)
            {
                int n = i == 1 ? D_800B8FD4 : i == 2 ? 199 - D_800B8FD8
                                                     : (D_800A4D34 & 255);
                out(D_80064E5C, n, n, 0);
            }
            else if (i == 3 || i == 8)
                out(D_80064E68, i == 3 ? (D_800A4D34 & 65535) : tileValue, 0, 0);
            else if (i == 5)
                out(D_80064DE8, 0, 0, D_80064E70.label[(D_800A4D34 >> 8) & 3]);
            else if (i == 6)
                out(D_80064DE8, 0, 0, D_80064E80.label[(D_800A4D34 >> 15) & 1]);
            out(D_80064E18, 0, 0, 0);
        }
        out(D_80064E1C, 0, 0, 0);
        expect(FLUSH);
        expect(TILES);
    }
    expect(SYNC);
    expect(VSYNC);
    expect(RESET);
    expect(PAGES);
    expect(EXTRA);
    expect(SWAP);
    expect(CLEAR);
    for (i = 0; i < 7; i++)
        expect(DRAW);
    expect(FINISH);
}
static void one(int seed)
{
    int        i;
    static int busyValues[] = { -1, 0, 1, 7 }, flagValues[] = { -1, 0, 1, 2 };
    cases++;
    printed = wanted = eventCount = wantEventCount = drawn = adds = cleared = sorted = flagsTested = 0;
    mode                                                                                           = seed % 7 - 1;
    frame                                                                                          = (seed / 7) % 10;
    side                                                                                           = (seed / 70) % 2;
    swap                                                                                           = (seed / 140) % 2;
    selected                                                                                       = (seed / 280) % 14 - 1;
    flushChange                                                                                    = (seed / 3920) % 2;
    syncMode                                                                                       = seed % 3 - 1;
    initialBlank                                                                                   = (seed / 3) % 2;
    afterBlank                                                                                     = initialBlank ^ flushChange;
    D_800A4D08                                                                                     = initialBlank;
    D_800A4D20                                                                                     = mode;
    D_800A4D1C                                                                                     = selected;
    g_draw_side                                                                                    = side;
    g_s2d_frame                                                                                    = frame;
    g_cd_busy                                                                                      = busyValues[seed % 4];
    g_key_lock                                                                                     = flagValues[(seed / 4) % 4];
    eventFlag                                                                                      = flagValues[(seed / 16) % 4];
    D_800B8FD4                                                                                     = (seed * 13) % 200;
    D_800B8FD8                                                                                     = (seed * 17) % 200;
    g_s2d_exit                                                                                     = seed % 4;
    g_map_id                                                                                       = (seed * 71) & 65535;
    g_map_pos_x                                                                                    = seed * 31;
    g_map_pos_y                                                                                    = seed * 19;
    g_s2d_selected_event_flag                                                                      = (short)(seed * 37);
    D_800A4D34                                                                                     = seed * 293 + 0xAA00;
    D_800B0EF0                                                                                     = -17;
    D_800B8650                                                                                     = seed + 40;
    D_800B0F18                                                                                     = seed % 11;
    g_fade.rgb[0]                                                                                  = 17;
    g_fade.rgb[1]                                                                                  = 39;
    g_fade.rgb[2]                                                                                  = 201;
    g_fade.unk04                                                                                   = seed % 3 - 1;
    g_fade.unk08                                                                                   = (seed / 3) % 3 - 1;
    packetSize                                                                                     = (seed * 23) % 1024;
    g_s2d_workbase                                                                                 = packets;
    g_map_obj_count                                                                                = seed % 161;
    D_800A4D3C = oldMaxObjects = (seed * 7) % 180;
    D_800A4D40 = oldMaxPacket = (seed * 37) % 1100;
    for (i = 0; i < 4; i++)
    {
        D_800A4D0C[i]           = seed % 3 - 1 + i;
        ((short*)0x801F538E)[i] = (seed & (1 << i)) ? i : -1;
    }
    *(u_char*)0x801F2B30                                          = seed * 11;
    *(u_char*)0x801F5354                                          = seed * 7;
    *(u_char*)0x801F5355                                          = seed * 5;
    *(u_int*)0x80122000                                           = 0x100;
    tileValue                                                     = (u_short)(seed * 331);
    *(u_short*)(0x80122100UL + D_800B8FD8 * 400 + D_800B8FD4 * 2) = tileValue;
    reference();
    func_80089F5C(syncMode);
    CHECK(eventCount == wantEventCount);
    for (i = 0; i < eventCount; i++)
        CHECK(events[i] == wantEvents[i]);
    CHECK(printed == wanted);
    for (i = 0; i < printed; i++)
    {
        CHECK(prints[i].format == wantPrints[i].format);
        CHECK(prints[i].a == wantPrints[i].a);
        CHECK(prints[i].b == wantPrints[i].b);
        CHECK(strcmp(prints[i].text, wantPrints[i].text) == 0);
    }
    CHECK(adds == 2 && drawn == 7 && cleared == 1);
    CHECK(sorted == (mode >= 1 && mode <= 4));
    CHECK(flagsTested == (mode == 2));
    CHECK(D_800A4D3C == (g_map_obj_count > oldMaxObjects ? g_map_obj_count : oldMaxObjects));
    CHECK(D_800A4D40 == (packetSize > oldMaxPacket ? packetSize : oldMaxPacket));
    CHECK(g_s2d_drawenv.clip.x == 123 && g_s2d_drawenv.clip.y == 47);
}
int main(void)
{
    int   i;
    void* ram = mmap((void*)0x80100000UL, 0x100000, 3, 0x100022, -1, 0);
    CHECK(ram == (void*)0x80100000UL);
    for (i = 0; i < 2; i++)
    {
        g_ot_back[i].tag = (GsOT_TAG*)(0x1000UL + i * 16);
        g_ot_obj[i].tag  = (GsOT_TAG*)(0x2000UL + i * 16);
    }
    for (i = 0; i < 7840; i++)
        one(i);
    CHECK(munmap(ram, 0x100000) == 0);
    printf("map frame: %d cases / %ld checks passed\n", cases, checks);
    return 0;
}
