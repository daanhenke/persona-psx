/* Persona 1 (JP) - the two-option prompt.
 *
 * A prompt is two objects standing one above the other, each a picture out of
 * g_btl_pick_defs with a frame in front of it. BtlChoiceSpawn builds one set
 * and leaves the two frames in that set's array, and BtlChoiceUpdate reads the
 * pad for it and keeps the set's row.
 *
 * There are two sets, built from the same routine with different pictures -
 * 0x0D and 0x10 for the first, 0x0E and 0x0F for the second - and each has an
 * array and a row of its own. Which question each one asks is not settled, so
 * they are numbered rather than named.
 */
#ifndef PERSONA_BTLP_CHOICE_H
#define PERSONA_BTLP_CHOICE_H

#include <decomp/types.h>
#include <persona/btlp/object.h>

/* Options to a prompt. */
#define CHOICE_OPTIONS 2

/* How bright the option that is up is drawn, and the other one, and a fade
   that lands on the next frame. */
#define CHOICE_LIT  0xFF
#define CHOICE_DARK 0x60
#define CHOICE_FADE 0xFF

extern BtlObj *g_btl_choice0_objs[];
extern BtlObj *g_btl_choice1_objs[];
extern short   g_btl_choice0_row;
extern short   g_btl_choice1_row;

/* The help line the first set keeps up for the option that is up. */
extern const u_char *g_btl_choice_help[CHOICE_OPTIONS];

/* Builds one set. memberact.c. */
extern void BtlChoiceSpawn(int set);

/* Each set put up and painted, and shrunk away. choiceprompt.c. */
extern void BtlOpenChoice0(void);
extern void BtlOpenChoice1(void);
extern void BtlCloseChoice0(void);
extern void BtlCloseChoice1(void);

/* One frame of whichever set `row` is the row of. The answer is the row on a
   confirm, -1 on a cancel, -2 on the third key and BTL_PICK_WAIT while
   nothing has been decided. choiceupdate.c. */
extern int BtlChoiceUpdate(short *row);

#endif
