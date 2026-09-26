/* Persona 1 (JP) - a frame of walking around a room.  ADV @ 0x8007E814.
 *
 * AdvFieldTick's walking state. The d-pad picks the walk direction (4 for
 * none); a leader half way through a step finishes it, and one standing on a
 * tile first looks at what the tile holds - a step script, a trigger, or an
 * exit left the way the player is pushing - before LeaderWalkStep takes it
 * on. Leaving the pad alone for 0x100 frames puts the idle sprite over the
 * leader.
 *
 * A standing leader can also open the menus, and the accept button acts on
 * whatever is in front of it: talking to one of the room's actors (0-7),
 * opening a chest (8-15), examining a mark (16-23), or an approach script
 * for the tile, with a stock answer when there is none.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

#define L g_adv_actors[0]

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_slots      ((Slot *)0x800DC10C)
#define g_seq_handle (*(short *)0x801F5390)

#define PAD_UP    0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN  0x4000
#define PAD_LEFT  0x8000

#define WALK_NONE     4
#define WALK_HOLD     0x14
#define LEADER_STANDS 1
#define IDLE_FRAMES   0x100
#define IDLE_SLOT     0x34
#define LABEL_SLOT    0x28
#define TILE_ENTRY    5    /* 5 and 6 show the entry's name */
#define TRIGGER_NONE  0xFF
#define SCRIPT_NONE   ((u_char *)-1)
#define CHEST_ACTOR   8
#define TALK_FLIPS    0x100 /* the actor turns to face the leader */

/* The two sound bytes and the automap word, read as the halfwords the
   original reads them as. */
#define SCENE_SOUNDS (*(u_short *)&g_adv_scene->exit_sound)
#define SCENE_MAP    (*(u_short *)&g_adv_scene->pad31[1])

/* The leader's tile and the one it last stood on, each as one halfword. */
typedef struct {
    u_char  pad00[0x1C];
    u_short tile;       /* x and y */
    u_short next;       /* next_x and next_y */
    u_short home;       /* home_x and home_y */
    u_char  pad22[0xA];
} AdvActorTiles;

#define L_TILE (((AdvActorTiles *)0x801F15D8)->tile)
#define L_HOME (((AdvActorTiles *)0x801F15D8)->home)

typedef struct {
    /* 0x0 */ short   unk0;
    /* 0x2 */ u_char  kind;
    /* 0x3 */ u_char  pad03;
    /* 0x4 */ short   flag;
    /* 0x6 */ u_short payload;
    /* 0x8 */ u_char  pad08[2];
} AdvChest;

#define g_chest_defs ((AdvChest *)0x80100BD0)

extern u_short g_adv_walk_dir;
extern u_char  g_walk_hold;
extern short   g_script_97C;        /* frames until the idle sprite */
extern short   g_field_exit;
extern u_char  g_script_leave;
extern int     g_pad_held[];
extern int     g_pad_pressed[];
extern u_short g_key_dash;
extern u_short g_key_menu;
extern u_short g_key_persona;
extern u_short g_key_map;
extern u_char  g_trigger_dirs[];    /* 1 << direction */
extern u_char  g_chest_dirs[];      /* by scene: the sides a chest opens from */
extern Slot   *g_slot_cur;
extern u_char  g_idle_def[];
extern u_char  g_nothing_script[];
extern u_char  g_chest_shut_script[];

extern void   SlotClear(u_char slot);
extern void   SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void   SsSeqStop(short seq);
extern void   ActorSetStandSprite(u_char a);
extern void   ActorStepToward(u_char actor, u_char dir);
extern u_char ActorAtTile(u_char x, u_char y);
extern u_char ActorFindAt(u_char x, u_char y);
extern void   ActorsSetDepth();
extern void   ActorsPlaceSprites(void);
extern void   SlotsApplyXScale(void);
extern void   ActorWalkStep(u_char i);
extern void   LeaderWalkStep(void);
extern u_char SceneTileAt(u_char x, u_char y);
extern u_char SceneFindStep(u_char x, u_char y);
extern u_char SceneFindTrigger(u_char x, u_char y);
extern u_char SceneFindEntry(const AdvActor *a);
extern u_char SceneFindApproach(u_char x, u_char y, u_char dirs, u_char kind);
extern void   SceneDrawEntryLabel(void);
extern void   AdvExitPlaceLeader(u_char n);
extern void   SceneApplyEntry(u_char entry);
extern void   SceneLeaderArrive();
extern int    AdvRunScript(u_char *s);
extern void   AdvSceneFadeOut();
extern void   MainMenuOpen(void);
extern void   PersonaDataOpen(void);
extern void   MapScreenOpen(void);
extern void   AdvQueueCmdBar(void);
extern void   AdvRoomRebuild(void);
extern void   AdvFadeUpBlocking(short step, short limit);
extern u_char InputCheckAcceptA(u_char kind);
extern u_char FlagBank1Get(short id);
extern void   FlagBank1Set(short id);
extern void   AdvSoundCommand(short cmd);
extern void   CinemaDim(short step);
extern void   AdvRunFrame(void);
extern void   AdvChestOpen(u_char chest);
extern u_char AdvChestApply(u_char chest);
extern void   AdvChestShowItem(u_char actor);
extern void   AdvChestShowMoney(u_char actor);
extern void   AdvChestShowEmpty(u_char actor);

/* 99.46%: every instruction is the image's but three register choices.
   The image compares a copy of `e` against 0xFF where this compares `e`
   itself, and it gives `who` s1 and the actors' base s2 where this build
   swaps them (the chest's index copy follows). Tried: int/short/u_char for
   e and who in every pairing, assignments inside the tests and indexes,
   u_char re-casts, unprototyped callees, separate talk and chest indexes,
   declaration orders. The tile halfwords must be struct members (not
   casted constants), or the addresses go into registers at expand time. */
#ifdef NON_MATCHING
void AdvFieldRun(void)
{
    u_char    n;
    int       e;
    short     who;
    u_char   *script;
    AdvActor *a;
    short     i;
    int       unused[2];

    g_adv_walk_dir = L.dir;
    if (g_pad_held[0] & PAD_UP) {
        g_adv_walk_dir = 0;
    } else if (g_pad_held[0] & PAD_DOWN) {
        g_adv_walk_dir = 1;
    } else if (g_pad_held[0] & PAD_LEFT) {
        g_adv_walk_dir = 2;
    } else if (g_pad_held[0] & PAD_RIGHT) {
        g_adv_walk_dir = 3;
    } else {
        g_adv_walk_dir = WALK_NONE;
        g_walk_hold = WALK_HOLD;
    }
    if (g_pad_held[0] == 0) {
        if (g_script_97C != -1) {
            g_script_97C--;
        }
    } else {
        g_script_97C = IDLE_FRAMES;
        SlotClear(IDLE_SLOT);
    }
    if (g_script_97C == 0) {
        SlotInit(g_idle_def, IDLE_SLOT, L.z, L.world_x, L.world_y + 4);
    }
    if (!(L.phase & 1)) {
        L.steps = 1;
        if (g_key_dash & g_pad_held[0]) {
            L.steps = 2;
        }
    }

    if (L.phase & 0xF) {
        ActorWalkStep(0);
        g_walk_hold = WALK_HOLD;
    } else {
        L.slope = 0;
        n = SceneFindStep(L.x, L.y);
        if (n != TRIGGER_NONE && L.kind != LEADER_STANDS) {
            script = (u_char *)g_adv_scene->steps[n].script;
            goto run;
        }
        n = SceneFindTrigger(L.x, L.y);
        if (n != TRIGGER_NONE && L.kind != LEADER_STANDS) {
            script = (u_char *)g_adv_scene->triggers[n].script;
        run:
            if (script != SCRIPT_NONE) {
                g_script_leave = AdvRunScript(script);
            }
            goto walk;
        }
        e = SceneFindEntry(&L);
        if (e != TRIGGER_NONE) {
            if (L_HOME != L_TILE) goto off;
        leave:
            if (g_adv_scene->entries[e].dirs & g_trigger_dirs[g_adv_walk_dir]) {
                g_field_exit = 0xFF;
                SsSeqStop(g_seq_handle);
                ActorSetStandSprite(0);
                L.kind = LEADER_STANDS;
                AdvExitPlaceLeader(e);
                SceneApplyEntry(e);
                SceneLeaderArrive(e);
                goto moved;
            }
            goto walk;
        off:
            if (g_adv_walk_dir == WALK_NONE) goto stand;
            L.home_x = L.x;
            L.home_y = L.y;
            goto leave;
        }
    walk:
        if (g_adv_walk_dir == WALK_NONE) {
        stand:
            if (L.kind != LEADER_STANDS) {
                SsSeqStop(g_seq_handle);
                ActorSetStandSprite(0);
                L.kind = LEADER_STANDS;
            }
        } else {
            LeaderWalkStep();
        }
    }

moved:
    e = SceneTileAt(L.x, L.y);
    if (e == TILE_ENTRY || e == TILE_ENTRY + 1) {
        SceneDrawEntryLabel();
    } else {
        SlotClear(LABEL_SLOT);
    }
    if (L.kind == LEADER_STANDS) {
        if (g_key_menu & g_pad_pressed[0]) {
            AdvSceneFadeOut();
            MainMenuOpen();
            goto back;
        }
        if (g_key_persona & g_pad_pressed[0]) {
            AdvSceneFadeOut();
            PersonaDataOpen();
            goto back;
        }
        if (g_key_map & g_pad_pressed[0]) {
            if (SCENE_MAP != 0xFFFF) {
                AdvSceneFadeOut();
                MapScreenOpen();
            back:
                AdvQueueCmdBar();
                AdvRoomRebuild();
                AdvFadeUpBlocking(8, 0x80);
            }
        } else if (InputCheckAcceptA(0)) {
            ActorStepToward(0, L.dir);
            switch (ActorAtTile(L.next_x, L.next_y)) {
            case 0 ... 7:
                ActorStepToward(0, L.dir);
                who = ActorFindAt(L.next_x, L.next_y);
                a = &g_adv_actors[who];
                if ((int)a->script == -1) goto approach;
                if (g_adv_actors[who].flags & TALK_FLIPS) {
                    g_adv_actors[who].next_dir = g_adv_actors[who].dir;
                    g_adv_actors[who].dir = (L.dir & 3) ^ 1;
                    ActorSetStandSprite(who);
                }
                ActorsSetDepth(0, 0xF);
                ActorsPlaceSprites();
                SlotsApplyXScale();
                g_script_leave = AdvRunScript((u_char *)a->script);
                if (g_adv_actors[who].flags & TALK_FLIPS) {
                    g_adv_actors[who].dir = g_adv_actors[who].next_dir;
                    ActorSetStandSprite(who);
                }
                break;
            case 8 ... 15:
                ActorStepToward(0, L.dir);
                who = ActorFindAt(L.next_x, L.next_y);
                if (FlagBank1Get(g_chest_defs[who - CHEST_ACTOR].flag)) {
                    goto empty;
                }
                if (!(g_chest_dirs[g_adv_scene->pad25] & g_trigger_dirs[L.dir])) {
                    goto shut;
                }
                AdvSoundCommand(SCENE_SOUNDS >> 8);
                g_slot_cur = &g_slots[who];
                g_slot_cur->u_add = 0x30;
                FlagBank1Set(g_chest_defs[who - CHEST_ACTOR].flag);
                for (i = 0; i < 8; i++) {
                    CinemaDim(i);
                    AdvRunFrame();
                }
                AdvChestOpen(who - CHEST_ACTOR);
                switch (AdvChestApply(who - CHEST_ACTOR)) {
                case 1:
                    AdvChestShowItem(who);
                    break;
                case 2:
                    AdvChestShowMoney(who);
                    break;
                }
                for (i = 7; i >= 0; i--) {
                    CinemaDim(i);
                    AdvRunFrame();
                }
                break;
            shut:
                script = g_chest_shut_script;
                goto run2;
            empty:
                AdvChestShowEmpty(who);
                break;
            case 16 ... 23:
                ActorStepToward(0, L.dir);
                a = &g_adv_actors[ActorFindAt(L.next_x, L.next_y)];
                if ((int)a->script == -1) goto approach;
                ActorsSetDepth(0, 0xF);
                ActorsPlaceSprites();
                SlotsApplyXScale();
                script = (u_char *)a->script;
                goto run2;
            case ACTOR_NA:
            approach:
                ActorStepToward(0, L.dir);
                n = SceneFindApproach(L.next_x, L.next_y, g_trigger_dirs[L.dir],
                                      L.lift);
                if (n != TRIGGER_NONE) {
                    script = (u_char *)g_adv_scene->approaches[n].script;
                } else {
                    script = g_nothing_script;
                }
            run2:
                g_script_leave = AdvRunScript(script);
                break;
            }
        }
    }
    ActorsSetDepth(0, 0x18);
    ActorsPlaceSprites();
    SlotsApplyXScale();
}

#undef L
#else
INCLUDE_ASM("adv/nonmatchings/game/fieldrun", AdvFieldRun);
#endif
