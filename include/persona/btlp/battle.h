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
/* Which fight this is. Two sources declare it int rather than short; the
   image reads it with lh everywhere, so short is the one it was written as. */
extern short g_btl_encounter;

/* Set while the party is only being placed on the field - a formation screen
   rather than a fight. Most of the round is skipped under it. */
extern u_char g_btl_place_party;

/* What kind of fight this is. Read as a word almost everywhere; the handful
   of byte reads are a cast at the use, not a different variable. */
extern int g_btl_battle_kind;

/* The pages the battle draws its sprites out of - one per upload slot; a
   wide image takes two or four in a row. Every source but uploadtim.c has
   this unsigned, which is also what GetTPage hands back. */
extern u_short g_btl_tpage[];

/* The moon phase the battle is fought under - its own copy of g_moon, taken
   as the fight opens and written back as it closes. */
extern u_char g_btl_moon;

/* The debug HUD, and the two switches beside it in the main executable: one
   ends an ordinary fight the moment it starts, the other is raised by the
   fight that leads into another. Retail leaves the first zero. */
extern u_char g_btl_debug_hud;
extern u_char g_btl_debug_skip;
extern u_char g_btl_chain_battle;

/* Two more of the same, read only by the enemy AI: one makes every enemy
   stand still, the other stops any of them running. Both zero on disc. */
extern u_char g_btl_debug_idle;
extern u_char g_btl_debug_no_flee;

/* And one that keeps a fight to the party side alone. */
extern u_char g_btl_debug_party_only;

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
extern void BtlHudShow(void);

/* One marker object per party slot, plus the shared one past them. */
extern BtlObj *g_btl_marker_obj[];

/* Where each marker is drawn. Two rows of five, one row per side; only the x
   is ever written, and only for a member still in the fight. */
typedef struct {
    /* 0x0 */ short  x;
    /* 0x2 */ u_char pad2[6];
} BtlMarker;                    /* 8 bytes */

extern BtlMarker g_btl_member_marker[];
extern void      BtlPlaceMemberMarkers(int row, int near);
extern void    BtlBuildMarkers(void);
/* The middle argument is whether the marker goes up or comes down, and
   the last is which of the seven it is - not a level. */
extern void    BtlShowMarker(int slot, int on, int kind);
extern void    BtlEnemiesResetGfx(void);
extern void    BtlEffectDrop(void);

/* The colour the whole scene is drawn at. */
extern short g_btl_scene_rgb[];

/* How fast a line is put up: 0 slow, 2 off altogether. */
extern u_char g_btl_msg_speed;

/* Suppresses the hit sound while a member's own turn plays. */
extern u_char g_btl_se_off;

/* The debug grid, cleared at the top of every round. */
extern u_char g_btl_debug_grid_cells[];

extern void BtlDeriveBattleStats(BtlActor *a);
extern int  BtlActorPersona(int slot);
extern int  BtlAnyStanding(void);
extern void BtlRefreshMarkers(void);
extern void BtlShowReadyMarkers(void);
extern void BtlDrawFrame(void);
extern void BtlRunFrames(int frames);
extern void BtlQueueVramLoad(const void *src, int x, int y, int w,
                             int h);
extern void BtlSeqPlay(const u_char *script);
extern void BtlSeqSetState(int state, int frames);
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
