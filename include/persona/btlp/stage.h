/* Persona 1 (JP) - the battle's stages.
 *
 * A battle is not one loop but a table of them. ovl_btlp_entry walks it:
 *
 *     while ((stage = g_btl_stages[g_btl_stage]) != NULL) {
 *         g_btl_step = 0;
 *         stage();
 *         BtlDrawFrame();
 *     }
 *
 * A stage returns to hand over, and the null past the last entry is what ends
 * the battle - so a stage leaves behind the index of whoever runs next, and
 * anything that wants to cut a battle short only has to write BTL_STAGE_CLOSE.
 * That is how a negotiation ends one: talkstep.c sets the stage rather than
 * unwinding.
 *
 * The walk clears g_btl_step on the way in, and a stage uses it as its own
 * inner state - BtlStageRound is a switch on it from end to end.
 */
#ifndef PERSONA_BTLP_STAGE_H
#define PERSONA_BTLP_STAGE_H

#include <decomp/types.h>

/* Where each stage sits in g_btl_stages. */
#define BTL_STAGE_OPEN    0  /* BtlStageOpen    - roundflow.c   */
#define BTL_STAGE_COMMAND 1  /* BtlStageCommand - the picker    */
#define BTL_STAGE_ROUND   2  /* BtlStageRound   - roundflow.c   */
#define BTL_STAGE_CLOSE   3  /* BtlStageClose   - roundflow.c   */

/* Which stage runs next, and where that stage has got to. Both are read and
   written all over the overlay, so they are here rather than in whichever
   file happens to set them. */
extern u_char g_btl_stage;
extern u_char g_btl_step;

extern void (*g_btl_stages[])(void);

extern void BtlStageOpen(void);
extern void BtlStageCommand(void);
extern void BtlStageRound(void);
extern void BtlStageClose(void);

#endif
