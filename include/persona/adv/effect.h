#ifndef PERSONA_ADV_EFFECT_H
#define PERSONA_ADV_EFFECT_H

/* Persona 1 (JP) - the field's full-screen effects.  ADV only.
 *
 * An effect's sprites are played through a slot script (SlotInit): frames of
 * sixteen entries, each a cel and where it sits, closed by a terminator word.
 * Some effects rewrite the positions in their own script every frame, which
 * is how their sprites drift without a script of their own per position. */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ void  *cel;
    /* 0x04 */ short  x;
    /* 0x06 */ short  y;
    /* 0x08 */ int    unk08;
} EffectSprite;                   /* 0x0C bytes */

#define EFFECT_SPRITES 16

typedef struct {
    /* 0x00 */ EffectSprite spr[EFFECT_SPRITES];
    /* 0xC0 */ int          end;
} EffectFrame;                    /* 0xC4 bytes */

/* The drifting-sparkle effect's six frames. */
#define EFFECT3_FRAMES 6
extern EffectFrame g_effect3_frames[EFFECT3_FRAMES];

/* Counts up once a frame while an effect runs; its waves are all driven by
   it. */
extern u_int g_effect_tick;

#endif
