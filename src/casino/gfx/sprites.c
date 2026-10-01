/* Persona 1 (JP) - CASINO's frame: the draw lists and the swap.
 *   0x800680C0 CasinoResetLists
 *   0x80068140 CasinoInitSprites
 *   0x80068274 CasinoDrawFrame
 *   0x800683F4 CasinoDummy6
 *   0x800683FC CasinoAddSprites
 *
 * Each frame the lists are filled into the buffer not being shown, the
 * queued images go up to VRAM once the GPU is idle, the buffers swap and
 * the sprites are linked into the new ordering table.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <persona/casino/casino.h>

extern short D_800A8668;
extern short D_800A9120[2];
extern short D_800AAF5C;
extern short D_800B0A48[2];

extern void func_8006924C(void);
extern void CasinoFlushQuads(void);
extern void CasinoFlushUVs(void);
extern void func_80069F60(void);
extern void func_8006A150(void);
extern void CasinoFlushImages(void);
extern void func_8006ACBC(void);
extern void func_8006B1B0(void);
extern void func_8006B8B0(void);
extern void func_8006BFF0(void);
extern void func_8006C330(void);
extern void func_8006C5B0(void);
extern int  rand(void);

void CasinoAddSprites(CasinoDB *db);

void CasinoResetLists(void)
{
    g_casino_load_queue.n = 0;
    g_casino_quads.n[1] = 0;
    g_casino_quads.n[0] = 0;
    g_casino_uvs.n[1] = 0;
    g_casino_uvs.n[0] = 0;
    D_800B0974.n[1] = 0;
    D_800B0974.n[0] = 0;
    D_800A86A0.n[1] = 0;
    D_800A86A0.n[0] = 0;
    D_800B0A48[1] = 0;
    D_800B0A48[0] = 0;
    D_800A9120[1] = 0;
    D_800A9120[0] = 0;
    D_800AAF5C = 0;
    D_800A8668 = 0;
}

/* Every sprite starts as an opaque, untinted textured quad at the back of
   the table, switched off. */
void CasinoInitSprites(void)
{
    int i;

    for (i = 0; i < CASINO_SPRITES; i++) {
        SetPolyFT4(&g_casino_sprites.prim[0][i]);
        SetPolyFT4(&g_casino_sprites.prim[1][i]);
        SetSemiTrans(&g_casino_sprites.prim[0][i], 0);
        SetSemiTrans(&g_casino_sprites.prim[1][i], 0);
        g_casino_sprites.prim[0][i].r0 = 0;
        g_casino_sprites.prim[1][i].r0 = 0;
        g_casino_sprites.prim[0][i].g0 = 0;
        g_casino_sprites.prim[1][i].g0 = 0;
        g_casino_sprites.prim[0][i].b0 = 0;
        g_casino_sprites.prim[1][i].b0 = 0;
        g_casino_sprites.z[i] = 0xFF;
        g_casino_sprites.on[i] = 0;
    }
    g_casino_sprites.n = 0;
}

void CasinoDrawFrame(void)
{
    g_casino_frame++;
    g_casino_timer++;
    g_casino_buf = g_casino_frame & 1;
    g_casino_cur_db = &g_casino_db[g_casino_buf];
    func_8006B8B0();
    func_8006ACBC();
    func_8006B1B0();
    func_8006924C();
    CasinoFlushQuads();
    CasinoFlushUVs();
    func_8006A150();
    func_8006C330();
    func_8006C5B0();
    func_80069F60();
    func_8006BFF0();
    rand();
    DrawSync(0);
    if (g_casino_load_queue.n) {
        CasinoFlushImages();
    }
    DrawSync(0);
    SetDispMask(1);
    VSync(0);
    SetDispMask(1);
    PutDispEnv(g_casino_cur_db->disp);
    PutDrawEnv(g_casino_cur_db->draw);
    ClearOTag(g_casino_cur_db->ot, 0x400);
    CasinoAddSprites(g_casino_cur_db);
    DrawOTag(g_casino_cur_db->ot);
}

void CasinoDummy6(void)
{
}

void CasinoAddSprites(CasinoDB *db)
{
    short i;

    for (i = 0; i < g_casino_sprite_count; i++) {
        if (g_casino_sprites.on[i]) {
            AddPrim(db->ot + g_casino_sprites.z[i], &g_casino_sprites.prim[g_casino_buf][i]);
        }
    }
}
