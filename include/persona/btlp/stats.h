/* Persona 1 (JP) - the battle's fighter records.
 *
 * One 0x48-byte record describes anything with stats in a fight: the party's
 * Personas, copied in from the save game when the overlay starts, and the
 * enemies read off the disc. BtlEnemyDeriveStats finishes either kind, which
 * is what says the two are the same shape.
 *
 * The copy from the save game is not a straight one: the save record's first
 * 0x14 bytes line up, ten bytes of the battle's own follow, and everything
 * from the save record's key at +0x18 lands ten bytes further along here. So
 * Persona.key is `key` below, Persona.stat is `stat`, and Persona.spell is
 * `spell`.
 *
 * Only the fields the code named so far are spelt out.
 */
#ifndef PERSONA_BTLP_STATS_H
#define PERSONA_BTLP_STATS_H

#include <decomp/types.h>
#include <persona/common/char.h>

/* Spell slots a record has, and what an empty one holds. */
#define BTL_STATS_SPELLS 7
#define BTL_SLOT_NONE    0xFF

typedef struct {
    /* 0x00 */ u_long  unk00;    /* the save record's first four words, which
                                    line up one for one                     */
    /* 0x04 */ u_long  unk04;
    /* 0x08 */ u_long  unk08;
    /* 0x0C */ u_long  unk0C;
    /* 0x10 */ u_short unk10;
    /* 0x12 */ u_short unk12;
    /* 0x14 */ u_short unk14;    /* BtlApplyPersona copies these two onto the
                                    actor beside unk10 and unk12, and nothing
                                    has read them back yet */
    /* 0x16 */ u_short unk16;
    /* 0x18 */ u_short attack;   /* the three BtlEnemyDeriveStats works out */
    /* 0x1A */ u_short accuracy;
    /* 0x1C */ u_short guard;
    /* 0x1E */ u_char  key;      /* which Persona this is, where the save
                                    record keeps it at +0x18. An offer's
                                    wanted list is matched against it and the
                                    actor's object takes its graphics by it */
    /* 0x1F */ u_char  unk1F[10];
    /* 0x29 */ u_char  unk29;
    /* 0x2A */ u_char  level;
    /* 0x2B */ u_char  kind;     /* the save record's +0x25; a demon species
                                    is paired with one of these through
                                    g_btl_kin_persona                       */
    /* 0x2C */ u_char  stat[5];
    /* 0x31 */ u_char  unk31;
    /* 0x32 */ u_char  slots;                    /* entries of the order to walk */
    /* 0x33 */ u_char  spell[BTL_STATS_SPELLS];  /* the packed list             */
    /* 0x3A */ u_char  raw[BTL_STATS_SPELLS];    /* as it came off the disc     */
    /* 0x41 */ u_char  unk41;
    /* 0x42 */ u_char  pad42[1];
    /* 0x43 */ u_char  unk43;    /* cleared when a Persona is copied in      */
    /* 0x44 */ u_char  unk44;    /* g_persona_defs[key] +0x28, kept here     */
    /* 0x45 */ u_char  pad45[3];
} BtlStats;                      /* 0x48 bytes */

extern BtlStats g_btl_personas[];

#endif
