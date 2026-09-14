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
    /* 0x02 */ u_char pad02;
    /* 0x03 */ u_char death;      /* the fall an enemy is put on           */
    /* 0x04 */ u_char hit;        /* the flinch an enemy is put on         */
    /* 0x05 */ u_char hurt;       /* the reaction a blow puts an enemy in  */
    /* 0x06 */ u_char react3;
    /* 0x07 */ u_char react0;
    /* 0x08 */ u_char react1;
    /* 0x09 */ u_char react2;
    /* 0x0A */ u_char pad0A[6];
    /* 0x10 */ signed char number_z; /* how high over the enemy the amount a
                                     blow took is put, before the shift    */
    /* 0x11 */ u_char pad11[0x1F];
} BtlModel;                       /* 48 bytes */

extern BtlModel g_btl_models[];

#endif
