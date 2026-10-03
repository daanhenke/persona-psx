/* Persona 1 (JP) - the field's party and its frame.  S2D.
 *   0x8008E8B4 S2dDrawParty
 *
 * The party is two placed objects, g_party_obj and [1]. Left standing
 * still long enough on a map that allows it, one of them starts an idle
 * animation picked at random: [1] wobbles, or (one time in 256) [0]
 * breathes. Moving again puts both back.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

/* Frames the party has to stand still before an idle animation starts. */
#define IDLE_FRAMES 9000

/* The idle animation: none picked yet, the wobble, the breath. */
#define IDLE_PICK   0
#define IDLE_WOBBLE 1
#define IDLE_BREATH 2

extern int   D_800B0E84[];
extern int   D_800A4CF0;
extern int   g_idle_frames;
extern int   g_idle_kind;
extern int   g_idle_phase;
extern short D_800A4CDC;

extern void func_8008D988(VECTOR *trans, SVECTOR *rot, VECTOR *scale,
                          GsCOORDINATE2 *coord);

void S2dDrawParty(GsOT *ot, int shift)
{
    MATRIX     ls;
    MATRIX     lw;
    SVECTOR    rot;
    S2dMapObj *o;

    GsSetRefView2(&g_s2d_view);
    if (D_800B0E84[8] == 0 && D_800A4CF0 == 1) {
        g_idle_frames++;
        if (g_idle_frames > IDLE_FRAMES) {
            g_idle_frames = IDLE_FRAMES;
        }
        if (g_idle_frames == IDLE_FRAMES) {
            switch (g_idle_kind) {
            case IDLE_PICK:
                if (!(rand() & 0xFF)) {
                    g_idle_kind = IDLE_BREATH;
                } else {
                    g_idle_kind = IDLE_WOBBLE;
                }
                break;
            case IDLE_WOBBLE:
                g_idle_phase++;
                g_party_mark.scale.vz =
                    (((rsin(g_idle_phase * 80 + 0xA00) + ONE) * 0x400) >> 12) + ONE;
                break;
            case IDLE_BREATH:
                g_idle_phase++;
                g_party_obj.scale.vx =
                    ((((rsin(g_idle_phase * 80 + 0xC00) + ONE) >> 1) * 0xC00) >> 12) + 0x1800;
                g_party_obj.scale.vz = g_party_obj.scale.vx;
                g_party_obj.scale.vy = g_party_obj.scale.vx;
                break;
            }
        }
    } else {
        g_party_mark.scale.vz = ONE;
        g_party_mark.rot.vz = 0;
        g_party_obj.scale.vx = 0x1800;
        g_party_obj.scale.vy = 0x1800;
        g_party_obj.scale.vz = 0x1800;
        g_idle_frames = 0;
        g_idle_kind = IDLE_PICK;
        g_idle_phase = 0;
    }

    o = &g_party_obj;
    g_party_obj.rot.vy += D_800A4CDC;
    func_8008D988(&o->trans, &o->rot, &o->scale, &o->coord);
    GsGetLws(g_party_obj.obj.coord2, &lw, &ls);
    GsSetLightMatrix(&lw);
    GsSetLsMatrix(&ls);
    GsSortObject4(&o->obj, ot, shift, getScratchAddr(0));

    o = &g_party_mark;
    rot = g_party_mark.rot;
    func_8008D988(&o->trans, &rot, &o->scale, &o->coord);
    GsGetLws(g_party_mark.obj.coord2, &lw, &ls);
    GsSetLightMatrix(&lw);
    GsSetLsMatrix(&ls);
    GsSortObject4(&o->obj, ot, shift, getScratchAddr(0));

    o = &D_800B0F1C;
    if (o->kind != 0) {
        func_8008D988(&o->trans, &o->rot, &o->scale, &o->coord);
        GsGetLws(D_800B0F1C.obj.coord2, &lw, &ls);
        GsSetLightMatrix(&lw);
        GsSetLsMatrix(&ls);
        GsSortObject4(&o->obj, &g_ot_back[g_draw_side], 10, getScratchAddr(0));
    }
}
