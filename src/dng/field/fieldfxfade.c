/* Persona 1 (JP) - the fading battle transition.  DNG only.
 *   0x80071B94 FieldFadeFxBegin
 *   0x80071BFC FieldFadeFxRun
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* As FieldFxBegin, for the transition that fades the backdrop strip. */
void FieldFadeFxBegin(void)
{
    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    func_80071E04();
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
}

/* Steps the fade until the strip's first column has gone dark. */
void FieldFadeFxRun(void)
{
    while (g_scene->sprites[0].r != 0) {
        func_80071C50();
    }
}
