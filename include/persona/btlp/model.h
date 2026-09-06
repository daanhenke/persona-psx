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

#include <types.h>

typedef struct {
    /* 0x00 */ u_char spawn;      /* armed when the actor is first put out */
    /* 0x01 */ u_char talk;       /* the reaction every other kind takes   */
    /* 0x02 */ u_char pad02[4];
    /* 0x06 */ u_char react3;
    /* 0x07 */ u_char react0;
    /* 0x08 */ u_char react1;
    /* 0x09 */ u_char react2;
    /* 0x0A */ u_char pad0A[0x26];
} BtlModel;                       /* 48 bytes */

extern BtlModel g_btl_models[];

#endif
