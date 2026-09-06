/* Persona 1 (JP) - a party member's side of a negotiation.
 *
 * One 0x26-byte record per party member. The key at +0 is zero in a slot
 * nobody occupies; the two bytes after it are what the Persona lookup is run
 * against, and its answer lands at +4. The rest is what this member can say:
 * four chosen slots and four groups of three lines, all held as 0xFFFF while
 * empty.
 */
#ifndef PERSONA_BTLP_MEMBER_H
#define PERSONA_BTLP_MEMBER_H

#include <types.h>

#define BTL_MEMBER_GROUPS 4
#define BTL_MEMBER_LINES  3

/* An empty slot, in both arrays. */
#define BTL_NO_LINE 0xFFFF

/* Both arrays are reached from the record base in shorts: chosen[g] is three
   shorts in, and the last line of a group nine. */
#define BTL_MEMBER_CHOSEN 3
#define BTL_MEMBER_LAST   9

/* The group loop counts the bytes of the lines array rather than the groups. */
#define BTL_MEMBER_GROUP_BYTES 6
#define BTL_MEMBER_LINE_BYTES  0x18

typedef struct {
    /* 0x00 */ u_char  key;
    /* 0x01 */ u_char  pair[2];
    /* 0x03 */ u_char  pad03;
    /* 0x04 */ u_short answer;
    /* 0x06 */ short   chosen[BTL_MEMBER_GROUPS];
    /* 0x0E */ short   lines[BTL_MEMBER_GROUPS][BTL_MEMBER_LINES];
} BtlMember;                     /* 0x26 bytes */

extern BtlMember g_btl_member[];

#endif
