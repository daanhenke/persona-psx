/* Persona 1 (JP) - the motions a party member's turn is played out in.
 * BTLP only.
 *   0x800ADAD4 BtlChoiceSpawn     0x800ADC24 BtlMemberMotion02
 *   0x800AE8B0 BtlMemberStrike    0x800AF60C BtlMemberMotion06
 *   0x800B034C BtlMemberMotion05
 *
 * Entries 2, 5 and 6 of g_btl_member_motion, and the routine the blow in
 * entry 2 lands through. Entry 2 is a plain attack, entry 5 the persona
 * coming out, entry 6 a spell; the choice prompt's spawn sits at the head of
 * the unit because that is where the image keeps it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/object.h>

/* The two prompts. Each is two pictures out of g_btl_pick_defs standing one
   above the other, with the shared frame of g_btl_obj_defs in front of each;
   the frame is what the set's array keeps, and the picture hangs off its
   `attached` link. See choiceprompt.c, which paints and shuts them. */
#define CHOICE_SETS    2
#define CHOICE_OPTIONS 2

/* Which group the prompt lives in, whether it is drawn, and the one template
   the frame is always taken from. */
#define CHOICE_GROUP 1
#define CHOICE_DRAW  1
#define CHOICE_FRAME 6

/* Marks the picture as the piece a set colours rather than the frame. */
#define CHOICE_ITEM_BIT 0x400

/* The frame is put up at unity and on the motion that opens it out. */
#define CHOICE_SCALE_XY 0x100
#define CHOICE_SCALE_Z  0x1000
#define CHOICE_ATTR     0x40000000
#define CHOICE_MOTION   3

/* The bytes BtlObjAlloc leaves at +0xCD and +0xCE, the same pair the picker
   uses for its own two passes. */
#define CHOICE_ITEM_CD  0x1F
#define CHOICE_ITEM_CE  0x27
#define CHOICE_FRAME_CD 0x19
#define CHOICE_FRAME_CE 0x1E

extern const BtlObjDef g_btl_pick_defs[];
extern const BtlObjDef g_btl_obj_defs[];
extern BtlObj         *g_btl_choice0_objs[];
extern BtlObj         *g_btl_choice1_objs[];

long g_btl_choice_pos[CHOICE_SETS][4] = {
    {0x280000, 0x180000, 0, 0},
    {0x280000, 0x280000, 0, 0},
};

u_short g_btl_choice_kind[CHOICE_SETS][CHOICE_OPTIONS] = {
    {0x0D, 0x10},
    {0x0E, 0x0F},
};

BtlObj **g_btl_choice_objs[CHOICE_SETS] = {g_btl_choice0_objs, g_btl_choice1_objs};

/* Both options of one set come out of a single pass, so the frame of the
   first is already in the group's list when the second picture is linked
   after it - and `after` stays the picture rather than the frame, which is
   what puts the second pair behind the first. */
void BtlChoiceSpawn(int set)
{
    BtlObj *obj;
    BtlObj *pic;
    int     i;

    i = 0;
    pic = 0;
    do {
        obj = BtlObjAlloc(g_btl_pick_defs, CHOICE_GROUP, pic, CHOICE_DRAW,
                          g_btl_choice_kind[set][i], g_btl_choice_pos[i], CHOICE_ITEM_CD,
                          CHOICE_ITEM_CE);
        pic = obj;
        pic->attr |= CHOICE_ITEM_BIT;
        obj = BtlObjAlloc(g_btl_obj_defs, CHOICE_GROUP, pic, CHOICE_DRAW,
                          CHOICE_FRAME, g_btl_choice_pos[i], CHOICE_FRAME_CD,
                          CHOICE_FRAME_CE);
        obj->attached = pic;
        BtlObjSetScale(obj, CHOICE_SCALE_XY, CHOICE_SCALE_XY, CHOICE_SCALE_Z);
        BtlObjSetAttr(obj, CHOICE_ATTR);
        BtlObjSetMotion(obj, CHOICE_MOTION);
        g_btl_choice_objs[set][i] = obj;
        i++;
    } while (i < CHOICE_OPTIONS);
}

INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberMotion02);

INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberStrike);

INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberMotion06);

INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberMotion05);
