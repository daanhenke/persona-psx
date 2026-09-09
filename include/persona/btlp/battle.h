#ifndef PERSONA_BTLP_BATTLE_H
#define PERSONA_BTLP_BATTLE_H

/* Persona 1 (JP) - the battle overlay's shared state and entry points.
 *
 * Everything here was declared over and over in the individual battle
 * sources; these are the forms every one of them agreed on. Anything the
 * sources declare differently from each other is deliberately left out, as
 * the prototype decides how the arguments are converted and so which code
 * comes out - those stay next to the code that depends on them.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>

/* Shared state. */
extern short g_btl_offer_slot;
extern BtlActor g_btl_actors[];
extern short g_btl_actor_slot;
extern BtlActor g_btl_enemies[];
extern u_char *g_btl_prim_pool;
extern u_char g_btl_talk_scene[];
extern u_char g_btl_talk_stage[];
extern u_char g_btl_talk_depth;
extern u_short g_btl_clut[];
extern short g_btl_talk_target;
extern u_char g_btl_frame;
extern int g_btl_phase;
extern POLY_FT4 *g_btl_polyft4_next;
extern u_char g_btl_fast_anim;
extern short g_btl_obj_x;
extern short g_btl_obj_y;
extern DR_MODE *g_btl_drmode_next;
extern u_char g_btl_talk_flags;

/* Entry points. */
/* Which face is loaded, -1 for none. */
extern int g_btl_face_id;

extern void BtlHudLoad(void);
extern void BtlDrawFrame(void);
extern void BtlRunFrames(int frames);
extern void BtlQueueVramLoad(const void *src, int x, int y, int w,
                             int h);
extern void BtlSeqPlay(const u_char *script);
extern void BtlSeqRun(void);
extern void BtlSeqWaitDone(void);
extern void BtlFaceClose(void);
extern void BtlPanelClose(void);
extern void BtlEnemiesReset(void);
extern void BtlIndicatorClear(void);
extern void BtlEndTalking(void);
extern void BtlSeqClear(void);
extern void BtlBoxClose(void);
extern void BtlHudHide(void);
extern void BtlPartyReset(void);
extern void BtlWaitAnyKey(void);
extern void BtlTalkersLeave(void);
extern int BtlStockHasRoom(void);

#endif
