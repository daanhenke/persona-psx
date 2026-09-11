/* Persona 1 (JP) - the two-option prompt, put up and taken away.  BTLP only.
 *   0x800AA4C8 BtlOpenChoice0   0x800AA56C BtlOpenChoice1
 *   0x800AA610 BtlCloseChoice0  0x800AA67C BtlCloseChoice1
 *
 * A prompt is two objects standing one above the other, each a picture out of
 * g_btl_pick_defs with a frame in front of it. BtlChoiceSpawn builds one set
 * and leaves the two frames in that set's array; func_800A3998 is what reads
 * the pad and keeps the set's row.
 *
 * There are two sets, built from the same routine with different pictures -
 * 0x0D and 0x10 for the first, 0x0E and 0x0F for the second - and each has an
 * array and a row of its own. Which question each one asks is not settled, so
 * they are numbered rather than named.
 *
 * Opening a set spawns it and then paints it: the row that is up is drawn at
 * full brightness and the other at 0x60, and both are given the fastest fade
 * so the change lands on the next frame. Closing it is the shrink every board
 * shuts with - phase one, down to a hundredth, motion four.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* Options to a prompt. */
#define CHOICE_OPTIONS 2

/* How bright the option that is up is drawn, and the other one. */
#define CHOICE_LIT  0xFF
#define CHOICE_DARK 0x60

/* One frame to get there. */
#define CHOICE_FADE 0xFF

/* What a prompt shuts with: unity in BtlObjSetScaleTo's units, and the motion
   that walks down to it. */
#define CHOICE_SHUT_SCALE 0x100
#define CHOICE_SHUT_PHASE 1
#define CHOICE_SHUT_MOTION 4

/* Builds one set and leaves its two frames in that set's array. Still asm. */
extern void BtlChoiceSpawn(int set);

extern BtlObj *g_btl_choice0_objs[];
extern BtlObj *g_btl_choice1_objs[];
extern short   g_btl_choice0_row;
extern short   g_btl_choice1_row;

/* The frame is what the set keeps, so the colour goes on the piece behind it -
   and it is read back through the array each time rather than held, which is
   what the image does. */
void BtlOpenChoice0(void)
{
    int   i;
    short level;

    BtlChoiceSpawn(0);
    i = 0;
    do {
        level = CHOICE_DARK;
        if (i == g_btl_choice0_row) {
            level = CHOICE_LIT;
        }
        g_btl_choice0_objs[i]->attached->rgb_to[0] = level;
        g_btl_choice0_objs[i]->attached->rgb_to[1] = level;
        g_btl_choice0_objs[i]->attached->rgb_to[2] = level;
        g_btl_choice0_objs[i]->attached->fade      = CHOICE_FADE;
        i++;
    } while (i < CHOICE_OPTIONS);
}

void BtlOpenChoice1(void)
{
    int   i;
    short level;

    BtlChoiceSpawn(1);
    i = 0;
    do {
        level = CHOICE_DARK;
        if (i == g_btl_choice1_row) {
            level = CHOICE_LIT;
        }
        g_btl_choice1_objs[i]->attached->rgb_to[0] = level;
        g_btl_choice1_objs[i]->attached->rgb_to[1] = level;
        g_btl_choice1_objs[i]->attached->rgb_to[2] = level;
        g_btl_choice1_objs[i]->attached->fade      = CHOICE_FADE;
        i++;
    } while (i < CHOICE_OPTIONS);
}

void BtlCloseChoice0(void)
{
    int i;

    i = 0;
    do {
        BtlObjSetPhase(g_btl_choice0_objs[i], CHOICE_SHUT_PHASE);
        BtlObjSetScaleTo(g_btl_choice0_objs[i], CHOICE_SHUT_SCALE);
        BtlObjSetMotion(g_btl_choice0_objs[i], CHOICE_SHUT_MOTION);
        i++;
    } while (i < CHOICE_OPTIONS);
}

void BtlCloseChoice1(void)
{
    int i;

    i = 0;
    do {
        BtlObjSetPhase(g_btl_choice1_objs[i], CHOICE_SHUT_PHASE);
        BtlObjSetScaleTo(g_btl_choice1_objs[i], CHOICE_SHUT_SCALE);
        BtlObjSetMotion(g_btl_choice1_objs[i], CHOICE_SHUT_MOTION);
        i++;
    } while (i < CHOICE_OPTIONS);
}
