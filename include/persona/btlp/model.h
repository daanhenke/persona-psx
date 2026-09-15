/* Persona 1 (JP) - what the battle knows about a character's model.
 *
 * One 48-byte record per character, indexed by the Char key, read off the disc
 * into the work area. Most of it is still unread; what is known are the script
 * indices, which are offsets into the object's own table of scripts rather
 * than scripts themselves.
 *
 * The four reactions are picked by BtlTalkReact from the kind it is handed,
 * and they do not sit in kind order: kind 0 takes the second of the run, 1 the
 * third, 2 the fourth and 3 the first. Anything else takes `talk`.
 */
#ifndef PERSONA_BTLP_MODEL_H
#define PERSONA_BTLP_MODEL_H

/* Ten script indices per shape, four shapes per member. */
extern u_char g_btl_member_scripts[];

/* A member's run of g_btl_member_scripts starts at its key times
   MEMBER_SCRIPT_MODEL, and each shape its script_pick chooses lies
   MEMBER_SCRIPT_PICK further along. */
#define MEMBER_SCRIPT_MODEL 0x28
#define MEMBER_SCRIPT_PICK  10

/* The same layout again for the scripts a member reacts to a blow and to a
   demon's words in. */
extern u_char g_btl_talk_motion[];

#include <decomp/types.h>

typedef struct {
    /* 0x00 */ u_char spawn;      /* armed when the actor is first put out */
    /* 0x01 */ u_char talk;       /* the reaction every other kind takes   */
    /* 0x02 */ u_char stand;      /* a script BtlTalkSceneGift puts on the
                                     demon after its talk script         */
    /* 0x03 */ u_char death;      /* the fall an enemy is put on           */
    /* 0x04 */ u_char hit;        /* the flinch an enemy is put on         */
    /* 0x05 */ u_char hurt;       /* the reaction a blow puts an enemy in  */
    /* 0x06 */ u_char react3;
    /* 0x07 */ u_char react0;
    /* 0x08 */ u_char react1;
    /* 0x09 */ u_char react2;
    /* 0x0A */ u_char extra;      /* the script a third piece is put up on,
                                     or nought for a species without one   */
    /* 0x0B */ signed char depth; /* how far off the ground the body stands,
                                     in whole units                         */
    /* 0x0C */ u_char pad0C[2];
    /* 0x0E */ signed char mark_x; /* where the ailment marker hangs off the
                                      body, in whole units                 */
    /* 0x0F */ signed char mark_y;
    /* 0x10 */ signed char number_z; /* how high over the enemy the amount a
                                     blow took is put, before the shift    */
    /* 0x11 */ signed char gfx_size; /* which of the enemy slot shapes the
                                        species' artwork needs; what
                                        BtlLoadEnemyGfx switches on        */
    /* 0x12 */ u_char attack[6];  /* how each of the fighter's six spells is
                                     played out, by the slot it is cast
                                     from. BtlEnemyMotion06 takes the attack
                                     routine from it, and BtlEnemyMotion02
                                     reads 1 and 2 as a lunge and 5 as a
                                     flight                                */
    /* 0x18 */ u_char strike[6];  /* the script each is struck in           */
    /* 0x1E */ u_char after[6];   /* and the one the body is left in        */
    /* 0x24 */ u_char pad24[0xC];
} BtlModel;                       /* 48 bytes */

extern BtlModel g_btl_models[];

/* The same records seen four bytes earlier, which is how the spawn and the
   enemy motions reach them: the attribute word an enemy of the species
   starts with, and then the model's bytes under the names BtlModel gives
   them. The stride is the same, so each entry's tail is the start of the
   next species' model. */
typedef struct {
    /* 0x00 */ u_long attr;
    /* 0x04 */ u_char spawn;
    /* 0x05 */ u_char talk;
    /* 0x06 */ u_char stand;
    /* 0x07 */ u_char pad07[0xF];
    /* 0x16 */ u_char attack[6];
    /* 0x1C */ u_char strike[6];
    /* 0x22 */ u_char after[6];
    /* 0x28 */ u_char pad28[8];
} BtlSpecies;                     /* 48 bytes */

extern const BtlSpecies g_btl_species[];

/* The record of the enemy whose turn it is, left by BtlEnemyMotion06 for
   the attack routine it hands over to. */
extern const BtlSpecies *g_btl_species_now;

#endif
