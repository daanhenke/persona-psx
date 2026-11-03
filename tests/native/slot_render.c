/* Exercise the actual shared renderer with synthetic scripts and observed GPU
 * submissions. Low-address fixtures keep the original int-valued pointers. */
#define SKIP_ASM
#define WORK_BIAS 0
#include "../../src/common/gfx/slotrender.c"
extern int printf(const char*, ...);
extern void abort(void);
extern void* memset(void*, int, unsigned long);
extern void* mmap(void*, unsigned long, int, int, int, long);
extern int munmap(void*, unsigned long);
static unsigned long checks;
static int calls, expectedCalls, currentCase;
#define CHECK(c) do { checks++; if (!(c)) { printf("slot renderer check failed: line %d, case %d, call %d\n", __LINE__, currentCase, calls); abort(); } } while (0)
u_char g_flicker_ramp[32];
GsOT g_ot[2];
volatile int g_ot_index;
typedef struct { CelBlock header; u_short sx, sy; CelCell cells[6]; } ScaledBlock;
typedef struct { CelBlock header; CelCell cells[6]; } PlainBlock;
static GsSPRITE expected[12];
static u_short depths[12];
void GsSortSprite(GsSPRITE* sprite, GsOT* ot, unsigned short depth)
{
    GsSPRITE* e;
    CHECK(calls < expectedCalls); e = &expected[calls];
    CHECK(sprite == &g_sprites[calls] && ot == &g_ot[g_ot_index]);
    CHECK(depth == depths[calls]);
    CHECK(sprite->attribute == e->attribute && sprite->x == e->x && sprite->y == e->y);
    CHECK(sprite->w == e->w && sprite->h == e->h && sprite->tpage == e->tpage);
    CHECK(sprite->u == e->u && sprite->v == e->v && sprite->cx == e->cx && sprite->cy == e->cy);
    CHECK(sprite->scalex == e->scalex && sprite->scaley == e->scaley);
    CHECK(sprite->mx == e->mx && sprite->my == e->my && sprite->rotate == e->rotate);
    CHECK(sprite->r == e->r && sprite->g == e->g && sprite->b == e->b);
    calls++;
}
static void expectEntry(Slot* slot, CelEntry* entry, CelCell* cells, int scaled,
                        int offsetX, int offsetY, int brightness)
{
    int k;
    for (k = 0; k < 6; k++) {
        CelCell* c = cells + k;
        GsSPRITE* e;
        int u = (u_char)(c->u + slot->u_add), v = (u_char)(c->v + slot->v_add);
        if (u == 255 && v == 255) continue;
        e = &expected[expectedCalls]; memset(e, 0, sizeof(*e));
        e->attribute = (((u_long)entry->block->attr << 16) + ((slot->attr & 0x6000) << 9) + (slot->attr & SLOT_ATTR_SEMITRANS)) & 0x7FFFFFFF;
        e->x = entry->dx + offsetX + slot->x + slot->unk18 + (k % 3) * entry->block->w;
        e->y = entry->dy + offsetY + slot->y + slot->unk1A + (k / 3) * entry->block->h;
        e->w = entry->block->w; e->h = entry->block->h;
        e->tpage = c->tpage + slot->tpage_add;
        e->u = u; e->v = v; e->cx = c->cx + slot->clut_x; e->cy = c->cy + slot->clut_y;
        e->scalex = scaled ? ((ScaledBlock*)entry->block)->sx : (u_short)slot->scale_x;
        e->scaley = scaled ? ((ScaledBlock*)entry->block)->sy : (u_short)slot->scale_y;
        e->mx = scaled ? entry->block->w / 2 : (u_short)slot->mx;
        e->my = scaled ? entry->block->h / 2 : (u_short)slot->my;
        e->rotate = slot->rotate; e->r = e->g = e->b = brightness;
        depths[expectedCalls++] = entry->z + (slot->attr & SLOT_ATTR_Z);
    }
}
int main(void)
{
    void* high = mmap((void*)0x800C0000UL, 0x40000, 3, 0x100022, -1, 0);
    void* low = mmap((void*)0x10000000UL, 0x10000, 3, 0x100022, -1, 0);
    PlainBlock* plain = (PlainBlock*)low;
    ScaledBlock* scaled = (ScaledBlock*)((u_char*)low + 256);
    CelEntry* entries = (CelEntry*)((u_char*)low + 512);
    int* script = (int*)((u_char*)low + 768);
    int* branch = script + 16;
    int slotIndex, mode, light, side, k;
    if (high != (void*)0x800C0000UL || low != (void*)0x10000000UL) {
        printf("SKIP: fixed synthetic address ranges unavailable (including ASan shadow overlap)\n"); return 77;
    }
    for (k = 0; k < 32; k++) g_flicker_ramp[k] = (k * 7) & 127;
    for (slotIndex = 0; slotIndex < SLOT_COUNT; slotIndex++)
    for (mode = 0; mode < 10; mode++)
    for (light = 0; light < 6; light++)
    for (side = 0; side < 2; side++) {
        Slot* slot; int brightness, expectedDelay, expectedScript, expectedFrame, ox, oy, draw;
        currentCase++; calls = expectedCalls = 0; g_ot_index = side;
        memset(high, 0, 0x40000); memset(low, 0, 0x10000);
        for (k = 0; k < SLOT_COUNT; k++) { g_slots[k].script = -1; g_slots[k].frame = -1; }
        for (k = 0; k < 2; k++) {
            CelBlock* b = k ? &scaled->header : &plain->header;
            CelCell* cells = k ? scaled->cells : plain->cells;
            b->cols = 3; b->rows = 2; b->w = 17 + k; b->h = 11 + k; b->attr = k ? 0x8123 : 0x123;
            entries[k].block = b; entries[k].dx = 7 - k * 15; entries[k].dy = 5 + k * 3; entries[k].z = 20 + k;
            for (int j = 0; j < 6; j++) { cells[j].u = j * 9; cells[j].v = j * 5; cells[j].tpage = j + 2; cells[j].cx = j * 16; cells[j].cy = 100 + j; }
            cells[2].u = 254; cells[2].v = 253;
        }
        scaled->sx = 3000; scaled->sy = 5000; entries[2].block = (CelBlock*)-1;
        slot = &g_slots[slotIndex]; slot->active = 1; slot->script = -2; slot->frame = (int)(unsigned long)entries;
        slot->x = -41; slot->y = 25; slot->unk18 = 3; slot->unk1A = -7; slot->unk1C = 11; slot->unk1E = -13;
        slot->u_add = 1; slot->v_add = 2; slot->tpage_add = 5; slot->clut_x = 32; slot->clut_y = 17;
        slot->scale_x = 4096; slot->scale_y = 2048; slot->mx = 4; slot->my = 6; slot->rotate = -8192;
        slot->attr = 0x40006007; slot->brightness = 70; slot->fade_step = 9;
        *FADE_LEVEL = light == 5 ? 200 : light * 32;
        if (light == 1) { slot->attr |= SLOT_ATTR_FADE_IN; slot->brightness = 28; }
        if (light == 2) { slot->attr |= SLOT_ATTR_FADE_OUT; slot->brightness = 5; }
        if (light == 3) { slot->attr |= SLOT_ATTR_FLICKER; slot->flicker = 255; }
        brightness = slot->brightness;
        if (light == 1 && brightness < *FADE_LEVEL) { brightness += slot->fade_step; if (brightness > *FADE_LEVEL) brightness = *FADE_LEVEL; }
        if (light == 2) brightness = 0;
        if (light == 3) brightness = g_flicker_ramp[31];
        if (brightness > *FADE_LEVEL || *FADE_LEVEL > 128) brightness = *FADE_LEVEL;
        ox = 11; oy = -13; draw = 1; expectedDelay = 0; expectedScript = -2; expectedFrame = slot->frame;
        script[0] = slot->frame; script[1] = 2;
        switch (mode) {
        case 0: break; /* held frame */
        case 1: slot->script = (int)(unsigned long)script; expectedScript = (int)(unsigned long)(script + 2); expectedDelay = 1; ox = oy = 0; break;
        case 2: script[1] = 0x80000002; script[2] = 19; script[3] = -23; slot->script = (int)(unsigned long)script; expectedScript = (int)(unsigned long)(script + 4); expectedDelay = 1; ox = 19; oy = -23; break;
        case 3: script[0] = -1; slot->script = (int)(unsigned long)script; expectedScript = -1; expectedFrame = -1; expectedDelay = -1; draw = 0; break;
        case 4: script[0] = -2; slot->script = (int)(unsigned long)script; break;
        case 5: script[0] = -4; script[1] = 3; slot->script = (int)(unsigned long)script; expectedScript = (int)(unsigned long)(script + 2); expectedFrame = -1; expectedDelay = 2; draw = 0; break;
        case 6: branch[0] = slot->frame; branch[1] = 2; script[0] = -3; script[1] = (int)(unsigned long)branch; slot->script = (int)(unsigned long)script; expectedScript = (int)(unsigned long)(branch + 2); expectedDelay = 1; ox = oy = 0; break;
        case 7: slot->script = (int)(unsigned long)script; slot->delay = 5; expectedScript = slot->script; expectedDelay = 4; break;
        case 8: slot->active = 0; draw = 0; break;
        case 9: slot->attr |= SLOT_ATTR_HIDE; draw = 0; break;
        }
        if (draw) { expectEntry(slot, entries, plain->cells, 0, ox, oy, brightness); expectEntry(slot, entries + 1, scaled->cells, 1, ox, oy, brightness); }
        SlotsRenderAll();
        CHECK(calls == expectedCalls && slot->script == expectedScript && slot->frame == expectedFrame && slot->delay == expectedDelay);
        if (draw) { CHECK(slot->unk1C == ox && slot->unk1E == oy); if (light == 3) CHECK(slot->flicker == 0); if (light == 2) CHECK(slot->brightness == 0 && !(slot->attr & SLOT_ATTR_FADE_OUT)); }
    }
    printf("slot renderer: %d cases, %lu checks passed\n", currentCase, checks);
    munmap(high, 0x40000); munmap(low, 0x10000); return 0;
}
