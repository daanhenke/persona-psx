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

/* g_btl_spell_fx's own start handlers, in the order the image has them. The
   ones with something to be called after are named for it. */
extern void   BtlFxStartNone(void);   /* the one that answers nothing */
extern BtlObj *BtlFxStartLayers(void);
extern BtlObj *BtlFxStartSheetLate(void);
extern BtlObj *BtlFxStartLayers2(void);
extern BtlObj *BtlFxStartSideLayers(void);
extern BtlObj *BtlFxStartSheet(void);
extern BtlObj *BtlFxStartRing(void);
extern BtlObj *BtlFxStartOnAim(void);
extern BtlObj *BtlFxStartSheetLit(void);
extern BtlObj *BtlFxStartOnSide(void);
extern BtlObj *BtlFxStartSheetLit2(void);
extern BtlObj *BtlFxStartSweep(void);

/* The rest of the table's start handlers. Nothing calls one by hand - they are
   reached only through g_btl_spell_fx - so where a handler does no more than
   open one record there is nothing to name it after but the move whose record
   in the table points at it, and that id is what they carry, the way the fixed
   boards carry the picture they are drawn from. Several moves share a handler;
   the name is the lowest id that reaches it. In the order the image has them. */
extern BtlObj *BtlFxStart05(void);
extern BtlObj *BtlFxStart0D(void);
extern BtlObj *BtlFxStart0E(void);
extern BtlObj *BtlFxStart0F(void);
extern BtlObj *BtlFxStart10(void);
extern BtlObj *BtlFxStart11(void);
extern BtlObj *BtlFxStart13(void);
extern BtlObj *BtlFxStart14(void);
extern BtlObj *BtlFxStart15(void);
extern BtlObj *BtlFxStart16(void);
extern BtlObj *BtlFxStart17(void);
extern BtlObj *BtlFxStart18(void);
extern BtlObj *BtlFxStart19(void);
extern BtlObj *BtlFxStart1A(void);
extern BtlObj *BtlFxStart1B(void);
extern BtlObj *BtlFxStart1C(void);
extern BtlObj *BtlFxStart1D(void);
extern BtlObj *BtlFxStart1E(void);
extern BtlObj *BtlFxStart1F(void);
extern BtlObj *BtlFxStart20(void);
extern BtlObj *BtlFxStart21(void);
extern BtlObj *BtlFxStart22(void);
extern BtlObj *BtlFxStart23(void);
extern BtlObj *BtlFxStart24(void);
extern BtlObj *BtlFxStart25(void);
extern BtlObj *BtlFxStart26(void);
extern BtlObj *BtlFxStart27(void);
extern BtlObj *BtlFxStart28(void);
extern BtlObj *BtlFxStart29(void);
extern BtlObj *BtlFxStart2A(void);
extern BtlObj *BtlFxStart2B(void);
extern BtlObj *BtlFxStart2C(void);
extern BtlObj *BtlFxStart12(void);
extern BtlObj *BtlFxStart2E(void);
extern BtlObj *BtlFxStart2F(void);
extern BtlObj *BtlFxStart30(void);
extern BtlObj *BtlFxStart31(void);
extern BtlObj *BtlFxStart32(void);
extern BtlObj *BtlFxStart33(void);
extern BtlObj *BtlFxStart34(void);
extern BtlObj *BtlFxStart35(void);
extern BtlObj *BtlFxStart36(void);
extern BtlObj *BtlFxStart38(void);
extern BtlObj *BtlFxStart37(void);
extern BtlObj *BtlFxStart41(void);
extern BtlObj *BtlFxStart39(void);
extern BtlObj *BtlFxStart3A(void);
extern BtlObj *BtlFxStart3B(void);
extern BtlObj *BtlFxStart3C(void);
extern BtlObj *BtlFxStart3D(void);
extern BtlObj *BtlFxStart3F(void);
extern BtlObj *BtlFxStart40(void);
extern BtlObj *BtlFxStart43(void);
extern BtlObj *BtlFxStart42(void);
extern BtlObj *BtlFxStart44(void);
extern BtlObj *BtlFxStart45(void);
extern BtlObj *BtlFxStart46(void);
extern BtlObj *BtlFxStart47(void);
extern BtlObj *BtlFxStart48(void);
extern BtlObj *BtlFxStart49(void);
extern BtlObj *BtlFxStart4A(void);
extern BtlObj *BtlFxStart4B(void);
extern BtlObj *BtlFxStart4C(void);
extern BtlObj *BtlFxStart4D(void);
extern BtlObj *BtlFxStart4E(void);
extern BtlObj *BtlFxStart4F(void);
extern BtlObj *BtlFxStart50(void);
extern BtlObj *BtlFxStart51(void);
extern BtlObj *BtlFxStart52(void);
extern BtlObj *BtlFxStart53(void);
extern BtlObj *BtlFxStart54(void);
extern BtlObj *BtlFxStart55(void);
extern BtlObj *BtlFxStart56(void);
extern BtlObj *BtlFxStart57(void);
extern BtlObj *BtlFxStart58(void);
extern BtlObj *BtlFxStart59(void);
extern BtlObj *BtlFxStart5A(void);
extern BtlObj *BtlFxStart5B(void);
extern BtlObj *BtlFxStart5C(void);
extern BtlObj *BtlFxStart5D(void);
extern BtlObj *BtlFxStart5E(void);
extern BtlObj *BtlFxStart5F(void);
extern BtlObj *BtlFxStart60(void);
extern BtlObj *BtlFxStart61(void);
extern BtlObj *BtlFxStart62(void);
extern BtlObj *BtlFxStart63(void);
extern BtlObj *BtlFxStart64(void);
extern BtlObj *BtlFxStart65(void);
extern BtlObj *BtlFxStart66(void);
extern BtlObj *BtlFxStart67(void);
extern BtlObj *BtlFxStart68(void);
extern BtlObj *BtlFxStart69(void);
extern BtlObj *BtlFxStart6A(void);
extern BtlObj *BtlFxStart6E(void);
extern BtlObj *BtlFxStart6D(void);
extern BtlObj *BtlFxStart72(void);
extern BtlObj *BtlFxStart74(void);
extern BtlObj *BtlFxStartDB(void);
extern BtlObj *BtlFxStart75(void);
extern BtlObj *BtlFxStart76(void);
extern BtlObj *BtlFxStart77(void);
extern BtlObj *BtlFxStart78(void);
extern BtlObj *BtlFxStart79(void);
extern BtlObj *BtlFxStart7A(void);
extern BtlObj *BtlFxStart7B(void);
extern BtlObj *BtlFxStart7C(void);
extern BtlObj *BtlFxStart7D(void);
extern BtlObj *BtlFxStart7E(void);
extern BtlObj *BtlFxStart7F(void);
extern BtlObj *BtlFxStart80(void);
extern BtlObj *BtlFxStart81(void);
extern BtlObj *BtlFxStart82(void);
extern BtlObj *BtlFxStart83(void);
extern BtlObj *BtlFxStart84(void);
extern BtlObj *BtlFxStart86(void);
extern BtlObj *BtlFxStart87(void);
extern BtlObj *BtlFxStart88(void);
extern BtlObj *BtlFxStart8C(void);
extern BtlObj *BtlFxStart90(void);
extern BtlObj *BtlFxStart91(void);
extern BtlObj *BtlFxStart92(void);
extern BtlObj *BtlFxStart93(void);
extern BtlObj *BtlFxStart94(void);
extern BtlObj *BtlFxStart95(void);
extern BtlObj *BtlFxStart96(void);
extern BtlObj *BtlFxStart97(void);
extern BtlObj *BtlFxStart98(void);
extern BtlObj *BtlFxStart99(void);
extern BtlObj *BtlFxStart9A(void);
extern BtlObj *BtlFxStart9B(void);
extern BtlObj *BtlFxStart9C(void);
extern BtlObj *BtlFxStart9D(void);
extern BtlObj *BtlFxStart9E(void);
extern BtlObj *BtlFxStart9F(void);
extern BtlObj *BtlFxStartA0(void);
extern BtlObj *BtlFxStartA1(void);
extern BtlObj *BtlFxStartA2(void);
extern BtlObj *BtlFxStartE0(void);
extern BtlObj *BtlFxStartE6(void);
extern BtlObj *BtlFxStartE9(void);
extern BtlObj *BtlFxStartEA(void);
extern BtlObj *BtlFxStartEB(void);
extern BtlObj *BtlFxStartEE(void);
extern BtlObj *BtlFxStartEF(void);
extern BtlObj *BtlFxStartF0(void);
extern BtlObj *BtlFxStartF2(void);
extern BtlObj *BtlFxStartE4(void);
extern BtlObj *BtlFxStartE5(void);
extern BtlObj *BtlFxStartE8(void);
extern BtlObj *BtlFxStartEC(void);
extern BtlObj *BtlFxStartED(void);
extern BtlObj *BtlFxStartF1(void);
extern BtlObj *BtlFxStartF3(void);
extern BtlObj *BtlFxStartF4(void);
extern BtlObj *BtlFxStartF5(void);
extern BtlObj *BtlFxStartF6(void);

#endif
