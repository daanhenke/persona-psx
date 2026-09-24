/* Persona 1 (JP) - the field's sound handles, view and lighting.  DNG only.
 *   0x8006F150 FieldFadeSeqs
 *   0x8006F1BC FieldCloseSound
 *   0x8006F25C FieldSetView
 *   0x8006F3C8 FieldInitLight
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/dng/field.h>

/* Fades out every sequence the field has open. */
void FieldFadeSeqs(void)
{
    int i;

    for (i = 0; i < FIELD_SEQS; i++) {
        if (g_seq_handles[i] != -1) {
            SsSeqSetDecrescendo(g_seq_handles[i], 0x7F, 0x78);
        }
    }
}

/* Closes every sequence and VAB the field has open. */
void FieldCloseSound(void)
{
    int i;

    for (i = 0; i < FIELD_SEQS; i++) {
        if (g_seq_handles[i] != -1) {
            SsSetNck(g_seq_handles[i]);
        }
    }
    for (i = 0; i < FIELD_VABS; i++) {
        if (g_vab_handles[i] != -1) {
            SsVabClose(g_vab_handles[i]);
        }
    }
}

/* Points libgs's view along the party's heading, as FieldSetHeading does,
   and at eye height unless `keep_height` asks the current height kept. */
void FieldSetView(int keep_height)
{
    GsSetProjection(200);
    g_dng->view.vpx = g_dng->pos[POS_X] * STEP_LEN - ((g_scene->cos * 150) >> 12);
    g_dng->view.vpz = -g_dng->pos[POS_Y] * STEP_LEN - ((g_scene->sin * 150) >> 12);
    g_dng->view.vrx = g_dng->view.vpx + ((g_scene->cos * 5000) >> 12);
    g_dng->view.vrz = g_dng->view.vpz + ((g_scene->sin * 5000) >> 12);
    g_dng->view.rz = 0;
    if (!keep_height) {
        g_dng->view.vpy = -150;
        g_dng->view.vry = -150;
    }
    g_dng->view.super = 0;
    GsSetRefView2(&g_dng->view);
}

/* No flat light, no ambient, and the fog. The fog parameters are built in a
   local and never used - libgs is handed the ones in the data instead - but
   the arithmetic stays: gcc folds the product to a constant and keeps the
   libcalls that made it. */
void FieldInitLight(void)
{
    GsFOGPARAM fog;

    g_scene->light.vx = 0;
    g_scene->light.vy = 0;
    g_scene->light.vz = 0;
    g_scene->light.r = 0;
    g_scene->light.g = 0;
    g_scene->light.b = 0;
    GsSetFlatLight(0, &g_scene->light);
    GsSetAmbient(0, 0, 0);
    g_fog_near = 0x52D0;
    fog.dqa = -0x1A80;
    g_fog_scale = 4.27;
    fog.dqb = 4096.0 * g_fog_scale * 4096.0;
    fog.rfc = 0;
    fog.gfc = 0;
    fog.bfc = 0;
    GsSetFogParam(&g_fog);
    GsSetLightMode(1);
}
