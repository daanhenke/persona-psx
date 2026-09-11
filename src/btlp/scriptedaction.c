/* Persona 1 (JP) - the move a fighter is told to make.  BTLP only.
 *   0x80099124 BtlAimScriptedMember   0x800992C8 BtlAimScriptedEnemy
 *
 * A placement scene is a fight nobody picks anything in: every turn is read
 * out of a table instead. BtlStageRound calls one of these in place of the
 * menu for a party member and in place of BtlChooseEnemyMove for an enemy,
 * and each copies one sixteen-byte record onto the fighter - what it is
 * doing, who it hits, where it comes in the round and which move it makes.
 *
 * The member's record is found by character, ten to an encounter. The
 * enemy's is found by the mark its object carries, nine to an encounter,
 * except in the two fights that run off a counter instead: those hand out
 * consecutive records, so the same enemy gets a different line each time it
 * is asked.
 *
 * Four encounters carry a special case each, and all four are here rather
 * than in the table because the table has no room for them: one puts the HUD
 * up, one arms a scene, one hits the whole field, and one turns a flag over.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* One fighter's scripted turn. */
typedef struct {
    /* 0x0 */ u_char  pad0[4];
    /* 0x4 */ u_long  flags;    /* or-ed onto the fighter's own            */
    /* 0x8 */ u_short targets;  /* one bit per slot, for the member table;
                                   the enemy table names a single slot at
                                   `order` and the mask is built from it   */
    /* 0xA */ u_char  padA[2];
    /* 0xC */ u_char  order;
    /* 0xD */ u_char  action;
    /* 0xE */ u_char  move;
    /* 0xF */ u_char  padF[1];
} BtlScriptedAction;            /* 16 bytes */

/* The four tables and the counter. Which of each pair is used depends on how
   far through the game the encounter is; the second member table is reached
   through a base the compiler biased, so it keeps the name of the run its
   address falls in. */
extern BtlScriptedAction g_btl_member_actions[];
extern BtlScriptedAction g_btl_enemy_actions[];
extern BtlScriptedAction g_btl_enemy_lines[];
extern int               g_btl_enemy_line_next;
extern u_char            D_800CE460[];

#define g_btl_member_actions2 ((BtlScriptedAction *)D_800CE460)

#define BTL_MEMBER_ACTIONS 10   /* records per encounter, by character key */
#define BTL_ENEMY_ACTIONS  9    /* records per encounter, by mark          */

/* The encounter the second member table starts at, and the pair that hand
   out consecutive enemy records rather than one per mark. */
#define BTL_SCRIPT_LATE       0x11
#define BTL_SCRIPT_LINES_FIRST 0x11
#define BTL_SCRIPT_LINES_COUNT 2

/* The action code a member takes when the flag below is already up. */
#define BTL_ACTION_SCRIPTED 6

/* Three flags of the fighter's own. The first is turned over into the second
   the moment the scripted turn is handed out, and the third is the same trade
   one encounter makes on its own. */
#define BTL_ACTOR_SCRIPT_READY 0x20000000
#define BTL_ACTOR_SCRIPT_DONE  0x40000000
#define BTL_ACTOR_SCRIPT_ALT   0x04000000

/* The four encounters with a case of their own, and the fighters they watch. */
#define BTL_SCRIPT_HUD_ENCOUNTER   0
#define BTL_SCRIPT_HUD_KEY         1
#define BTL_SCRIPT_SCENE_ENCOUNTER 2
#define BTL_SCRIPT_ALT_ENCOUNTER   0x12
#define BTL_SCRIPT_ALT_KEY         7

/* The one that names its target by slot rather than by character. */
#define BTL_SCRIPT_BY_SLOT_ENCOUNTER 7

/* And the one that hits the whole field, and rewrites its own record so the
   next turn is a different action. */
#define BTL_SCRIPT_ALL_ENCOUNTER 5
#define BTL_SCRIPT_ALL_TARGETS   0x1F
#define BTL_SCRIPT_ALL_ACTION    0xE

extern int BtlActorSlotByKey(u_int key);


void BtlAimScriptedMember(BtlActor *a)
{
    BtlScriptedAction *rec;
    int                enc;

    enc = g_btl_encounter;
    if (enc < BTL_SCRIPT_LATE) {
        rec = g_btl_member_actions + enc * BTL_MEMBER_ACTIONS + a->c.key;
    } else {
        rec = g_btl_member_actions2 + enc * BTL_MEMBER_ACTIONS + a->c.key;
    }

    if (a->c.key == 2 && (a->flags & BTL_ACTOR_SCRIPT_READY)) {
        a->action  = BTL_ACTION_SCRIPTED;
        a->targets = rec->targets;
        a->order   = rec->order;
        a->move    = rec->move;
        a->flags   = (a->flags & ~BTL_ACTOR_SCRIPT_READY)
                     | BTL_ACTOR_SCRIPT_DONE;
    } else {
        if (g_btl_encounter == BTL_SCRIPT_HUD_ENCOUNTER
            && a->c.key == BTL_SCRIPT_HUD_KEY) {
            g_btl_scene_hud = 1;
        }
        if (g_btl_encounter == BTL_SCRIPT_SCENE_ENCOUNTER
            && a->c.key == BTL_SCRIPT_SCENE_ENCOUNTER) {
            g_btl_scene_pending = 1;
        }
        a->action  = rec->action;
        a->targets = rec->targets;
        a->order   = rec->order;
        a->move    = rec->move;
        if (g_btl_encounter == BTL_SCRIPT_ALT_ENCOUNTER
            && a->c.key == BTL_SCRIPT_ALT_KEY
            && (a->flags & BTL_ACTOR_SCRIPT_ALT)) {
            a->flags = (a->flags & ~BTL_ACTOR_SCRIPT_ALT)
                       | BTL_ACTOR_SCRIPT_DONE;
        } else {
            a->flags |= rec->flags;
        }
    }
}

void BtlAimScriptedEnemy(BtlActor *a)
{
    BtlScriptedAction *rec;
    BtlScriptedAction *block;
    int                next;
    int                slot;

    /* The term the base binds to is the last one written, and the image binds
       it to the record rather than to the encounter's block - so the block is
       written first here and last in the other arm. */
    if ((u_int)((u_short)g_btl_encounter - BTL_SCRIPT_LINES_FIRST)
        < BTL_SCRIPT_LINES_COUNT) {
        next = g_btl_enemy_line_next;
        rec = g_btl_enemy_lines + g_btl_encounter * BTL_ENEMY_ACTIONS + next;
        g_btl_enemy_line_next = next + 1;
    } else {
        block = g_btl_enemy_actions + g_btl_encounter * BTL_ENEMY_ACTIONS;
        rec = block + a->obj->mark_num;
    }

    a->action = rec->action;
    if (g_btl_encounter != BTL_SCRIPT_BY_SLOT_ENCOUNTER) {
        slot = BtlActorSlotByKey(rec->order);
    } else {
        slot = rec->order;
    }
    a->order   = slot;
    a->targets = 1 << slot;
    a->move    = rec->move;
    a->flags  |= rec->flags;
    a->obj->unkD3 = 0;

    if (g_btl_encounter == BTL_SCRIPT_ALL_ENCOUNTER) {
        a->targets  = BTL_SCRIPT_ALL_TARGETS;
        rec->action = BTL_SCRIPT_ALL_ACTION;
    }
}
