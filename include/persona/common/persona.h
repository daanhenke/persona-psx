#ifndef PERSONA_COMMON_PERSONA_H
#define PERSONA_COMMON_PERSONA_H

/* Persona 1 (JP) - the Persona records.
 *
 * They sit in the save-game work area immediately after the five Char records,
 * so the address is the same in every overlay. Char.list[3] holds indices into
 * this array - the Personas a character carries - with 0xFF for empty. */
#include <decomp/types.h>

#define PERSONA_SPELLS 7
#define PERSONA_COUNT  31

#define PERSONA_STATS 5

typedef struct {
    /* 0x00 */ u_long  unk00;
    /* 0x04 */ u_long  unk04;
    /* 0x08 */ u_long  unk08;
    /* 0x0C */ u_long  unk0C;     /* built from the definition record's +0x1C */
    /* 0x10 */ u_short unk10;     /* CharRecalcStats copies this pair into the
                                     carrier's Char+0x3A and +0x3C, or writes
                                     1 into both when no Persona is equipped */
    /* 0x12 */ u_short unk12;
    /* 0x14 */ u_char  pad14[4];
    /* 0x18 */ u_char  key;       /* identifies the record; 0 while unused */
    /* 0x19 */ u_char  unk19[10]; /* straight out of the definition's +0x08 */
    /* 0x23 */ u_char  unk23;
    /* 0x24 */ u_char  level;
    /* 0x25 */ u_char  kind;      /* the battle pairs a demon species with a
                                     Persona through this                    */
    /* 0x26 */ u_char  stat[PERSONA_STATS];
                                  /* the same five the character has, drawn as
                                     bars on the status screen and clamped at
                                     99 there */
    /* 0x2B */ u_char  unk2B;
    /* 0x2C */ u_char  slots;     /* how far into the spell order to read     */
    /* 0x2D */ u_char  spell[PERSONA_SPELLS];
                                  /* learned in order; 0 for a slot the
                                     Persona has not reached yet */
    /* 0x34 */ u_char  raw[PERSONA_SPELLS];
                                  /* the same list as it came off the disc,
                                     which is what the battle packs          */
    /* 0x3B */ u_char  unk3B;
    /* 0x3C */ u_char  pad3C[4];
} Persona;                        /* 0x40 bytes */

#define g_personas ((Persona *)0x801F1DAC)

/* The stock: the Personas nobody carries yet, as ids into g_personas. Fifteen
   slots exist and the first twelve are the rows the stock screen draws, which
   is the only part the battle edits. A free slot holds zero, and neither the
   battle's add nor its remove compacts the array - the field's own pass closes
   the holes later.

   Some units walk it signed and some unsigned, but that is decided by the
   type of the pointer they copy it into, not by the cast here. */
#define g_persona_stock ((u_char *)0x801F297C)

#define STOCK_ROWS 12
#define STOCK_FREE 0

/* What BtlStockRandomise rolls: ids 2 to 101. */
#define STOCK_RANDOM_RANGE 100
#define STOCK_RANDOM_FIRST 2

/* The reference data every Persona is built from, in main's rodata rather than
   the save-game area. The stock list shows an entry this way before it belongs
   to anybody. */
typedef struct {
    /* 0x00 */ int    hp;         /* what a fighter built from this record
                                     starts with; the maximum takes the same
                                     word, so the two are always equal on the
                                     way in                                  */
    /* 0x04 */ int    sp;
    /* 0x08 */ int    exp;        /* what a demon of this Persona is worth:
                                     BtlTalkSceneGift scales the experience it
                                     leaves behind by this                   */
    /* 0x0C */ u_short unk0C;     /* the pair a contact is weighed with, which
                                     lands on Char.unk3A and Char.unk3C      */
    /* 0x0E */ u_short unk0E;
    /* 0x10 */ int    unk10;      /* lands on Char +0x1C                     */
    /* 0x14 */ u_short drop;      /* what a demon of this Persona leaves
                                     behind: the item's id in the low nine
                                     bits and one of five drop rates above
                                     them. BtlRollDefeatDrop weighs the rate
                                     against the luck difference.          */
    /* 0x16 */ u_char pad16[2];
    /* 0x18 */ int    price;      /* and the money, scaled the same way and
                                     then rounded by BtlRoundMoney           */
    /* 0x1C */ u_char name[10];   /* tile bytes, terminated by 0xFF */
    /* 0x26 */ u_char level;
    /* 0x27 */ u_char arcana;     /* 1-based, into a table of six-cell labels */
    /* 0x28 */ u_char stat[PERSONA_STATS];
    /* 0x2D */ u_char unk2D;      /* lands on Char.unk5C                     */
    /* 0x2E */ u_char spell[6];    /* what a fighter built from this record can
                                     cast, kept whole on the actor at +0xAF.
                                     BtlChooseEnemyMove indexes g_spell_data
                                     with each of them.                     */
    /* 0x34 */ u_char pad34[4];
} PersonaData;                    /* 0x38 bytes */

extern PersonaData g_persona_data[];

/* The record a Persona is built from, one per Persona id with entry 0 blank.
   main fills a g_personas slot out of one of these; the battle keeps the byte
   at +0x28 beside every record it copies. */
typedef struct {
    /* 0x00 */ u_char  pad00[4];
    /* 0x04 */ u_short unk04;     /* the pair a contact is weighed with      */
    /* 0x06 */ u_short unk06;
    /* 0x08 */ u_char  unk08[10]; /* kept whole, on the battle's record at
                                     +0x1F and the save game's at +0x19     */
    /* 0x12 */ u_char  unk12;
    /* 0x13 */ u_char  level;
    /* 0x14 */ u_char  kind;      /* pairs a demon species with this Persona */
    /* 0x15 */ u_char  stat[PERSONA_STATS];
    /* 0x1A */ u_char  unk1A;
    /* 0x1B */ u_char  pad1B[1];
    /* 0x1C */ u_long  unk1C;     /* lands on the record's +0x0C             */
    /* 0x20 */ u_char  raw[PERSONA_SPELLS];
                                  /* the spell list as it came off the disc  */
    /* 0x27 */ u_char  pad27[1];
    /* 0x28 */ u_char  unk28;
    /* 0x29 */ u_char  pad29[3];
} PersonaDef;                     /* 0x2C bytes */

extern const PersonaDef g_persona_defs[];

#endif
