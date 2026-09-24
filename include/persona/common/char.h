#ifndef PERSONA_COMMON_CHAR_H
#define PERSONA_COMMON_CHAR_H

/* Persona 1 (JP) - the playable characters.
 *
 * Five records in the save-game work area, so the address is the same in every
 * overlay. g_party holds a record index per party slot. */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ int     hp;      /* the status HUD draws the danger colour once
                                 hp drops to hp_max / 4, which is what tells
                                 the pair apart */
    /* 0x04 */ int     hp_max;
    /* 0x08 */ int     sp;
    /* 0x0C */ int     sp_max;  /* both maxima cap at 999 */
    /* 0x10 */ int     unk10;     /* CharSetLevelExp puts the experience curve
                                     summed below the character's level here,
                                     and the casino adds however much it has
                                     grown since it last looked into two
                                     counters of its own. That is consistent
                                     with experience but not enough to say so. */
    /* 0x14 */ int     unk14;     /* the negotiation's parting gift stops
                                     handing out experience once this reaches
                                     9,999,999, which is a cap of the same
                                     shape as the one it puts on money      */
    /* 0x18 */ int     unk18;     /* the curve's own entry for that level, and
                                     zero once the level is 99                */
    /* 0x1C */ int     unk1C;     /* the debug editor draws it as seven digits
                                     beside unk10 and unk18; an enemy's record
                                     takes its definition's +0x10 here      */
    /* 0x20 */ u_short equip[7];  /* inventory entries; 0 for an empty slot.
                                     [0] is the weapon, [1] the gun and [2]
                                     its ammunition - the gun's two numbers
                                     are zero unless both are filled - and
                                     [3]..[6] the four armour slots.       */
    /* 0x2E */ u_short melee_atk; /* the six values CharRecalcStats derives
                                     from the equipment and the stats; the
                                     status screen prints them as three
                                     digits each                          */
    /* 0x30 */ u_short melee_hit;
    /* 0x32 */ u_short gun_atk;
    /* 0x34 */ u_short gun_hit;
    /* 0x36 */ u_short defence;
    /* 0x38 */ u_short evade;
    /* 0x3A */ u_short unk3A;     /* copied out of the equipped Persona's
                                     +0x10 and +0x12, or 1 when no Persona
                                     is equipped. ItemUse adds unk3A to
                                     rand() % 16 for a heal and 0x800AFEE4
                                     takes a fifth of it, which is not
                                     enough to name the pair.             */
    /* 0x3C */ u_short unk3C;
    /* 0x3E */ u_char  key;       /* identifies the record; 0 while unused  */
    /* 0x3F */ u_char  name[10];  /* packed glyph bytes; the status HUD draws
                                     eight of them, 0xFF ending the row   */
    /* 0x49 */ u_char  status;    /* ailment code, 0 for none. Event scripts
                                     set and clear it; recovery items ask
                                     CharHasStatus for the one they cure. */
    /* 0x4A */ u_char  ail_level; /* how far the ailment at `status` has been
                                     driven, 0 to 2. Landing the same ailment
                                     again raises it and caps it there, and a
                                     different one puts it back to zero. The
                                     battle reads it three ways: which mask of
                                     ailments still lets the fighter act, how
                                     many pips its marker shows, and which
                                     ailments a fresh one may land over. */
    /* 0x4B */ u_char  level;     /* a fifth of it goes into melee_atk and
                                     defence; the script interpreter tests
                                     it against an opcode operand         */
    /* 0x4C */ u_char  stat[5];   /* indexed by the STAT_ codes below */
    /* 0x51 */ u_char  stat_base[5];
                                  /* The status screen draws stat_base as the
                                     bar and highlights stat - stat_base on top
                                     of it, and clamps both at 99. */
    /* 0x56 */ u_char  unk56;     /* the debug editor writes the level into it
                                     along with `level` and draws it beside
                                     it                                      */
    /* 0x57 */ u_char  entry;     /* which list slot is active, 0xFF for none;
                                     CharRecalcStats reads list[entry] */
    /* 0x58 */ u_char  list[3];   /* 0xFF marks an empty entry              */
    /* 0x5B */ u_char  pad5B[1];
    /* 0x5C */ u_char  resist;    /* the equipped Persona's +0x31, put here by
                                     the battle's BtlApplyPersona          */
    /* 0x5D */ u_char  unk5D;     /* the battle clears its low nibble when it
                                     holds 3 and the character has no gun or
                                     no ammunition for one                  */
    /* 0x5E */ u_char  blocked; /* while set the entry list reads as empty   */
    /* 0x5F */ u_char  pad5F[1];
} Char;                           /* 0x60 bytes */

/* The five stats, in the order every record keeps them. Char.stat, BtlActor's
   own copy and BtlStats.stat are all this same order, so the codes are shared
   rather than repeated beside each of them. */
#define STAT_STRENGTH  0
#define STAT_VITALITY  1
#define STAT_DEXTERITY 2
#define STAT_AGILITY   3
#define STAT_LUCK      4

#define g_chars ((Char *)0x801F1BCC)

/* A character's starting record, in the main image; CharInit copies one
   into a Char. */
typedef struct {
    /* 0x00 */ int     hp;
    /* 0x04 */ int     sp;
    /* 0x08 */ u_short equip[7];
    /* 0x16 */ u_char  name[10];
    /* 0x20 */ u_char  stat[5];
    /* 0x25 */ u_char  pad25;
    /* 0x26 */ u_char  resist;
    /* 0x27 */ u_char  pad27;
} CharTemplate;                   /* 0x28 bytes */

extern CharTemplate g_char_templates[];

#define CHAR_COUNT  5
#define CHAR_EQUIP  7
#define CHAR_LIST_N 3
#define CHAR_STATS  5

/* Glyph cells a name is kept in. */
#define CHAR_NAME_CELLS 10

/* What `entry` holds when no list slot is active, and what an empty list slot
   holds. */
#define CHAR_NO_ENTRY 0xFF

/* The furthest Char.ail_level goes. */
#define CHAR_AIL_LEVEL_MAX 2

/* What each level adds. g_char_hp_growth has a row of fifty per character
   from key 2 on; g_char_stat_growth has five such rows per key, one for
   each stat. A level reads its column halved, so two levels share one. */
extern u_char g_char_hp_growth[];
extern u_char g_char_stat_growth[];

/* The experience each level ends at, a word a level. BtlLevelUpParty reads it
   from level 0 and BtlDrainLevel from level 1, which is the same table a word
   in. */
extern int g_level_exp[];
extern int g_level_exp_1[];

/* A stat or number that has grown, held between 1 and `max`: one that has
   wrapped round to nought goes back to 1. */
#define CHAR_GROW_CLAMP(v, max) ((v) != 0 ? ((v) < (max) + 1 ? (v) : (max)) : 1)

/* The experience still owed to reach the level after `level`, given `have`
   (explevel.c); the middle argument is not read. */
int  ExpToLevel(u_char level, int unused, int have);

/* A member's five stats as bars on the status screen (statbars.c). */
void DrawCharStatBars(Char *rec);

#endif
