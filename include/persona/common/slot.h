#ifndef PERSONA_COMMON_SLOT_H
#define PERSONA_COMMON_SLOT_H

/* Persona 1 (JP) - display slot records.
 *
 * The slot routines are compiled into more than one overlay rather than called
 * across the boundary, so DNG and ADV share src/p1-jp/common/gfx/slot.c while
 * S2D, which indexes a table at a different address, keeps
 * src/p1-jp/s2d/gfx/slot.c. The record layout is the same in both, which is why
 * it lives here.
 *
 * The table base is deliberately *not* declared here: which table an overlay
 * uses is a per-overlay fact, so each source defines its own g_slots.
 *
 * A slot is one animated sprite. Every frame the renderer (ADV 0x80065730)
 * walks all SLOT_COUNT of them, steps each one's animation script, and emits
 * one GsSPRITE per cel into a 512-entry array, sorting it into the OT at a
 * depth taken from the attribute word. Nearly every field below is either an
 * offset added to the script's own value or a value passed straight through to
 * the GsSPRITE - the names come from which GsSPRITE member the renderer writes
 * it into.
 */
#include <types.h>

/* 0x44-byte records, indexed by a u8 slot. */
typedef struct {
    /* 0x00 */ int    script;       /* animation script; -1 free, -2 stopped   */
    /* 0x04 */ int    frame;        /* current cel list; -1 = nothing to draw  */
    /* 0x08 */ int    attr;         /* see SLOT_ATTR_* below                   */
    /* 0x0C */ int    delay;        /* frames left on the current script step  */
    /* 0x10 */ int    rotate;       /* -> GsSPRITE.rotate                      */
    /* 0x14 */ short  x;            /* -> GsSPRITE.x, with unk18 and unk1C     */
    /* 0x16 */ short  y;            /* -> GsSPRITE.y, with unk1A and unk1E     */
    /* 0x18 */ short  unk18;        /* x offset, set by SlotSetAnim            */
    /* 0x1A */ short  unk1A;        /* y offset, set by SlotSetAnim            */
    /* 0x1C */ short  unk1C;        /* x offset, set from the script itself    */
    /* 0x1E */ short  unk1E;        /* y offset, set from the script itself    */
    /* 0x20 */ short  clut_x;       /* added to the cel's GsSPRITE.cx          */
    /* 0x22 */ short  clut_y;       /* added to the cel's GsSPRITE.cy          */
    /* 0x24 */ short  mx;           /* -> GsSPRITE.mx (rotation centre)        */
    /* 0x26 */ short  my;           /* -> GsSPRITE.my                          */
    /* 0x28 */ short  scale_x;      /* -> GsSPRITE.scalex, 0x1000 = 1.0        */
    /* 0x2A */ short  scale_y;      /* -> GsSPRITE.scaley                      */
    /* 0x2C */ short  brightness;   /* -> GsSPRITE r=g=b, 0x80 = full          */
    /* 0x2E */ u_char tpage_add;    /* added to the cel's GsSPRITE.tpage       */
    /* 0x2F */ u_char u_add;        /* added to the cel's GsSPRITE.u           */
    /* 0x30 */ u_char v_add;        /* added to the cel's GsSPRITE.v           */
    /* 0x31 */ u_char flicker;      /* free-running phase, & 0x1f indexes a
                                       32-entry brightness table              */
    /* 0x32 */ u_char active;       /* zero means skip the slot entirely       */
    /* 0x33 */ u_char fade_step;    /* brightness change per frame while
                                       SLOT_ATTR_FADE_IN/OUT is set           */
    /* 0x34 */ int    unk34;
    /* 0x38 */ int    unk38;
    /* 0x3C */ int    unk3C;
    /* 0x40 */ int    unk40;
} Slot;

/* The attribute word is part sort key, part flags. The renderer sorts each
   sprite into the OT at `cel_z + (attr & 0xFFF)`, so the bottom 12 bits are a
   depth; SlotSetPos rewrites only the bottom 10 of them. Bits 13-14 and bit 30
   are forwarded into GsSPRITE.attribute, and bit 31 is masked off there so a
   slot is never handed to libgs marked "do not display". */
#define SLOT_ATTR_Z        0x00000FFF
#define SLOT_ATTR_FLICKER  0x00001000  /* brightness cycles through a table   */
#define SLOT_ATTR_HIDE     0x00008000  /* skip without clearing the slot      */
#define SLOT_ATTR_FADE_IN  0x00010000  /* ramp brightness up to g_fade_level  */
#define SLOT_ATTR_FADE_OUT 0x00020000  /* ramp brightness down to zero        */
#define SLOT_ATTR_SEMITRANS 0x40000000 /* -> GsSPRITE.attribute bit 30        */

/* 80 records. Both bounds in the overlay agree: SlotClearAll counts 79 down to
   0, and the renderer's slot loop ends with `if (i > 0x4f) return`. */
#define SLOT_COUNT 80

extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern void SlotClear(u_char slot);
extern void SlotClearAll(void);
extern void SlotSetFlicker(u_char slot, u_char on);
extern void SlotSetAnim(short slot, short unk18, short unk1A, u_char tpage_add,
                        u_char u_add, u_char v_add, u_short clut_x,
                        u_short clut_y);

#endif
