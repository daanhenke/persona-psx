/* Persona 1 (JP) - everyone taking part in a battle.
 *
 * One table, 0xEC bytes a record: the party in slots 0 to 4 and the enemies in
 * the nine that follow, which is why the target picker walks nine records from
 * slot 5 and the command cursor wraps at five.
 *
 * Each record opens with a whole Char, so the battle reads a fighter's ailment,
 * name and key straight out of it and the field's own code understands the same
 * bytes. The key doubles as "this slot is in use" - it is zero in a record
 * nobody occupies - and the battle indexes the contact labels with it and loads
 * that character's graphics by it.
 *
 * Char.status runs on past the seventeen ailments the status screen can name;
 * the codes below are battle-only and never leave the fight.
 */
#ifndef PERSONA_BTLP_ACTOR_H
#define PERSONA_BTLP_ACTOR_H

#include <types.h>
#include <persona/common/char.h>
#include <persona/btlp/object.h>

typedef struct {
    /* 0x00 */ Char    c;
    /* 0x60 */ BtlObj *obj;       /* what is drawn for this actor */
    /* 0x64 */ u_long  flags;
    /* 0x68 */ u_char  pad68[0x4D];
    /* 0xB5 */ u_char  species;    /* which demon this is; the negotiation
                                      matches its tables against it       */
    /* 0xB6 */ u_char  padB6[0xE];
    /* 0xC4 */ u_char  script_pick; /* chooses between two of the model's
                                       scripts when the actor is set going */
    /* 0xC5 */ u_char  padC5[0x27];
} BtlActor;                       /* 0xEC bytes */

#define BTL_PARTY   5     /* slots 0..4 */
#define BTL_ENEMIES 9     /* slots 5..13 */

/* Two codes past the end of the ailment table, so the status screen has no
   name for either and neither survives the battle. An actor carrying the first
   is out of the fight - not a target, no HUD row, skipped by the cursor. The
   second only stops the cursor: the actor is alive and still a target, it just
   cannot be given an order. */
#define BTL_STATUS_DOWN    0x11
#define BTL_STATUS_NOINPUT 0x13

/* An actor flag with the same effect as BTL_STATUS_DOWN everywhere it is
   tested; the two are always checked together. */
#define BTL_ACTOR_OUT 0x4000

extern BtlActor g_btl_actors[];

extern int BtlActorIsDown(int slot);

#endif
