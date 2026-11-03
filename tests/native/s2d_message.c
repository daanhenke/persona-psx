/* Run the actual interpreter with synthetic scripts and observed UI calls.
 * Its fixed PS1 work/save addresses are private anonymous test mappings. */
#define SKIP_ASM
#define WORK_BIAS 0x20000
#include "../../src/s2d/gfx/bgreset.c"
extern int   printf(const char*, ...);
extern void  abort(void);
extern void* memset(void*, int, unsigned long);
extern void* mmap(void*, unsigned long, int, int, int, long);
extern int   munmap(void*, unsigned long);

GsBG             g_bg_layers[BG_LAYERS];
int              g_pad_held_s2d[1], g_pad_pressed[1], g_pad_held[1];
u_char           g_msg_answer, g_msg_choice_cur_def[8], g_member_names[100];
u_char           g_insert8_names[8][40], g_kind_labels[40];
u_long           g_msg_clut[16], *g_msg_blink[2];
MsgMark          g_msg_marks[15];
MsgChoice        g_msg_choices[8];
u_char*          g_msg_scripts[8];
ItemDef          g_item_defs[512];
SpellData        g_spell_data[256];
PersonaData      g_persona_data[256];
const PersonaDef g_persona_defs[4] = {
    [2] = { .name = { 0x57, 0xFF }, .kind = 3 }
};
static unsigned long checks, cases, calls;
static int           glyphs, cells, uploads, starts, stops, clears, cursorCalls, moved, accepted;
static int           positioned, px, py, rows[4], rowCount, tagged, flickered, found;
static unsigned      lastGlyph, lastCell;
static short         glyphX, glyphY;
static u_char        script[32], sub[40];
#define CHECK(c)                                                           \
    do                                                                     \
    {                                                                      \
        checks++;                                                          \
        if (!(c))                                                          \
        {                                                                  \
            printf("message: case %lu line %d failed\n", cases, __LINE__); \
            abort();                                                       \
        }                                                                  \
    } while (0)

void FontUploadGlyph(short x, short y, u_short glyph)
{
    glyphs++;
    lastGlyph = glyph;
    glyphX    = x;
    glyphY    = y;
}
void BgMapSetCell(u_short index, u_short glyph)
{
    cells++;
    lastCell = index;
    CHECK(glyph == lastGlyph);
}
void BgMapClearRow(u_short row)
{
    CHECK(row < 4 && rowCount < 4);
    rows[rowCount++] = row;
}
int MenuStepCursor(MenuList* m)
{
    CHECK(m == &g_msg->choice[cursorCalls]);
    cursorCalls++;
    return moved;
}
void MenuListInit(MenuList* m, int cur, int lo, int hi, u_short flags)
{
    m->cur   = cur;
    m->lo    = lo;
    m->hi    = hi;
    m->flags = flags;
}
u_char InputCheckAcceptA(u_char repeat)
{
    CHECK(repeat == 1);
    return accepted;
}
void SlotSetPos(u_char slot, int attr, short x, short y)
{
    CHECK(slot == 46 && attr == 30);
    positioned++;
    px = x;
    py = y;
}
void SlotClear(u_char slot)
{
    CHECK(slot == 46);
    clears++;
}
void SlotInitTagged(void* def, u_char slot, int attr, short x, short y)
{
    CHECK(def == g_msg_choice_cur_def && slot == 46 && attr == 30 && x == 40 && y == 180);
    tagged++;
}
void SlotSetFlicker(u_char slot, u_char on)
{
    CHECK(slot == 46 && on == 1);
    flickered++;
}
void QueueImageUpload(u_short* r, u_long* data)
{
    CHECK(r[0] == 32 && r[1] == 482 && r[2] == 11 && r[3] == 1 && data == g_msg_clut);
    uploads++;
}
void ImageAnimStart(int channel, u_long** data, u_short x, u_short y, u_short w, u_short h)
{
    CHECK(channel == 0 && data == g_msg_blink && x == 32 && y == 482 && w == 11 && h == 1);
    starts++;
}
void ImageAnimStop(int channel)
{
    CHECK(channel == 0);
    stops++;
}
short ItemsFind(u_short id)
{
    CHECK(id == 35);
    return found;
}
short FormatDecimal(u_int value, u_char* out, u_short width)
{
    unsigned i, n = 1;
    for (i = 0; i < width; i++)
    {
        out[i] = value % 10;
        value /= 10;
        if (out[i])
            n = i + 1;
    }
    return n;
}
static void reset(void)
{
    cases++;
    memset(g_msg, 0, sizeof(*g_msg));
    memset(script, 0, sizeof(script));
    memset(sub, 0, sizeof(sub));
    memset(g_msg_marks, 0x77, sizeof(g_msg_marks));
    g_msg->script     = script;
    g_msg->sub        = sub;
    g_msg->speed      = 3;
    g_pad_held_s2d[0] = 0;
    g_pad_pressed[0]  = 0;
    g_pad_held[0]     = 1;
    glyphs = cells = uploads = starts = stops = clears = cursorCalls = moved = accepted = 0;
    positioned = px = py = rowCount = tagged = flickered = 0;
    found                                                = -1;
    g_bg_layers[4].scrolly                               = 0;
    g_msg_answer                                         = 0xEE;
}
static int step(void)
{
    int i, result;
    for (i = 0; i < 64; i++)
        g_bg_slots[i] = 0xA5A50000U + i;
    result = MsgStep();
    calls++;
    for (i = 0; i < 64; i++)
        CHECK(g_bg_slots[i] == 0);
    CHECK(cells == glyphs);
    return result;
}
static void ordinary_glyphs(void)
{
    int cursor, wide, full, newline;
    for (cursor = 0; cursor <= 60; cursor++)
        for (wide = 0; wide < 2; wide++)
            for (full = 0; full < 2; full++)
                for (newline = 0; newline < 2; newline++)
                {
                    int index = cursor % 60;
                    reset();
                    g_msg->cursor = cursor;
                    g_msg->flags  = (full ? MSG_FULL : 0) | (newline ? MSG_NEWLINE : 0);
                    script[0]     = wide ? 0x83 : 0x42;
                    script[1]     = 0xA9;
                    CHECK(step() == 0 && glyphs == 1 && lastGlyph == (wide ? 0x3A9 : 0x42));
                    CHECK(lastCell == (unsigned)index && g_msg->cursor == index + 1 && g_msg->delay == 3);
                    CHECK(glyphX == (((index + 1) & 15) * 4 | 960) && glyphY == (((index + 1) & ~15) + 256));
                    CHECK(g_msg->script == script + 1 + wide);
                    CHECK(!!(g_msg->flags & MSG_FULL) == (full || index == 30));
                    CHECK(!!(g_msg->flags & MSG_SCROLL) == (full && index % 15 == 0 && !newline));
                }
}
static void pauses(void)
{
    int n, held;
    reset();
    g_msg->flags = MSG_DONE;
    CHECK(step() == 1 && glyphs == 0 && g_msg->script == script);
    for (n = 1; n < 12; n++)
    {
        reset();
        g_msg->wait = n;
        CHECK(step() == 0 && g_msg->wait == n - 1 && !glyphs);
        for (held = 0; held < 2; held++)
        {
            reset();
            script[0]         = 0x23;
            g_msg->delay      = n;
            g_pad_held_s2d[0] = held;
            g_pad_held[0]     = !held;
            CHECK(step() == 0 && glyphs == held && g_msg->delay == (held ? 3 : n - 1));
        }
    }
    reset();
    g_msg->flags = MSG_KEY;
    CHECK(step() == 0 && !glyphs && !uploads);
    reset();
    g_msg->flags     = MSG_KEY;
    g_pad_pressed[0] = 1;
    script[0]        = 0x33;
    CHECK(step() == 0 && glyphs == 1 && uploads == 1 && stops == 1 && !(g_msg->flags & MSG_KEY));
    for (n = 0; n < 16; n += 4)
    {
        reset();
        g_msg->flags           = MSG_SCROLL;
        g_msg->cursor          = 59;
        g_bg_layers[4].scrolly = n;
        CHECK(step() == 0 && !glyphs && g_bg_layers[4].scrolly == n + 4);
        CHECK(rowCount == (n == 12) && !!(g_msg->flags & MSG_SCROLL) == (n != 12));
        if (n == 12)
            CHECK(rows[0] == 0);
    }
}
static void choices(void)
{
    int a, b, yes, move;
    for (a = 0; a < 3; a++)
        for (b = 0; b < 4; b++)
            for (yes = 0; yes < 2; yes++)
                for (move = 0; move < 2; move++)
                {
                    unsigned answer = a ? (b ? ((b + 1) * a + b) : a) : (b ? b : a);
                    reset();
                    g_msg->flags         = MSG_CHOICE;
                    g_msg->choices       = 7;
                    g_msg->choice[0].cur = a;
                    g_msg->choice[0].hi  = a;
                    g_msg->choice[1].cur = b;
                    g_msg->choice[1].hi  = b;
                    accepted             = yes;
                    moved                = move;
                    CHECK(step() == 0 && cursorCalls == (move ? 1 : 2) && positioned == 1);
                    CHECK(px == 40 + 128 * b && py == 180 + 16 * a && !glyphs);
                    CHECK(g_msg_answer == (yes ? answer : 0xEE) && clears == yes && uploads == yes && stops == yes);
                    CHECK(!!(g_msg->flags & MSG_CHOICE) == !yes);
                }
    reset();
    g_msg->flags         = MSG_CHOICE;
    g_msg->choices       = 255;
    g_msg->choice[0].cur = 3000;
    g_msg->choice[1].cur = 3000;
    CHECK(step() == 0 && px == (short)(40 + 256 * 3000 * 16) && py == (short)(180 + 3000 * 16));
}
static void controls(void)
{
    int n, cursor;
    reset();
    script[0] = 255;
    script[1] = 1;
    CHECK(step() == 0 && (g_msg->flags & MSG_DONE));
    CHECK(step() == 1);
    reset();
    script[0] = 255;
    script[1] = 2;
    CHECK(step() == 0 && (g_msg->flags & MSG_KEY) && starts == 1);
    for (cursor = 0; cursor < 60; cursor++)
    {
        reset();
        g_msg->cursor = cursor;
        script[0]     = 255;
        script[1]     = 3;
        CHECK(step() == 0 && g_msg->cursor == ((cursor + 15) / 15) % 4 * 15 && (g_msg->flags & MSG_NEWLINE));
    }
    reset();
    g_msg->flags           = MSG_FULL;
    g_msg->cursor          = 27;
    g_bg_layers[4].scrolly = 40;
    script[0]              = 255;
    script[1]              = 4;
    CHECK(step() == 0 && rowCount == 4 && !g_msg->cursor && !g_bg_layers[4].scrolly && !(g_msg->flags & MSG_FULL));
    for (n = 0; n < 4; n++)
        CHECK(rows[n] == n);
    reset();
    script[0] = 255;
    script[1] = 5;
    script[2] = 0x34;
    script[3] = 0x12;
    CHECK(step() == 0 && g_msg->wait == 0x1234 && g_msg->script == script + 4);
    for (n = 0; n < 16; n++)
    {
        reset();
        g_msg->flags = MSG_NEWLINE | MSG_FULL;
        script[0]    = 255;
        script[1]    = 6;
        script[2]    = n;
        CHECK(step() == 0 && g_msg->flags == ((MSG_NEWLINE | MSG_FULL) & 0xFC0F) + (n << 4));
    }
    reset();
    g_msg_scripts[2] = sub;
    script[0]        = 255;
    script[1]        = 13;
    script[2]        = 2;
    sub[0]           = 0x6A;
    CHECK(step() == 0 && g_msg->sub == sub && g_msg->left == 255 && (g_msg->flags & MSG_SUB));
    CHECK(step() == 0 && lastGlyph == 0x6A);
    reset();
    g_msg_choices[2] = (MsgChoice){ sub, 2, 3, 6, 0 };
    sub[0]           = 0x66;
    script[0]        = 255;
    script[1]        = 14;
    script[2]        = 2;
    CHECK(step() == 0 && lastGlyph == 0x66 && g_msg->choices == 6 && g_msg->choice[0].hi == 1 && g_msg->choice[1].hi == 2);
    for (n = 0; n < 15; n++)
        CHECK(g_msg_marks[n].unk0 == (n < 6 ? 0 : 255) && g_msg_marks[n].unk1 == (n < 6 ? 180 : 255));
    reset();
    g_msg->flags = MSG_SUB | MSG_CHOOSING;
    sub[0]       = 255;
    sub[1]       = 1;
    CHECK(step() == 0 && tagged == 1 && flickered == 1 && starts == 1 && (g_msg->flags & MSG_CHOICE));
}
static void inserts(void)
{
    int op, n, back;
    for (op = 7; op <= 19; op++)
    {
        if (op == 13 || op == 14 || op == 17 || op == 18)
            continue;
        reset();
        script[0] = 255;
        script[1] = op;
        script[2] = 2;
        memset(g_member_names, 0x51, sizeof(g_member_names));
        memset(g_insert8_names, 0x52, sizeof(g_insert8_names));
        memset(g_item_defs[2].name, 0x53, 10);
        memset(g_spell_data[2].name, 0x54, 10);
        memset(g_persona_data[2].name, 0x55, 10);
        memset(g_kind_labels, 0x56, sizeof(g_kind_labels));
        CHECK(step() == 0 && glyphs == 1 && (g_msg->flags & MSG_SUB));
        CHECK(lastGlyph == (unsigned)(op == 8 ? 0x52 : op == 9 ? 0x53
                                                   : op == 10  ? 0x54
                                                   : op == 11  ? 0x55
                                                   : op == 12  ? 0x57
                                                   : op == 19  ? 0x56
                                                               : 0x51));
    }
    for (n = 0; n < 4; n++)
        for (back = 0; back < 2; back++)
        {
            reset();
            g_msg->flags = MSG_SUB | MSG_COUNTED | (back ? MSG_BACKWARD : 0);
            g_msg->left  = n;
            sub[1]       = 0x45;
            g_msg->sub   = sub + 1;
            CHECK(step() == 0 && glyphs == (n != 0));
            if (n)
                CHECK(g_msg->left == n - 1 && g_msg->sub == sub + (back ? 0 : 2));
            else
                CHECK(g_msg->script == script + 1 && !(g_msg->flags & (MSG_SUB | MSG_COUNTED | MSG_BACKWARD)));
        }
    reset();
    g_msg->flags = MSG_SURNAME;
    script[0]    = 2;
    memset(g_member_names, 0x51, sizeof(g_member_names));
    CHECK(step() == 0 && lastGlyph == 0 && g_msg->sub == g_member_names + 30 && g_msg->left == 4);
    for (n = 0; n < 2; n++)
    {
        reset();
        script[0]  = 255;
        script[1]  = n ? 18 : 17;
        g_money2   = 12345678;
        found      = 3;
        g_items[3] = (42 << 9) | 35;
        CHECK(step() == 0 && g_msg->left == (n ? 2 : 8) && (g_msg->flags & MSG_BACKWARD));
        CHECK(step() == 0 && lastGlyph == (unsigned)(n ? 0xC4 : 0xC1));
    }
    reset();
    script[0] = 255;
    script[1] = 18;
    CHECK(step() == 0 && g_msg->left == 1 && g_msg->digits[0] == 0xC0);
}
int main(void)
{
    void* memory = mmap((void*)0x80100000, 0x100000, 3, 0x100022, -1, 0);
    CHECK(memory == (void*)0x80100000);
    ordinary_glyphs();
    pauses();
    choices();
    controls();
    inserts();
    CHECK(munmap(memory, 0x100000) == 0);
    printf("s2d message: %lu cases, %lu interpreter calls, %lu checks passed\n", cases, calls, checks);
    return 0;
}
