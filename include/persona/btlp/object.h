/* Persona 1 (JP) - the battle overlay's display objects.
 *
 * Everything the battle draws - a character, its shadow, a spell effect, a
 * floating number - is one 0xD8-byte record out of a single pool. The pool is
 * cut into six groups by g_btl_obj_count, and each group is a doubly linked
 * list threaded through the records themselves, with the last one in
 * g_btl_obj_tail. Drawing order is the list order, which is why moving a record
 * is a list operation rather than a sort key.
 *
 * The attribute word at +0 doubles as the in-use flag: BtlObjAlloc takes the
 * first record in the group whose attribute is clear, and BtlObjFree clears it
 * again.
 *
 * Each group opens with a head record that is never allocated - it carries
 * BTL_OBJ_HEAD and stays in use for the life of the overlay. Allocation starts
 * one past its group's head and stops when it reaches the next one, so the
 * heads are the group boundaries as well as the list anchors.
 *
 * A record can carry another one along with it. Spawning an enemy allocates a
 * second record for its shadow and hangs it off `attached`, and every setter in
 * objset.c walks that link so a change reaches the whole assembly.
 */
#ifndef PERSONA_BTLP_OBJECT_H
#define PERSONA_BTLP_OBJECT_H

#include <types.h>

/* One step of an animation script. The high byte of `flags` is set on every
   step but the last, which is how the end is found. */
typedef struct {
    /* 0x0 */ u_long  value;
    /* 0x4 */ u_short flags;
    /* 0x6 */ u_short pad;
} BtlSeqStep;                          /* 8 bytes */

#define BTL_SEQ_MORE 0xFF00

/* What a record is made from. The two fields are the first two of BtlObj, so
   a template is the head of the object it becomes: the attribute word it
   starts with and the model's table of scripts. */
typedef struct {
    /* 0x0 */ u_long         attr;
    /* 0x4 */ const u_long **scripts;
} BtlObjDef;                           /* 8 bytes */

struct BtlActor;

typedef struct BtlObj {
    /* 0x00 */ u_long         attr;    /* zero when the record is free     */
    /* 0x04 */ const u_long **scripts; /* the model's table of scripts, the
                                          one a spawn was given            */
    /* 0x08 */ long           x;       /* 16.16, and what a shadow follows */
    /* 0x0C */ long           y;
    /* 0x10 */ long           z;
    /* 0x14 */ u_char         pad14[4];
    /* 0x18 */ long           x2;      /* a second copy of x and y, given   */
    /* 0x1C */ long           y2;      /* the same value on an outright put */
    /* 0x20 */ u_char         pad20[0x1C];
    /* 0x3C */ long           shift;   /* 16.16; what the tick displaces  */
    /* 0x40 */ u_char         pad40[4];
    /* 0x44 */ struct BtlObj *prev;
    /* 0x48 */ struct BtlObj *next;
    /* 0x4C */ struct BtlObj *attached; /* carried along by every setter    */
    /* 0x50 */ struct BtlObj *shadow;   /* kept on this one's position      */
    /* 0x54 */ u_char         pad54[0x10];
    /* 0x64 */ BtlSeqStep    *script;  /* animation script                 */
    /* 0x68 */ u_long         last;    /* first word of the script's last step */
    /* 0x6C */ struct BtlActor *actor; /* whose object this is; the spawn
                                          writes it and BtlObjStatusTint
                                          reads the ailment through it   */
    /* 0x70 */ short          unk70;   /* a shadow is given -0x320 here and
                                          zero in the two below it        */
    /* 0x72 */ short          unk72;
    /* 0x74 */ short          unk74;
    /* 0x76 */ u_char         pad76[2];
    /* 0x78 */ long           scale_x;  /* unity is 0x100 in both of these  */
    /* 0x7C */ long           scale_y;
    /* 0x80 */ long           scale_z;  /* unity is 0x1000 in this one      */
    /* 0x84 */ u_char         pad84[0x2C];
    /* 0xB0 */ u_char         col2;    /* the grid column, doubled          */
    /* 0xB1 */ char           row;     /* the grid row                      */
    /* 0xB2 */ u_short        kind;    /* BTL_OBJ_HEAD marks a list head    */
    /* 0xB4 */ u_char         padB4[2];
    /* 0xB6 */ short          step;    /* how far into the script it is     */
    /* 0xB8 */ u_char         padB8[2];
    /* 0xBA */ short          age;     /* frames since the record was taken */
    /* 0xBC */ u_char         padBC[2];
    /* 0xBE */ short          timer;   /* counts down a frame at a time     */
    /* 0xC0 */ short          rgb[3];  /* the colour actually drawn         */
    /* 0xC6 */ short          rgb_to[3]; /* the colour it is walking toward */
    /* 0xCC */ u_char         fade;    /* how far it walks in one frame     */
    /* 0xCD */ u_char         padCD[2];
    /* 0xCF */ u_char         group;   /* which list the record belongs to  */
    /* 0xD0 */ u_char         padD0[1];
    /* 0xD1 */ u_char         motion;  /* what it is doing, 0 idle          */
    /* 0xD2 */ u_char         padD2[2];
    /* 0xD4 */ u_char         phase;   /* how far into that motion          */
    /* 0xD5 */ u_char         padD5[3];
} BtlObj;                              /* 0xD8 bytes */

#define BTL_OBJ_GROUPS 6
#define BTL_OBJ_INUSE  0x80000000   /* attribute bit that marks a record taken */

/* The kind field's top bit is not a kind at all: it marks the record that
   stands at the head of a group's list. BtlObjAlloc starts one record past it
   and stops at the next one, so a head doubles as the previous group's end. */
#define BTL_OBJ_HEAD   0x8000

/* Two more attribute bits, always changed together. BtlObjSetScript sets the
   first and clears the second when it arms a script, and BtlObjAlloc turns the
   first on for a record whose template has the second clear - so the pair says
   whether the object is running a script or standing still. */
/* Set on an actor's object exactly when its shadow is hidden, and cleared
   again with it - BtlObjStatusTint is the only thing that touches either. */
#define BTL_OBJ_NO_SHADOW 0x1

#define BTL_OBJ_ANIMATING 0x10000000
#define BTL_OBJ_STATIC    0x20000000

#endif
