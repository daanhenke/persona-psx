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

#include <decomp/types.h>

/* One step of an animation script. The high byte of `flags` is set on every
   step but the last, which is how the end is found. */
typedef struct {
    /* 0x0 */ u_long  value;
    /* 0x4 */ u_short flags;
    /* 0x6 */ signed char arg0;   /* what the opcode acts on: a sound bank and
                                     number, an offset pair, or the shift a
                                     plain step leaves behind */
    /* 0x7 */ signed char arg1;
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
    /* 0x20 */ long           z2;      /* and the third of that copy       */
    /* 0x24 */ u_char         pad24[4];
    /* 0x28 */ long           step_x;  /* what a motion carries the object  */
    /* 0x2C */ long           step_y;  /* along by each frame, in the same  */
    /* 0x30 */ long           step_z;  /* 16.16 as the position. BtlObjAlloc
                                          clears all three; only the first
                                          two are known to be read.         */
    /* 0x34 */ u_char         pad34[4];
    /* 0x38 */ long           shift_x;  /* 16.16, the pair a script step's
                                           last two signed bytes set      */
    /* 0x3C */ long           shift;   /* 16.16; what the tick displaces  */
    /* 0x40 */ long           scale_to; /* where a shrink settles: the two
                                           motion-4 phases take a third off
                                           scale_x and then scale_y each frame
                                           and stop here                    */
    /* 0x44 */ struct BtlObj *prev;
    /* 0x48 */ struct BtlObj *next;
    /* 0x4C */ struct BtlObj *attached; /* carried along by every setter    */
    /* 0x50 */ struct BtlObj *shadow;   /* kept on this one's position      */
    /* 0x54 */ long           unk54;   /* both cleared as a record is taken */
    /* 0x58 */ long           unk58;
    /* 0x5C */ struct BtlObj *mark;    /* the ailment marker floating on
                                          this one: the frame tick keeps it
                                          and its attached piece on this
                                          record's x, y and z            */
    /* 0x60 */ const u_long  *unk60;  /* BtlPickShowPage is the only thing
                                          that writes it, with one of the
                                          page's two script pointers      */
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
    /* 0xB4 */ u_short        children; /* a kind-3 parent counts one on here
                                           each time its script spawns one  */
    /* 0xB6 */ short          step;    /* how far into the script it is     */
    /* 0xB8 */ short          unkB8;   /* cleared with age beside it        */
    /* 0xBA */ short          age;     /* frames since the record was taken */
    /* 0xBC */ short          unkBC;   /* cleared with timer beside it      */
    /* 0xBE */ short          timer;   /* counts down a frame at a time     */
    /* 0xC0 */ short          rgb[3];  /* the colour actually drawn         */
    /* 0xC6 */ short          rgb_to[3]; /* the colour it is walking toward */
    /* 0xCC */ u_char         fade;    /* how far it walks in one frame     */
    /* 0xCD */ u_char         unkCD;   /* BtlObjAlloc fills it from its
                                          seventh argument - 0x18 for an
                                          actor, 0x1F for a shadow. The only
                                          thing that reads it is
                                          BtlTalkersLeave, which picks the
                                          group's goodbye bank with
                                          (unkCD >> 1) - 5              */
    /* 0xCE */ u_char         unkCE;   /* BtlObjAlloc fills it from its last
                                          argument; an ailment marker gets the
                                          ailment code plus 0x40           */
    /* 0xCF */ u_char         group;   /* which list the record belongs to  */
    /* 0xD0 */ u_char         draw;    /* which of g_btl_obj_draw's eight
                                          handlers draws it. BtlDrawObjects
                                          rewrites 4 and 5 into each other
                                          every frame, since those two are the
                                          flat and transformed halves of one
                                          drawing                          */
    /* 0xD1 */ u_char         motion;  /* what it is doing, 0 idle          */
    /* 0xD2 */ u_char         mark_num; /* which of a set this record stands
                                          for, and what BtlObjSetMarkNum
                                          writes: the party slot for a
                                          member's object and every piece of
                                          that member's marker, the mark
                                          index for an enemy's, and how far
                                          down the trail for one of the six
                                          records a summoned Persona is made
                                          of. A member's is under
                                          BTL_MEMBERS and an enemy's is not,
                                          which is how BtlApplyPersona tells
                                          the two apart                  */
    /* 0xD3 */ u_char         unkD3;   /* the acting fighter takes a copy of
                                          this as its turn is set up      */
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

/* Taken out of the drawing pass without being freed. Set on an object, its
   shadow, its ailment marker and the marker's own attached piece together. */
#define BTL_OBJ_HIDDEN    0x40000000

/* The pair that says an object is still running its script: the animating bit
   set and the one above it clear. Tested together, never on their own. */
#define BTL_OBJ_BUSY_MASK 0x18000000
#define BTL_OBJ_BUSY      0x10000000

/* One cell of an object's artwork: where it sits relative to the object, which
   corner of the texture page it comes from, and how big it is. The untextured
   kinds read only the corner and the size. */
typedef struct {
    /* 0x0 */ short  x;
    /* 0x2 */ short  y;
    /* 0x4 */ u_char u;
    /* 0x5 */ u_char v;
    /* 0x6 */ u_char w;
    /* 0x7 */ u_char h;
} BtlGfxCell;                          /* 8 bytes */

/* What BtlObj.last points at while a script step is showing artwork: how many
   cells the frame is made of, and where they are. Every one of the eight
   g_btl_obj_draw handlers walks one of these. */
typedef struct {
    /* 0x0 */ u_int            count;
    /* 0x4 */ const BtlGfxCell *cells;
} BtlGfxList;                          /* 8 bytes */

/* What the two text kinds find there instead: a line of character codes and
   where to put it. The run ends either after `count` codes or at the first
   0xFF, whichever comes first. Each code picks a glyph out of a font 31 cells
   wide, so its column is code % 31 and its row code / 31; the cells are 8
   across and `h` down, and `v` is the row the font starts at. */
typedef struct {
    /* 0x0 */ short         x;
    /* 0x2 */ short         y;
    /* 0x4 */ const u_char *text;
    /* 0x8 */ u_char        count;
    /* 0x9 */ u_char        clut;
    /* 0xA */ signed char   h;
    /* 0xB */ u_char        v;
} BtlGfxText;                          /* 12 bytes */

#define BTL_FONT_COLS 31
#define BTL_FONT_W    8
#define BTL_FONT_H    12
#define BTL_TEXT_END  0xFF

/* The pool, and the six lists cut out of it. */
extern BtlObj  *g_btl_obj_pool;
extern BtlObj  *g_btl_obj_tail[];
extern u_short  g_btl_obj_first[];
extern u_short  g_btl_obj_count[];
extern BtlObj  *g_btl_obj_prev;    /* left by BtlObjLast, one short of the end */

/* The eight drawing handlers and the tick, indexed by BtlObj.draw. */
extern void (*g_btl_obj_draw[])(BtlObj *o);
extern void (*g_btl_obj_tick[])(BtlObj *obj);

/* List edits. */
extern int     BtlObjFree(BtlObj *obj);
extern int     BtlObjMoveBefore(BtlObj *at, BtlObj *obj);
extern BtlObj *BtlObjLast(BtlObj *obj);
extern BtlObj *BtlObjClone(BtlObj *obj);

/* Whether every record in the attached chain is on that motion. A chain that
   is not there at all counts as agreeing, which is what lets a caller ask
   about a marker it may never have spawned. */
extern int BtlObjChainAtMotion(BtlObj *obj, u_char motion);

/* Takes the first free record of `group`, links it after `after`, and gives it
   the template's attribute word and script table. The three numbers after the
   position land in BtlObj.draw, unkCD and unkCE. */
extern BtlObj *BtlObjAlloc(const BtlObjDef *defs, int group, BtlObj *after,
                           int draw, int index, const long *pos, int unkCD,
                           int unkCE);

/* Setters. Each walks BtlObj.attached, so a change reaches the whole
   assembly. */
extern int  BtlObjSetScript(BtlObj *obj, BtlSeqStep *script);
extern void BtlObjSetScaleTo(BtlObj *obj, long scale);
extern void BtlObjSetRgbNow(BtlObj *obj, short r, short g, short b);
extern void BtlObjSetAttr(BtlObj *obj, u_long bits);
extern void BtlObjClearAttr(BtlObj *obj, u_long bits);
extern void BtlObjSetScale(BtlObj *obj, long x, long y, long z);
extern void BtlObjSetPhase(BtlObj *obj, u_char phase);
extern void BtlObjSetTimer(BtlObj *obj, short frames);

/* Three the sources used to reach through two prototypes each. The forms the
   definitions take serve every caller: what the wide declarations bought was
   a narrowing the callers were already doing. */
extern void BtlObjSetMotion(BtlObj *obj, u_char motion);
extern void BtlObjSetMarkNum(BtlObj *obj, u_char num);
extern void BtlObjSetKind(BtlObj *obj, u_char kind);
extern void BtlObjSetPos(BtlObj *obj, long x, long y, long z);
extern void BtlObjSetFade(BtlObj *obj, u_char rate);
extern void BtlObjSetRgb(BtlObj *obj, short r, int g, short b);

#endif
