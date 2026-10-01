/* Persona 1 (JP) - CASINO's display setup.
 *   0x80066FB4 CasinoInitDraw
 *   0x80066FEC CasinoInitDrawEnv
 *
 * Two 320x240 buffers, one above the other, each drawn into while the
 * other is shown, both cleared to black.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern void CasinoResetLists(void);
extern void CasinoInitSprites(void);

void CasinoInitDrawEnv(CasinoDB *db);

void CasinoInitDraw(void)
{
    CasinoResetLists();
    CasinoInitDrawEnv(g_casino_db);
    CasinoInitSprites();
}

void CasinoInitDrawEnv(CasinoDB *db)
{
    InitGeom();
    SetGeomOffset(160, 120);
    SetGeomScreen(512);
    SetDefDrawEnv(db[0].draw, 0, 0, 320, 240);
    SetDefDispEnv(db[0].disp, 0, 240, 320, 240);
    SetDefDrawEnv(db[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(db[1].disp, 0, 0, 320, 240);
    db[0].draw->isbg = db[1].draw->isbg = 1;
    db[0].draw->r0 = 0;
    db[0].draw->g0 = 0;
    db[0].draw->b0 = 0;
    db[1].draw->r0 = 0;
    db[1].draw->g0 = 0;
    db[1].draw->b0 = 0;
}
