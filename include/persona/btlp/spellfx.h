/* Persona 1 (JP) - the effect a move is played out with.  BTLP only.
 *
 * Every move the round plays out is drawn by the same machinery: the artwork
 * is staged once by BtlStartMoveFx, the move's record in g_btl_spell_fx says
 * which handler builds the effect, and the handler makes its records out of
 * the object pool's effects group. The handlers take no arguments - they are
 * reached through the table - so which move is being played out and what it
 * is aimed at are left in g_btl_fx_move and g_btl_fx_target instead.
 *
 * The records themselves are all opened the same way, which is what the three
 * openers below are: one object on a fighter, three layers on a fighter, and a
 * five-by-five grid over the field. Each of them fills g_btl_fx_def in from
 * whichever of the staged script tables it wants first, so the template is
 * only ever right for the call being made.
 */
#ifndef PERSONA_BTLP_SPELLFX_H
#define PERSONA_BTLP_SPELLFX_H

#include <decomp/types.h>
#include <persona/btlp/object.h>

/* What a spell does, indexed by the same id g_spell_data is. Record zero is
   the empty one, so the table is as long as the spell list plus its head. The
   three handlers are the effect's start, its step and its finish; `group`
   sorts the spell into one of five families. A record whose start handler is
   absent has no effect at all, and spelllines.c draws such a spell greyed. */
typedef struct {
    /* 0x0 */ BtlObj *(*start)();
    /* 0x4 */ void (*step)();
    /* 0x8 */ void (*finish)();
    /* 0xC */ int  group;
} BtlSpellFx;                          /* 16 bytes */

extern BtlSpellFx g_btl_spell_fx[];

/* What the handlers are told, in place of arguments. */
extern u_char g_btl_fx_move;
extern u_char g_btl_fx_target;

/* Where the staged artwork is, which BtlBindGfx is handed the address of, and
   the spare graphics slot the script tables are read out of. */
extern u_char *g_btl_fx_gfx;
extern u_char *g_btl_unused_gfx;

/* The template every effect record is built from, filled in afresh before
   each allocation from the staged artwork's own script table. */
extern BtlObjDef g_btl_fx_def;

/* The colour a fighter is put on while an effect stands over it. */
extern u_char g_btl_tint_fx_r;
extern u_char g_btl_tint_fx_g;
extern u_char g_btl_tint_fx_b;

/* How far each layer of a spread move stands from the middle, in pixels. */
extern signed char g_btl_fx_shift[];

/* The group effect records come out of, and what one is drawn as. */
#define FX_OBJ_GROUP 2
#define FX_OBJ_DRAW  5

/* The two bytes BtlObjAlloc leaves at +0xCD and +0xCE. */
#define FX_OBJ_CD 0x1D
#define FX_OBJ_CE 0xE

/* What a plain effect record starts as: hidden, static, and without a
   shadow. */
#define FX_OBJ_ATTR (BTL_OBJ_HIDDEN | BTL_OBJ_STATIC | BTL_OBJ_NO_SHADOW)

extern BtlObj *BtlStartMoveFx(int index);
extern BtlObj *BtlOpenFxObj(int slot, int timer);
extern BtlObj *BtlOpenFxObj2(int slot, int timer);
extern BtlObj *BtlOpenFxLayers(int slot, int timer);
extern BtlObj *BtlOpenFxOnTargets(int r, int g, int b, int timer);
extern BtlObj *BtlOpenFxGrid(int table);

/* Starts every live fighter the acting record reaches walking toward one
   colour. The slot is not read - the routine takes the mask off
   g_btl_actor_turn itself - but every caller hands it one. */
extern void BtlTintTargets(int slot, short r, short g, short b);

/* g_btl_spell_fx's own start handlers, in the order the image has them. */
extern BtlObj *BtlFxStartLayers(void);
extern BtlObj *BtlFxStartLayers2(void);
extern BtlObj *BtlFxStartSheet(void);
extern BtlObj *BtlFxStartOnAim(void);
extern BtlObj *BtlFxStartSheetLit(void);
extern BtlObj *BtlFxStartSheetLit2(void);

#endif
