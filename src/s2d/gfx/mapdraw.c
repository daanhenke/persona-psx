/* Persona 1 (JP) - drawing the map's pieces.  S2D.
 *   0x8008B48C S2dDrawMapObjs
 *
 * Each piece the map uses is lit and sorted through its own coordinate
 * system: the map's two kinds into the map's ordering table, everything else
 * into the objects' one at a finer shift.
 */
#include <decomp/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

extern void func_8008DA80(void);

void S2dDrawMapObjs(void)
{
    MATRIX     ls;
    MATRIX     lw;
    S2dMapObj *m;
    int        i;

    func_8008DA80();
    GsSetRefView2(&g_s2d_view);
    m = g_map_objs;
    /* The pointer steps ahead of the count, as the image has them. */
    for (i = 0; i < g_map_obj_count; m++, i++) {
        GsGetLws(m->obj.coord2, &lw, &ls);
        GsSetLightMatrix(&lw);
        GsSetLsMatrix(&ls);
        switch (m->kind) {
        case MAPOBJ_MAP:
            GsSortObject4(&m->obj, &g_ot_map[g_draw_side], 3, getScratchAddr(0));
            break;
        case MAPOBJ_MAP2:
            GsSortObject4(&m->obj, &g_ot_map[g_draw_side], 3, getScratchAddr(0));
            break;
        default:
            GsSortObject4(&m->obj, &g_ot_obj[g_draw_side], 8, getScratchAddr(0));
            break;
        }
    }
}
