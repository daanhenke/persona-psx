/* Persona 1 (JP) - the table that says how every move is played out.
 * BTLP only.
 *   0x800D0484 g_btl_spell_fx
 *
 * 247 records of four words, indexed by the same move id g_spell_data is, so
 * record zero is the empty one in front of the list. The three handlers are the
 * effect's start, its step and its finish; the fourth word sorts the move into
 * one of five families.
 *
 * BtlStartMoveFx stages the artwork and then reaches the start handler through
 * here; BtlFxObjTick hands each frame to the step handler, and what the record
 * is holding when it ends goes to the finish. A record whose start handler is
 * absent has no effect at all, and spelllines.c draws such a move greyed - 6
 * of the 247 are like that, which is why two thirds of the start column is one
 * handler that answers nothing.
 *
 * Handlers are shared freely. 27 step handlers cover every move that
 * has one and 5 finish handlers cover every move that has one, while the
 * start column names 168 handlers - the effect a move opens with is what
 * distinguishes it, and how the effect is carried and resolved is not.
 *
 * The step and finish handlers are still assembly and have not been worked out,
 * so they stand here under the addresses splat gave them. The start handlers
 * are named for the lowest move id that reaches each; see spellfx.h.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The step column - one per frame while the effect stands. */
extern void func_800B7AC4();
extern void func_800B8C30();
extern void func_800B8FF4();
extern void func_800B9774();
extern void func_800B98E4();
extern void func_800B9EF0();
extern void func_800BA4FC();
extern void func_800BA7A0();
extern void func_800BAE5C();
extern void func_800BB5BC();
extern void func_800BBD28();
extern void func_800BBFDC();
extern void func_800BC39C();
extern void func_800BC99C();
extern void func_800BCF6C();
extern void func_800BD3A8();
extern void func_800BD628();
extern void func_800BDBA8();
extern void func_800BDFB4();
extern void func_800BE3C8();
extern void func_800BE670();
extern void func_800BE9E8();
extern void func_800BF0F8();
extern void D_800BF55C();
extern void func_800BF7F8();
extern void func_800BFA04();
extern void func_800C09D0();

/* The finish column - what the record is holding when the effect ends. */
extern void func_800C0E54();
extern void func_800C10E0();
extern void func_800C17E8();
extern void func_800C23CC();
extern void func_800C2D54();

/* Which of the five families a move sorts into. The counts are 153 of group 0, 16 of group 1, 10 of group 2, 8 of group 3, 60 of group 4. */

BtlSpellFx g_btl_spell_fx[] = {
    /* 0x00 */ { 0, 0, 0, 0 },
    /* 0x01 */ { BtlFxStartLayers, func_800C09D0, func_800C0E54, 1 },
    /* 0x02 */ { BtlFxStartLayers2, func_800C09D0, func_800C0E54, 1 },
    /* 0x03 */ { BtlFxStartSheetLate, func_800C09D0, func_800C0E54, 1 },
    /* 0x04 */ { BtlFxStartSideLayers, func_800C09D0, func_800C0E54, 1 },
    /* 0x05 */ { BtlFxStart05, func_800C09D0, func_800C0E54, 1 },
    /* 0x06 */ { BtlFxStartSheet, func_800C09D0, func_800C0E54, 1 },
    /* 0x07 */ { BtlFxStartRing, func_800C09D0, func_800C0E54, 2 },
    /* 0x08 */ { BtlFxStartOnAim, func_800C09D0, func_800C0E54, 2 },
    /* 0x09 */ { BtlFxStartSheetLit, func_800B7AC4, func_800C0E54, 2 },
    /* 0x0A */ { BtlFxStartSheetLit2, func_800C09D0, func_800C0E54, 2 },
    /* 0x0B */ { BtlFxStartOnSide, func_800C09D0, func_800C0E54, 2 },
    /* 0x0C */ { BtlFxStartSweep, func_800C09D0, func_800C0E54, 2 },
    /* 0x0D */ { BtlFxStart0D, func_800C09D0, func_800C0E54, 0 },
    /* 0x0E */ { BtlFxStart0E, func_800C09D0, func_800C0E54, 0 },
    /* 0x0F */ { BtlFxStart0F, func_800C09D0, func_800C0E54, 0 },
    /* 0x10 */ { BtlFxStart10, func_800C09D0, func_800C0E54, 0 },
    /* 0x11 */ { BtlFxStart11, func_800C09D0, func_800C0E54, 0 },
    /* 0x12 */ { BtlFxStart12, func_800B9774, func_800C0E54, 0 },
    /* 0x13 */ { BtlFxStart13, func_800C09D0, func_800C0E54, 0 },
    /* 0x14 */ { BtlFxStart14, func_800C09D0, func_800C0E54, 0 },
    /* 0x15 */ { BtlFxStart15, func_800C09D0, func_800C0E54, 0 },
    /* 0x16 */ { BtlFxStart16, func_800C09D0, func_800C0E54, 0 },
    /* 0x17 */ { BtlFxStart17, func_800C09D0, func_800C0E54, 0 },
    /* 0x18 */ { BtlFxStart18, func_800C09D0, func_800C0E54, 0 },
    /* 0x19 */ { BtlFxStart19, func_800C09D0, func_800C0E54, 1 },
    /* 0x1A */ { BtlFxStart1A, func_800C09D0, func_800C0E54, 1 },
    /* 0x1B */ { BtlFxStart1B, func_800C09D0, func_800C0E54, 1 },
    /* 0x1C */ { BtlFxStart1C, func_800C09D0, func_800C0E54, 1 },
    /* 0x1D */ { BtlFxStart1D, func_800C09D0, func_800C0E54, 1 },
    /* 0x1E */ { BtlFxStart1E, func_800C09D0, func_800C0E54, 1 },
    /* 0x1F */ { BtlFxStart1F, func_800C09D0, func_800C0E54, 0 },
    /* 0x20 */ { BtlFxStart20, func_800C09D0, func_800C0E54, 0 },
    /* 0x21 */ { BtlFxStart21, func_800C09D0, func_800C0E54, 0 },
    /* 0x22 */ { BtlFxStart22, func_800C09D0, func_800C0E54, 0 },
    /* 0x23 */ { BtlFxStart23, func_800C09D0, func_800C0E54, 0 },
    /* 0x24 */ { BtlFxStart24, func_800C09D0, func_800C0E54, 0 },
    /* 0x25 */ { BtlFxStart25, func_800C09D0, func_800C0E54, 0 },
    /* 0x26 */ { BtlFxStart26, func_800C09D0, func_800C0E54, 0 },
    /* 0x27 */ { BtlFxStart27, func_800B8C30, func_800C0E54, 0 },
    /* 0x28 */ { BtlFxStart28, func_800C09D0, func_800C0E54, 0 },
    /* 0x29 */ { BtlFxStart29, func_800C09D0, func_800C0E54, 0 },
    /* 0x2A */ { BtlFxStart2A, func_800B8FF4, func_800C0E54, 0 },
    /* 0x2B */ { BtlFxStart2B, func_800C09D0, func_800C0E54, 3 },
    /* 0x2C */ { BtlFxStart2C, func_800C09D0, func_800C0E54, 3 },
    /* 0x2D */ { BtlFxStart12, func_800B9774, func_800C0E54, 3 },
    /* 0x2E */ { BtlFxStart2E, func_800C09D0, func_800C0E54, 3 },
    /* 0x2F */ { BtlFxStart2F, func_800B98E4, func_800C0E54, 3 },
    /* 0x30 */ { BtlFxStart30, func_800C09D0, func_800C0E54, 3 },
    /* 0x31 */ { BtlFxStart31, func_800C09D0, func_800C0E54, 0 },
    /* 0x32 */ { BtlFxStart32, func_800C09D0, func_800C0E54, 0 },
    /* 0x33 */ { BtlFxStart33, func_800B9EF0, 0, 0 },
    /* 0x34 */ { BtlFxStart34, func_800C09D0, func_800C0E54, 0 },
    /* 0x35 */ { BtlFxStart35, func_800C09D0, func_800C0E54, 0 },
    /* 0x36 */ { BtlFxStart36, func_800BA4FC, func_800C0E54, 0 },
    /* 0x37 */ { BtlFxStart37, func_800C09D0, func_800C23CC, 0 },
    /* 0x38 */ { BtlFxStart38, func_800BA7A0, func_800C23CC, 0 },
    /* 0x39 */ { BtlFxStart39, func_800C09D0, func_800C23CC, 0 },
    /* 0x3A */ { BtlFxStart3A, func_800BAE5C, func_800C23CC, 0 },
    /* 0x3B */ { BtlFxStart3B, func_800C09D0, func_800C23CC, 0 },
    /* 0x3C */ { BtlFxStart3C, func_800C09D0, func_800C23CC, 0 },
    /* 0x3D */ { BtlFxStart3D, func_800C09D0, func_800C23CC, 0 },
    /* 0x3E */ { BtlFxStart3A, func_800BAE5C, func_800C23CC, 0 },
    /* 0x3F */ { BtlFxStart3F, func_800C09D0, func_800C23CC, 0 },
    /* 0x40 */ { BtlFxStart40, func_800C09D0, func_800C23CC, 0 },
    /* 0x41 */ { BtlFxStart41, func_800C09D0, func_800C23CC, 4 },
    /* 0x42 */ { BtlFxStart42, func_800BB5BC, func_800C0E54, 4 },
    /* 0x43 */ { BtlFxStart43, func_800C09D0, func_800C23CC, 4 },
    /* 0x44 */ { BtlFxStart44, func_800BBD28, func_800C23CC, 4 },
    /* 0x45 */ { BtlFxStart45, func_800BBD28, func_800C23CC, 4 },
    /* 0x46 */ { BtlFxStart46, func_800C09D0, func_800C23CC, 4 },
    /* 0x47 */ { BtlFxStart47, func_800BBD28, func_800C23CC, 4 },
    /* 0x48 */ { BtlFxStart48, func_800BBD28, func_800C23CC, 4 },
    /* 0x49 */ { BtlFxStart49, func_800BBD28, func_800C23CC, 4 },
    /* 0x4A */ { BtlFxStart4A, func_800BBD28, func_800C23CC, 4 },
    /* 0x4B */ { BtlFxStart4B, func_800BBD28, func_800C23CC, 4 },
    /* 0x4C */ { BtlFxStart4C, func_800BBFDC, 0, 0 },
    /* 0x4D */ { BtlFxStart4D, func_800C09D0, func_800C0E54, 4 },
    /* 0x4E */ { BtlFxStart4E, func_800C09D0, func_800C0E54, 0 },
    /* 0x4F */ { BtlFxStart4F, func_800BC39C, 0, 0 },
    /* 0x50 */ { BtlFxStart50, func_800C09D0, func_800C0E54, 1 },
    /* 0x51 */ { BtlFxStart51, func_800C09D0, func_800C0E54, 0 },
    /* 0x52 */ { BtlFxStart52, func_800C09D0, func_800C0E54, 1 },
    /* 0x53 */ { BtlFxStart53, func_800C09D0, func_800C10E0, 4 },
    /* 0x54 */ { BtlFxStart54, func_800C09D0, func_800C10E0, 4 },
    /* 0x55 */ { BtlFxStart55, func_800C09D0, func_800C10E0, 4 },
    /* 0x56 */ { BtlFxStart56, func_800C09D0, func_800C10E0, 0 },
    /* 0x57 */ { BtlFxStart57, func_800C09D0, func_800C10E0, 4 },
    /* 0x58 */ { BtlFxStart58, func_800C09D0, func_800C10E0, 4 },
    /* 0x59 */ { BtlFxStart59, func_800C09D0, func_800C10E0, 4 },
    /* 0x5A */ { BtlFxStart5A, func_800C09D0, func_800C10E0, 4 },
    /* 0x5B */ { BtlFxStart5B, func_800BC99C, 0, 0 },
    /* 0x5C */ { BtlFxStart5C, func_800C09D0, func_800C10E0, 4 },
    /* 0x5D */ { BtlFxStart5D, func_800C09D0, func_800C10E0, 4 },
    /* 0x5E */ { BtlFxStart5E, func_800C09D0, func_800C10E0, 4 },
    /* 0x5F */ { BtlFxStart5F, func_800C09D0, func_800C17E8, 4 },
    /* 0x60 */ { BtlFxStart60, func_800C09D0, func_800C17E8, 4 },
    /* 0x61 */ { BtlFxStart61, func_800C09D0, func_800C17E8, 4 },
    /* 0x62 */ { BtlFxStart62, func_800C09D0, func_800C17E8, 4 },
    /* 0x63 */ { BtlFxStart63, func_800C09D0, func_800C17E8, 4 },
    /* 0x64 */ { BtlFxStart64, func_800C09D0, func_800C17E8, 4 },
    /* 0x65 */ { BtlFxStart65, func_800C09D0, func_800C17E8, 4 },
    /* 0x66 */ { BtlFxStart66, func_800C09D0, func_800C17E8, 4 },
    /* 0x67 */ { BtlFxStart67, func_800C09D0, func_800C17E8, 4 },
    /* 0x68 */ { BtlFxStart68, func_800C09D0, func_800C17E8, 4 },
    /* 0x69 */ { BtlFxStart69, func_800C09D0, func_800C17E8, 4 },
    /* 0x6A */ { BtlFxStart6A, func_800C09D0, func_800C17E8, 4 },
    /* 0x6B */ { BtlFxStart42, func_800BB5BC, 0, 0 },
    /* 0x6C */ { BtlFxStart42, func_800BB5BC, 0, 0 },
    /* 0x6D */ { BtlFxStart6D, func_800BCF6C, 0, 0 },
    /* 0x6E */ { BtlFxStart6E, func_800BD628, 0, 0 },
    /* 0x6F */ { 0, 0, 0, 0 },
    /* 0x70 */ { 0, 0, 0, 0 },
    /* 0x71 */ { 0, 0, 0, 0 },
    /* 0x72 */ { BtlFxStart72, func_800BD3A8, 0, 0 },
    /* 0x73 */ { 0, 0, 0, 0 },
    /* 0x74 */ { BtlFxStart74, func_800C09D0, func_800C10E0, 0 },
    /* 0x75 */ { BtlFxStart75, func_800C09D0, func_800C23CC, 4 },
    /* 0x76 */ { BtlFxStart76, func_800C09D0, func_800C23CC, 4 },
    /* 0x77 */ { BtlFxStart77, func_800C09D0, func_800C23CC, 4 },
    /* 0x78 */ { BtlFxStart78, func_800C09D0, func_800C23CC, 4 },
    /* 0x79 */ { BtlFxStart79, func_800C09D0, func_800C23CC, 4 },
    /* 0x7A */ { BtlFxStart7A, func_800C09D0, func_800C23CC, 4 },
    /* 0x7B */ { BtlFxStart7B, func_800C09D0, func_800C23CC, 4 },
    /* 0x7C */ { BtlFxStart7C, func_800C09D0, func_800C23CC, 4 },
    /* 0x7D */ { BtlFxStart7D, func_800C09D0, func_800C23CC, 4 },
    /* 0x7E */ { BtlFxStart7E, func_800C09D0, func_800C23CC, 4 },
    /* 0x7F */ { BtlFxStart7F, func_800C09D0, func_800C23CC, 4 },
    /* 0x80 */ { BtlFxStart80, func_800C09D0, func_800C23CC, 4 },
    /* 0x81 */ { BtlFxStart81, func_800C09D0, func_800C23CC, 4 },
    /* 0x82 */ { BtlFxStart82, func_800C09D0, func_800C23CC, 4 },
    /* 0x83 */ { BtlFxStart83, func_800C09D0, func_800C23CC, 4 },
    /* 0x84 */ { BtlFxStart84, func_800BDFB4, 0, 0 },
    /* 0x85 */ { BtlFxStart84, func_800BDFB4, func_800C0E54, 0 },
    /* 0x86 */ { BtlFxStart86, func_800BE3C8, 0, 4 },
    /* 0x87 */ { BtlFxStart87, func_800BE670, func_800C0E54, 4 },
    /* 0x88 */ { BtlFxStart88, func_800BE9E8, 0, 1 },
    /* 0x89 */ { BtlFxStart88, func_800BE9E8, 0, 2 },
    /* 0x8A */ { BtlFxStart88, func_800BE9E8, 0, 0 },
    /* 0x8B */ { BtlFxStart88, func_800BE9E8, 0, 0 },
    /* 0x8C */ { BtlFxStart8C, func_800C09D0, func_800C2D54, 0 },
    /* 0x8D */ { BtlFxStart8C, func_800C09D0, func_800C2D54, 0 },
    /* 0x8E */ { BtlFxStart8C, func_800C09D0, func_800C2D54, 0 },
    /* 0x8F */ { BtlFxStart8C, func_800C09D0, func_800C2D54, 0 },
    /* 0x90 */ { BtlFxStart90, func_800C09D0, func_800C0E54, 1 },
    /* 0x91 */ { BtlFxStart91, func_800C09D0, func_800C0E54, 2 },
    /* 0x92 */ { BtlFxStart92, func_800C09D0, func_800C0E54, 2 },
    /* 0x93 */ { BtlFxStart93, func_800C09D0, func_800C0E54, 2 },
    /* 0x94 */ { BtlFxStart94, func_800C09D0, func_800C0E54, 0 },
    /* 0x95 */ { BtlFxStart95, func_800C09D0, func_800C0E54, 4 },
    /* 0x96 */ { BtlFxStart96, func_800C09D0, func_800C0E54, 4 },
    /* 0x97 */ { BtlFxStart97, func_800C09D0, func_800C0E54, 4 },
    /* 0x98 */ { BtlFxStart98, func_800C09D0, func_800C0E54, 4 },
    /* 0x99 */ { BtlFxStart99, func_800C09D0, func_800C0E54, 4 },
    /* 0x9A */ { BtlFxStart9A, func_800C09D0, func_800C0E54, 3 },
    /* 0x9B */ { BtlFxStart9B, func_800C09D0, func_800C0E54, 3 },
    /* 0x9C */ { BtlFxStart9C, func_800C09D0, func_800C0E54, 4 },
    /* 0x9D */ { BtlFxStart9D, func_800C09D0, func_800C0E54, 4 },
    /* 0x9E */ { BtlFxStart9E, func_800C09D0, func_800C0E54, 4 },
    /* 0x9F */ { BtlFxStart9F, func_800C09D0, func_800C0E54, 4 },
    /* 0xA0 */ { BtlFxStartA0, func_800BF0F8, 0, 0 },
    /* 0xA1 */ { BtlFxStartA1, func_800C09D0, func_800C0E54, 0 },
    /* 0xA2 */ { BtlFxStartA2, D_800BF55C, 0, 0 },
    /* 0xA3 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA4 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA5 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA6 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA7 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA8 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xA9 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAA */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAB */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAC */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAD */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAE */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xAF */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB0 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB1 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB2 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB3 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB4 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB5 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB6 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB7 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB8 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xB9 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBA */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBB */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBC */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBD */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBE */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xBF */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC0 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC1 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC2 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC3 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC4 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC5 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC6 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC7 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC8 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xC9 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCA */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCB */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCC */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCD */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCE */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xCF */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD0 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD1 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD2 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD3 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD4 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD5 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD6 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD7 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD8 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xD9 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xDA */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xDB */ { BtlFxStartDB, func_800BDBA8, 0, 0 },
    /* 0xDC */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xDD */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xDE */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xDF */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xE0 */ { BtlFxStartE0, func_800C09D0, func_800C0E54, 0 },
    /* 0xE1 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xE2 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xE3 */ { BtlFxStartNone, 0, 0, 0 },
    /* 0xE4 */ { BtlFxStartE4, func_800C09D0, func_800C0E54, 0 },
    /* 0xE5 */ { BtlFxStartE5, func_800BF7F8, func_800C0E54, 0 },
    /* 0xE6 */ { BtlFxStartE6, func_800C09D0, func_800C0E54, 0 },
    /* 0xE7 */ { 0, 0, 0, 0 },
    /* 0xE8 */ { BtlFxStartE8, func_800BFA04, func_800C0E54, 0 },
    /* 0xE9 */ { BtlFxStartE9, func_800C09D0, func_800C0E54, 0 },
    /* 0xEA */ { BtlFxStartEA, func_800C09D0, func_800C0E54, 0 },
    /* 0xEB */ { BtlFxStartEB, func_800C09D0, func_800C0E54, 0 },
    /* 0xEC */ { BtlFxStartEC, func_800C09D0, func_800C0E54, 0 },
    /* 0xED */ { BtlFxStartED, func_800C09D0, func_800C17E8, 0 },
    /* 0xEE */ { BtlFxStartEE, func_800C09D0, func_800C23CC, 0 },
    /* 0xEF */ { BtlFxStartEF, func_800C09D0, func_800C10E0, 0 },
    /* 0xF0 */ { BtlFxStartF0, func_800C09D0, func_800C0E54, 0 },
    /* 0xF1 */ { BtlFxStartF1, func_800C09D0, func_800C17E8, 0 },
    /* 0xF2 */ { BtlFxStartF2, func_800C09D0, func_800C23CC, 0 },
    /* 0xF3 */ { BtlFxStartF3, func_800C09D0, func_800C17E8, 0 },
    /* 0xF4 */ { BtlFxStartF4, func_800C09D0, func_800C17E8, 0 },
    /* 0xF5 */ { BtlFxStartF5, func_800C09D0, func_800C17E8, 0 },
    /* 0xF6 */ { BtlFxStartF6, func_800C09D0, func_800C17E8, 0 },
};
