/* Persona 1 (JP) - fading the field in and out, and pausing its tune.
 * DNG only.
 *   0x8006A2D0 FieldFadeOut
 *   0x8006A3CC FieldFadeIn
 *   0x8006A4D0 FieldPauseBgm
 *
 * The fades step the ambient light and the tint of every flat sprite on the
 * field together, a frame at a time, redrawing between steps.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/dng/field.h>

#define FADE_FULL 0x1000
#define FADE_STEP 0x200

/* The sprites a fade tints: the backdrop strip and what is drawn with it. */
#define TINT_SPRITES 105

/* Tints the field's sprites for light level `level`: GsSetAmbient takes
   0-0x1000, a sprite's colour 0-0x80. Each colour is one chained assignment,
   so g_scene is read once per sprite rather than once per channel. */
#define FIELD_TINT(level, c)                                                   \
    {                                                                          \
        int i_;                                                                \
                                                                               \
        GsSetAmbient(level, level, level);                                     \
        for (i_ = 0; i_ < TINT_SPRITES; i_++) {                               \
            g_scene->sprites[i_].r = g_scene->sprites[i_].g =                  \
                g_scene->sprites[i_].b = c = (level) / 32;                     \
        }                                                                      \
        g_scene->layers[LAYER_BACKDROP].bg.r = g_scene->layers[LAYER_BACKDROP].bg.g = g_scene->layers[LAYER_BACKDROP].bg.b = c;   \
        g_scene->layers[LAYER_SKY].bg.r = g_scene->layers[LAYER_SKY].bg.g = g_scene->layers[LAYER_SKY].bg.b = c;                  \
    }

void FieldFadeOut(void)
{
    int    level;
    u_char c;

    level = FADE_FULL;
    do {
        FIELD_TINT(level, c);
        level -= FADE_STEP;
        func_80065978();
    } while (level > 0);
    GsSetAmbient(0, 0, 0);
    g_field_lit = 0;
    func_80065978();
}

void FieldFadeIn(void)
{
    int    level;
    u_char c;

    g_field_lit = 1;
    level = 0;
    do {
        FIELD_TINT(level, c);
        level += FADE_STEP;
        func_80065978();
    } while (level < FADE_FULL);
    GsSetAmbient(FADE_FULL, FADE_FULL, FADE_FULL);
    func_80065978();
}

/* Stops the floor's tune while the party stands still, playing the idle
   jingle in its place unless the music is off. Declared int with nothing
   returned, like FieldStepTick: v0 stays live at the exit, so the flag's
   mask is not moved into the delay slot of the test. */
int FieldPauseBgm(void)
{
    SsSeqPause(g_bgm_seq);
    if (g_bgm_flags & BGM_RESUMED) {
        g_bgm_flags &= ~BGM_RESUMED;
        if (g_bgm_off == 0) {
            SsPlayBack(g_idle_seq, 0, 1);
        }
    }
}
