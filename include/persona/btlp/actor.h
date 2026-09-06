/* Persona 1 (JP) - everyone taking part in a battle.
 *
 * One table, 0xEC bytes a record: the party in slots 0 to 4 and the enemies in
 * the nine that follow, which is why the target picker walks nine records from
 * slot 5 and the command cursor wraps at five.
 *
 * `status` is an ailment code in the same space as Char.status - the battle
 * writes STATUS_BIND into it and the HUD turns it into an icon cell by adding
 * the icon strip's base - but it runs on past the seventeen ailments the status
 * screen can name. Those extra codes only exist inside a fight.
 */
#ifndef PERSONA_BTLP_ACTOR_H
#define PERSONA_BTLP_ACTOR_H

#include <types.h>
#include <persona/btlp/object.h>

typedef struct {
    /* 0x00 */ u_char  pad00[2];
    /* 0x02 */ u_char  id;        /* the character, and zero in an empty slot */
    /* 0x03 */ u_char  name[10];  /* packed glyph bytes, as Char keeps a name */
    /* 0x0D */ signed char status;    /* ailment code, extended past the table */
    /* 0x0E */ u_char  pad0E[0x16];
    /* 0x24 */ BtlObj *obj;       /* what is drawn for this actor         */
    /* 0x28 */ u_long  flags;
    /* 0x2C */ u_char  pad2C[0xC0];
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
