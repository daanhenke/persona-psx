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

/* A Persona's bond with each member, two bits a member by Char.key less one.
   Under PERSONA_BOND_WILLING it will not be cast for that member, nor reached
   for by a maddened one; PERSONA_BOND_FULL is as far as it goes. */
#define PERSONA_BOND_BITS    2
#define PERSONA_BOND_MASK    3
#define PERSONA_BOND_WILLING 2
#define PERSONA_BOND_FULL    3

typedef struct {
    /* 0x00 */ u_long  unk00;     /* PersonaDef.unk00, copied in             */
    /* 0x04 */ u_long  unk04;
    /* 0x08 */ u_long  unk08;
    /* 0x0C */ u_long  bond;      /* PersonaDef.bond, copied in as the record
                                     is built                                */
    /* 0x10 */ u_short unk10;     /* CharRecalcStats copies this pair into the
                                     carrier's Char+0x3A and +0x3C, or writes
                                     1 into both when no Persona is equipped */
    /* 0x12 */ u_short unk12;
    /* 0x14 */ u_short unk14;     /* both zeroed as the record is built     */
    /* 0x16 */ u_short unk16;
    /* 0x18 */ u_char  key;       /* identifies the record; 0 while unused */
    /* 0x19 */ u_char  name[10];  /* straight out of the definition's name */
    /* 0x23 */ u_char  sp_cost;   /* PersonaDef.sp_cost, copied the same way */
    /* 0x24 */ u_char  level;
    /* 0x25 */ u_char  kind;      /* the battle pairs a demon species with a
                                     Persona through this                    */
    /* 0x26 */ u_char  stat[PERSONA_STATS];
                                  /* the same five the character has, drawn as
                                     bars on the status screen and clamped at
                                     99 there */
    /* 0x2B */ u_char  resist;
    /* 0x2C */ u_char  slots;     /* how far into the spell order to read     */
    /* 0x2D */ u_char  spell[PERSONA_SPELLS];
                                  /* learned in order; 0 for a slot the
                                     Persona has not reached yet */
    /* 0x34 */ u_char  raw[PERSONA_SPELLS];
                                  /* the same list as it came off the disc,
                                     which is what the battle packs          */
    /* 0x3B */ u_char  unk3B;
    /* 0x3C */ u_char  unk3C;     /* both zeroed as the record is built     */
    /* 0x3D */ u_char  unk3D;
    /* 0x3E */ u_char  pad3E;
    /* 0x3F */ u_char  owner;     /* the key of the character carrying it; the
                                     scripts give and take Personas by it     */
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

/* Which demons the party has analysed, a bit per Persona key. In the same
   save-game work area and reached by hardcoded address like the rest of it. */
#define g_analysed_demons ((u_long *)0x801F2A48)

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
    /* 0x2D */ u_char resist;     /* lands on Char.resist                    */
    /* 0x2E */ u_char spell[6];    /* what a fighter built from this record can
                                     cast, kept whole on the actor at +0xAF.
                                     BtlChooseEnemyMove indexes g_spell_data
                                     with each of them.                     */
    /* 0x34 */ u_char pad34[1];
    /* 0x35 */ u_char attack;     /* the blow a demon of this Persona strikes
                                     with when it is not casting: which strike
                                     art goes up and which voice bank speaks
                                     it, and two short of its element       */
    /* 0x36 */ u_char pad36[2];
} PersonaData;                    /* 0x38 bytes */

extern PersonaData g_persona_data[];

/* The record a Persona is built from, one per Persona id with entry 0 blank.
   main fills a g_personas slot out of one of these; the battle keeps the byte
   at +0x28 beside every record it copies. */
typedef struct {
    /* 0x00 */ u_long  unk00;
    /* 0x04 */ u_short unk04;     /* the pair a contact is weighed with      */
    /* 0x06 */ u_short unk06;
    /* 0x08 */ u_char  name[10];  /* tile bytes; kept whole, on the battle's record at
                                     +0x1F and the save game's at +0x19     */
    /* 0x12 */ u_char  sp_cost;   /* SP a cast through this Persona takes    */
    /* 0x13 */ u_char  level;
    /* 0x14 */ u_char  kind;      /* pairs a demon species with this Persona */
    /* 0x15 */ u_char  stat[PERSONA_STATS];
    /* 0x1A */ u_char  resist;
    /* 0x1B */ u_char  pad1B[1];
    /* 0x1C */ u_long  bond;      /* how far the Persona answers each member,
                                     PERSONA_BOND_BITS a member. It lands on
                                     the record's +0x0C and on BtlStats.bond */
    /* 0x20 */ u_char  raw[PERSONA_SPELLS];
                                  /* the spell list as it came off the disc  */
    /* 0x27 */ u_char  pad27[1];
    /* 0x28 */ u_char  unk28;
    /* 0x29 */ u_char  pad29[3];
} PersonaDef;                     /* 0x2C bytes */

extern const PersonaDef g_persona_defs[];

/* The six-cell label each arcana is shown by, by the 1-based
   PersonaData.arcana, flat. Each overlay that draws one carries its copy. */
#define ARCANA_LABELS  26
#define ARCANA_LABEL_W 6
extern u_char g_arcana_labels[ARCANA_LABELS * ARCANA_LABEL_W];

/* A Persona's name into a menu row, from glyph bank `base`; the row is
   cleared first, so id 0 leaves it blank. */
extern void DrawPersonaName(short persona, short *dst, u_short base);

#endif
