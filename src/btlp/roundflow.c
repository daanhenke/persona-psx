/* Persona 1 (JP) - three of the battle's four stages.  BTLP only.
 *   0x8008E8A0 BtlStageOpen
 *   0x8008EB68 BtlStageClose
 *   0x8008F1FC BtlStageRound
 *   0x80091384 BtlChooseEnemyMove
 *
 * The table these three sit in, and the walk over it, are described in
 * persona/btlp/stage.h. The one stage missing here is the command menu, which
 * is BtlStageCommand and lives with the rest of the picker.
 *
 * BtlStageOpen settles the field: it waits out whatever the intro is still
 * doing, clears what the last round left behind, reads each fighter's voice
 * bank off the disc, and hands over to the picker - unless there is nothing
 * to pick, in which case it goes straight on without one.
 *
 * BtlStageRound is the round itself, and is the largest routine in the
 * overlay: everything from the first attack to the last is a case of its
 * switch on g_btl_step.
 *
 * BtlStageClose folds what the battle won into the save's event flags and
 * decides which scene the game goes back to. Both it and BtlStageOpen are
 * one loop from end to end - the frame they draw while waiting sits past the
 * last of the work rather than beside the test - which is what puts their
 * constants in saved registers before the frame is even set up.
 *
 * BtlChooseEnemyMove is not a stage. It picks the move one enemy makes, and
 * is called from the attack refresh in morph.c rather than from here; it is
 * in this file because the round is the only thing that wants it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/box.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/model.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/talk.h>
#include <persona/common/spell.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/text.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/main/cd.h>
#include <persona/common/eventflag.h>
#include <persona/main/state.h>

/* Declared here rather than taken from persona/btlp/battle.h: hud.c defines
   it int, and the byte the call sites want is what makes the andi come out.
   The two forms are not interchangeable. */
extern char BtlHudState(void);

/* The one fight that is followed by another rather than by the field, and
   the fight it leads to. */
#define BTL_ENCOUNTER_CHAIN 0x10
#define BTL_ENCOUNTER_NEXT  0x22

/* Above this the fight is one of the scripted ones. They are never cut short
   by the debug switch, and the two below keep their music playing. */
#define BTL_ENCOUNTER_SCRIPTED 0x22
#define BTL_ENCOUNTER_KEEP_BGM_A 0x1F
#define BTL_ENCOUNTER_KEEP_BGM_B 0x21

/* The two the party is only being placed against, which the results are not
   run for. */
#define BTL_ENCOUNTER_PLACE_A 0x20
#define BTL_ENCOUNTER_PLACE_B 0x21

/* The sound slots the battle gives back on its way out. */
#define BTL_SLOT_SCENE 5
#define BTL_SLOT_VOICE 2
#define BTL_SLOT_HIT   1

/* What the placement hands the party for turning up. */
#define BTL_PLACE_GIFT_A 0xA3
#define BTL_PLACE_GIFT_B 0xAF

/* Where the opening object ends up as the battle is taken down: white, at
   six steps a frame, out of the page the battle draws from. */
#define BTL_INTRO_TPAGE 0x40
#define BTL_INTRO_FADE  6

/* The three ailments the close leaves on. POISON and SICK are carried out of
   the fight and onto the field; DEAD is already in persona/btlp/actor.h. */
#define BTL_STATUS_SICK   0x10

/* The wait at the top is the whole routine's loop rather than a loop of its
   own: everything below it is the body of one `if`, and every way out of that
   body is a return. It is written that way in the image - the frame the wait
   draws sits at the very end, past the last of the work - and it is what puts
   BTL_STATUS_DOWN in a saved register before the frame is even set up. */
void BtlStageOpen(void)
{
    u_long *dest;
    int i;

    for (;;) {
        if (g_btl_step == 0 && g_btl_intro_dist[0] == BTL_INTRO_SETTLED) {
            if (g_btl_encounter < BTL_ENCOUNTER_SCRIPTED && g_btl_debug_skip != 0) {
                g_btl_stage = BTL_STAGE_CLOSE;
                g_btl_step = 0;
                return;
            }

            if (g_btl_encounter == BTL_ENCOUNTER_CHAIN) {
                g_btl_chain_battle = 1;
            }

            g_btl_won_hp = 0;
            g_btl_round = 0;
            g_btl_won_exp = 0;
            g_btl_won_unk10 = 0;
            g_btl_won_casts = 0;
            g_btl_won_money = 0;
            g_btl_drop_item = 0;

            BtlOpenEnemyEntrance();
            BtlOpenEnemyRise();
            BtlOpenEnemyWhiten();
            BtlOpenDialogue();

            if (BtlResetTalk(1) == 0) {
                if (g_btl_place_party == 0) {
                    BtlRefreshMarkers();
                }

                /* The negotiation reads the same run of banks as it opens,
                   and whichever got there first is the one that pays. */
                if (g_btl_talk_sounds_loaded == 0) {
                    dest = BTL_VOICE_BANKS;
                    i = 0;
                    do {
                        if (g_btl_actors[i].c.key != 0
                            && (signed char)g_btl_actors[i].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                            BtlLoadSound(dest, g_btl_actors[i].c.key);
                            dest = (u_long *)((int)dest
                                              + BTL_VOICE_BANK_BYTES);
                        }
                        i++;
                    } while (i < BTL_PARTY);

                    i = BTL_VOICE_SLOT_FIRST;
                    do {
                        if (g_btl_slot_owner[i] >= 0) {
                            BtlLoadSlotSound(dest, i);
                            dest = (u_long *)((int)dest
                                              + BTL_VOICE_BANK_BYTES);
                        }
                        i += BTL_VOICE_SLOT_STEP;
                    } while (i < BTL_VOICE_SLOT_LAST);
                }

                BtlCloseMessage(0);

                /* Nothing to order about: the fight either runs itself or is
                   only a placement, so the picker is skipped and the round
                   follows straight on. */
                if (g_btl_battle_kind == 2 || g_btl_place_party != 0) {
                    g_btl_stage++;
                } else {
                    BtlPickRefresh();
                    BtlShowReadyMarkers();
                }
            }

            BtlPackEnemyGrid();
            while (BtlActorsIdle() == 0) {
                BtlDrawFrame();
            }

            g_btl_stage++;
            g_btl_step = 0;
            return;
        }

        BtlDrawFrame();
    }
}

/* Same shape as BtlStageOpen: the tail of the loop is the frame, and every
   step that is not finished simply falls out of the switch to draw one. Only
   the last step returns, and it is the one that hands the stage on - past the
   end of the table, which is what ends the battle. */
void BtlStageClose(void)
{
    BtlObj *obj;
    int m;
    int i;
    int n;
    int prev;

    /* Whatever was won belongs to the save now. Written as two indexed
       arrays rather than as two walking pointers, which is the same code in
       the end - gcc strength-reduces it to the pointers itself. The reason
       it has to be written this way is where the counter's zero lands: set
       by hand it is emitted after both pointers, and left to strength
       reduction it is emitted before them, which is the order the image has.
       It borrows the actor walk's counter below for the same reason the
       image does - one counter, used twice, never live across a call. */
    m = 0;
    do {
        EVENT_FLAGS_BANK2[m] |= g_btl_persona_won[m];
        m++;
    } while (m < EVENT_FLAG_BANK_WORDS);

    /* Turning up on the field for the first time is paid for in items. */
    if (g_btl_place_party != 0 && g_btl_encounter == 0) {
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < 60);
        BtlOpenMessage(0, 0, g_btl_msg_place1, 0x10, 0xBC);
        while (g_btl_pad1_edge == 0) {
            BtlDrawFrame();
        }
        BtlTextSetState(5, 0, 1);
        BtlTextWaitDone();
        BtlOpenMessage(0, 0, g_btl_msg_place2, 0x10, 0xBC);
        while (g_btl_pad1_edge == 0) {
            BtlDrawFrame();
        }
        BtlGiveItem(BTL_PLACE_GIFT_A);
        BtlGiveItem(BTL_PLACE_GIFT_B);
    }

    g_btl_delay = 0;
    BtlCloseMessage(0);

    for (;;) {
        switch (g_btl_step) {
        case 0:
            if (BtlHudState() == 0 && BtlBoxState() == 0) {
                if (g_btl_party_lost == 0) {
                    /* A placement is not a fight, so nothing is tallied for
                       it and nobody is picked up off the floor. */
                    if (g_btl_chain_battle == 0
                        && (u_short)g_btl_encounter != BTL_ENCOUNTER_PLACE_A
                        && (u_short)g_btl_encounter != BTL_ENCOUNTER_PLACE_B) {
                        m = 0;
                        n = 0;
                        do {
                            if (g_btl_actors[n].c.key != 0) {
                                /* POISON stands on its own, and SICK and DOWN
                                   fold into one range test - a three-case
                                   switch walks all three instead. All three
                                   read the field itself, not a local: the fold
                                   then keeps the byte in a saved copy of its
                                   own, which is the image's `addu v1,v0,zero`,
                                   and that copy's slot is the eight bytes of
                                   frame no local accounts for. Taken into a
                                   local first, the copy and the eight bytes
                                   both go. */
                                if ((signed char)g_btl_actors[n].c.status != BTL_STATUS_POISON
                                    && (signed char)g_btl_actors[n].c.status != BTL_STATUS_SICK
                                    && (signed char)g_btl_actors[n].c.status != BTL_STATUS_DOWN) {
                                    g_btl_actors[n].c.status = 0;
                                    g_btl_actors[n].c.ail_level = 0;
                                }
                                if (g_btl_actors[n].c.hp < 1) {
                                    g_btl_actors[n].c.hp = 1;
                                }
                            }
                            m++;
                            n++;
                        } while (m < BTL_PARTY);
                        BtlBattleResults();
                        BtlLevelUpParty();
                        BtlRestoreField();
                    }

                    BtlStoreParty();
                    BtlStorePersonas();
                    BtlSoundClose(BTL_SLOT_SCENE);
                    BtlSoundClose(BTL_SLOT_VOICE);
                    BtlSoundClose(BTL_SLOT_HIT);

                    if (SsIsEos(g_btl_seq[0], 0) == 0
                        || g_btl_bgm_kinds[g_btl_bgm_index]
                               == BTL_BGM_KIND_KEEP
                        || g_btl_encounter == BTL_ENCOUNTER_KEEP_BGM_A
                        || g_btl_encounter == BTL_ENCOUNTER_KEEP_BGM_B) {
                        g_btl_intro_bgm_done = 1;
                    } else {
                        SsSetMarkCallback(g_btl_seq[0], 0, BtlIntroBgmMark);
                        g_btl_intro_bgm_done = 0;
                    }

                    if (g_btl_debug_hud != 0) {
                        g_moon = g_btl_moon;
                    }
                    g_btl_closing = 1;

                    if (g_btl_chain_battle == 0) {
                        prev = g_state_prev;
                        switch (prev) {
                        case GAME_STATE_DNG:
                            PreloadDng();
                            g_state_next = GAME_STATE_DNG;
                            break;
                        case GAME_STATE_S2D:
                            PreloadS2d();
                            g_state_next = prev;
                            break;
                        default:
                            BtlSetReturnMap();
                            PreloadAdv();
                            g_state_next = GAME_STATE_ADV;
                        }
                    } else if (g_btl_encounter == BTL_ENCOUNTER_CHAIN) {
                        g_btl_encounter = BTL_ENCOUNTER_NEXT;
                        g_map_id = BTL_ENCOUNTER_NEXT;
                        PreloadBtlField();
                        g_state_next = GAME_STATE_BTL;
                    } else {
                        BtlSetReturnMap();
                        PreloadAdv();
                        g_state_next = GAME_STATE_ADV;
                    }
                } else {
                    if (SsIsEos(g_btl_seq[0], 0) == 0
                        || g_btl_bgm_kinds[g_btl_bgm_index]
                               == BTL_BGM_KIND_KEEP
                        || g_btl_encounter == BTL_ENCOUNTER_KEEP_BGM_A
                        || g_btl_encounter == BTL_ENCOUNTER_KEEP_BGM_B) {
                        g_btl_intro_bgm_done = 1;
                    } else {
                        SsSetMarkCallback(g_btl_seq[0], 0, BtlIntroBgmMark);
                        g_btl_intro_bgm_done = 0;
                    }

                    /* Whoever was leading gets their name in the line that
                       says the party is finished. */
                    BtlSetInsert(4, g_btl_actors[0].c.name);
                    BtlOpenMessage(0, 0, g_btl_msg_defeat, 0x10, 0x94);
                    while (g_btl_pad1_edge == 0) {
                        BtlDrawFrame();
                    }
                    BtlCloseMessage(0);
                    BtlSoundClose(BTL_SLOT_SCENE);
                    BtlSoundClose(BTL_SLOT_VOICE);
                    BtlSoundClose(BTL_SLOT_HIT);
                    g_state_next = GAME_STATE_NONE;
                }
                g_btl_step++;
            }
            break;

        case 1:
            /* Once the music is done with, the opening object is taken back
               down: white, full brightness, and no offset. */
            if (g_btl_intro_bgm_done != 0) {
                BtlRetractMarkers();
                /* Taken into a local first: the page below is a global too,
                   and storing to it would otherwise make the object be read
                   again for every field. */
                obj = g_btl_intro_obj;
                g_btl_tpage[0] = BTL_INTRO_TPAGE;
                obj->rgb_to[0] = 0xFF;
                obj->rgb_to[1] = 0xFF;
                obj->rgb_to[2] = 0xFF;
                obj->fade = BTL_INTRO_FADE;
                obj->rgb[0] = 0;
                obj->rgb[1] = 0;
                obj->rgb[2] = 0;
                obj->attr &= ~BTL_OBJ_HIDDEN;
                g_btl_step++;
            }
            break;

        case 2:
            if (g_btl_intro_obj->rgb[0] == 0xFF) {
                BtlSoundClose(0);
                VSyncCallback(NULL);
                g_btl_stage++;
                return;
            }
            break;
        }

        BtlDrawFrame();
    }
}

/* The round itself, and the largest routine in the overlay.
 *
 * One loop and one switch, and the step it switches on is the only thing that
 * says where the round has got to - which is why so much of what a step works
 * out is kept in locals that outlive it. The fighter whose turn it is, the
 * object drawn for it and the action it settled on are set up in one step and
 * read several steps later.
 *
 * Steps three and eight share a body: the first is a member's turn playing
 * out, the second an enemy's, and once the script is running they are the
 * same wait.
 *
 * Every local is declared up here and the counters are shared between the
 * steps: `i` is every walk's index, `j` every frame wait and the fighter the
 * stat walk starts from, and `n` a count, a flag and a sound slot in turn.
 * gcc 2.6 gives one variable one register for its whole life, and only shared
 * this way do the image's saved registers come out. The declaration order is
 * the frame's: the four that never get a register take their stack slots in
 * the order they are declared here.
 */

/* See step one: a taken-over turn saves its targets through this. */
typedef struct {
    u_long value;
} BtlLongCell;

void BtlStageRound(void)
{
    /* Sixteen bytes the image reserves and never writes. */
    SVECTOR unused[2];
    int slowest;
    int member;
    int cancelled;
    BtlActor *actor;
    BtlObj *obj;
    u_char action;
    int i;
    int j;
    int n;
    int last;
    int kind;
    int amount;

    cancelled = 0;
    for (;;) {
        if ((g_btl_pad1 & g_btl_key_cancel) != 0) {
            cancelled = 1;
        }

        switch (g_btl_step) {
        case 0: {
            int r;
            int bits;
            BtlStats *p;

            /* Whose stats are worked out again: one side, or both. Written as
               a switch, which is what the image tests - it jumps to each arm
               rather than branching past it. */
            switch (g_btl_battle_kind) {
            case 1:
                j = 0;
                last = BTL_PARTY;
                break;

            case 2:
                j = BTL_PARTY;
                last = BTL_ACTORS;
                break;

            default:
                j = 0;
                last = BTL_ACTORS;
                break;
            }
            g_btl_battle_kind = 0;
            if (g_btl_debug_party_only != 0) {
                g_btl_battle_kind = 1;
            }

            if (SsIsEos(g_btl_seq[0], 0) == 0) {
                if (g_btl_bgm_kinds[g_btl_bgm_index] == BTL_BGM_KIND_STOP) {
                    SsSepStop(g_btl_seq[0], 2);
                }
                BtlSePlay(0, 0);
            }

            if (g_btl_place_party != 0) {
                n = BtlResetTurnOrder();
            } else {
                n = 0;
                for (i = j; i < last; i++) {
                    if (g_btl_actors[i].c.key != 0
                        && (signed char)g_btl_actors[i].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                        BtlDeriveBattleStats(&g_btl_actors[i]);
                        g_btl_turn_order[n] = i;
                        n++;
                        /* Mostly the third stat, sometimes the fourth, and on
                           a rare roll both added. */
                        r = rand();
                        if ((u_int)(r & 0xFF) < 0xCE) {
                            g_btl_actors[i].initiative = g_btl_actors[i].stat[3];
                        } else if ((u_int)(r & 0xFF) < 0xEF) {
                            g_btl_actors[i].initiative = g_btl_actors[i].stat[4];
                        } else {
                            g_btl_actors[i].initiative
                                = g_btl_actors[i].stat[3]
                                  + g_btl_actors[i].stat[4];
                        }
                        g_btl_actors[i].action = 0xFF;
                    }
                }
                n = BtlOrderTurns(g_btl_turn_order, n);
            }

            BtlAverageSides();
            g_btl_pack_ready = 0;
            g_btl_turn = 0;
            g_btl_turns = n;
            while (BtlMarkersHidden() == 0) {
                BtlDrawFrame();
            }

            /* Whether a Persona takes the turn away from its owner. */
            g_btl_act_kind = 0;
            g_btl_act_actor = 0;
            g_btl_step++;
            bzero(g_btl_debug_grid_cells, 0x4B);
            i = 0;
            n = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                    p = &g_btl_personas[BtlActorPersona(i)];
                    bits = (g_persona_defs[p->key].bond
                            >> ((g_btl_actors[i].c.key - 1)
                                * PERSONA_BOND_BITS))
                           & PERSONA_BOND_MASK;
                    if (p->slots > 5 && g_btl_actors[i].c.blocked == 0
                        && bits == PERSONA_BOND_FULL) {
                        kind = 0;
                        switch (p->unk41 & 0xF0) {
                        case 0x10:
                            if (g_btl_actors[i].c.hp_max / 16
                                >= g_btl_actors[i].c.hp) {
                                kind = 4;
                            } else if (g_btl_actors[i].c.hp_max / 8
                                       >= g_btl_actors[i].c.hp) {
                                kind = 8;
                            } else if (g_btl_actors[i].c.hp_max / 4
                                       >= g_btl_actors[i].c.hp) {
                                kind = 0x10;
                            }
                            if (g_btl_debug_act_kind1 != 0) {
                                kind = 1;
                            }
                            if ((signed char)g_btl_actors[i].c.status
                                    != BTL_STATUS_NOINPUT
                                && kind != 0
                                && (rand() & (kind - 1)) == 0) {
                                n = 1;
                                g_btl_act_kind = 1;
                                g_btl_act_actor = i;
                            }
                            break;

                        case 0x30:
                            if (g_btl_actors[i].c.hp_max / 16
                                >= g_btl_actors[i].c.hp) {
                                kind = 4;
                            } else if (g_btl_actors[i].c.hp_max / 8
                                       >= g_btl_actors[i].c.hp) {
                                kind = 8;
                            }
                            if (g_btl_debug_act_kind2 != 0) {
                                kind = 1;
                            }
                            if ((signed char)g_btl_actors[i].c.status
                                    != BTL_STATUS_NOINPUT
                                && kind != 0
                                && (rand() & (kind - 1)) == 0) {
                                n = 1;
                                g_btl_act_kind = 2;
                                g_btl_act_actor = i;
                            }
                            break;
                        }
                    }
                }
                i++;
            } while (i < BTL_PARTY && n == 0);

            if (g_btl_place_party != 0) {
                g_btl_act_kind = 0;
            }
            g_btl_round++;
            /* and straight on into the turn it just settled */
        }
        case 1: {
            short slot;
            int t;
            u_char *scripts;
            u_char *line;

            if (g_cd_busy == -1) {
                BtlShowAilmentMarks(0);
                if (BtlAnyStanding() == 0
                    || (g_btl_debug_hud != 0
                        && (g_btl_pad1 & g_btl_key_end) != 0)
                    || BtlBattleOutcome() == 0) {
                    g_btl_step = 6;
                } else if ((signed char)g_btl_turn < g_btl_turns) {
                    /* Whose turn this is, and how it came to be theirs. */
                    switch (g_btl_act_kind) {
                    case 0:
                        g_btl_actor_turn = g_btl_turn_order[g_btl_turn];
                        actor = &g_btl_actors[g_btl_actor_turn];
                        member = actor->c.key;
                        obj = actor->obj;
                        break;

                    case 1:
                        slot = g_btl_act_actor;
                        actor = &g_btl_actors[slot];
                        g_btl_actor_turn = slot;
                        member = actor->c.key;
                        actor->action = 6;
                        obj = actor->obj;
                        actor->flags |= 0x10000000;
                        BtlSetPickable();
                        g_btl_act_move = actor->move;
                        g_btl_act_speed = actor->order;
                        g_btl_act_targets = actor->targets;
                        actor->order = BtlSlowestOrder() + 5;
                        actor->targets = BtlPickableMask();
                        actor->move = 0x6E;
                        break;

                    case 2:
                        /* The owner's turn is put aside and the Persona's move
                           turned on the fighter itself. Two details are
                           load-bearing. The saved targets go through a struct:
                           gcc's scheduler lets a store to a struct field sink
                           past stores to plain globals, and only a global it
                           takes for a structure pins the flags update above
                           the saves, where the image has it. And the targets
                           are worked out before the move is set, which reads
                           the turn ahead of the move's store. */
                        slot = g_btl_act_actor;
                        actor = &g_btl_actors[slot];
                        g_btl_actor_turn = slot;
                        member = actor->c.key;
                        obj = actor->obj;
                        actor->action = 6;
                        g_btl_act_move = actor->move;
                        actor->flags |= 0x10000000;
                        g_btl_act_speed = actor->order;
                        ((BtlLongCell *)&g_btl_act_targets)->value
                            = actor->targets;
                        actor->order = g_btl_actor_turn;
                        actor->targets = 1 << g_btl_actor_turn;
                        actor->move = 0x61;
                        break;

                    case 3:
                        /* Step two again, with the Persona's other move. */
                        slot = g_btl_act_actor;
                        actor = &g_btl_actors[slot];
                        g_btl_actor_turn = slot;
                        member = actor->c.key;
                        obj = actor->obj;
                        actor->action = 6;
                        g_btl_act_move = actor->move;
                        actor->flags |= 0x10000000;
                        g_btl_act_speed = actor->order;
                        ((BtlLongCell *)&g_btl_act_targets)->value
                            = actor->targets;
                        actor->order = g_btl_actor_turn;
                        actor->targets = 1 << g_btl_actor_turn;
                        actor->move = 0x6C;
                        break;
                    }

                    if (g_btl_act_kind != 0
                        || (g_btl_actors[g_btl_actor_turn].c.key != 0
                            && (signed char)g_btl_actors[g_btl_actor_turn]
                                       .c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[g_btl_actor_turn].flags
                                & BTL_ACTOR_OUT)
                                   == 0)) {
                        BtlDeriveBattleStats(actor);
                        if (g_btl_actor_turn < BTL_PARTY) {
                            if (g_btl_act_kind != 0
                                || (actor->flags & 0x22000) == 0) {
                                actor->pickable = 1;
                                scripts = &g_btl_member_scripts
                                              [member * MEMBER_SCRIPT_MODEL];
                                if (g_btl_act_kind == 0) {
                                    g_btl_marker_obj[g_btl_actor_turn]->attr
                                        |= BTL_MARK_CHOSEN;
                                    if ((signed char)actor->c.status != 0) {
                                        obj->mark->attr &= ~BTL_OBJ_HIDDEN;
                                    }
                                }
                                if (g_btl_act_kind != 0) {
                                    BtlReadMemberBank(
                                        1,
                                        g_btl_actors[g_btl_actor_turn].c.key);
                                } else if (g_btl_pack_ready == 0) {
                                    BtlReadyTurnNow();
                                }
                                action = actor->action;
                                switch (action) {
                                case 2:
                                    if (actor->counter != 0
                                        && g_btl_msg_speed != 2) {
                                        BtlOpenMessage(1, 1, g_btl_msg_ailment,
                                                       0x10, 0xC);
                                        g_btl_msg_timer
                                            = g_btl_msg_speed == 0 ? 0xB4
                                                                   : 0x1E;
                                    }
                                    if (actor->script_pick == 2) {
                                        BtlReloadMemberGfx(member,
                                                           g_btl_actor_turn);
                                        if (actor->c.equip[0] != 0
                                            && actor->c.equip[0] != 0xA3
                                            && actor->c.equip[0] != 0xAF) {
                                            actor->script_pick = 1;
                                        } else {
                                            actor->script_pick = 0;
                                        }
                                        BtlObjSetScript(
                                            obj,
                                            (BtlSeqStep *)obj->scripts
                                                [scripts[actor->script_pick
                                                         * MEMBER_SCRIPT_PICK]]);
                                    }
                                    BtlOpenMemberBank();
                                    break;

                                case 0xC:
                                    if (actor->script_pick != 2) {
                                        BtlReloadMemberGfx(member + 10,
                                                           g_btl_actor_turn);
                                        actor->script_pick = 2;
                                        BtlObjSetScript(
                                            obj,
                                            (BtlSeqStep *)obj->scripts
                                                [scripts[2 * MEMBER_SCRIPT_PICK]]);
                                    }
                                    obj->children = obj->kind == 3 ? 0x24 : 0x22;
                                case 6:
                                open_bank:
                                    BtlOpenMemberBank();
                                    break;

                                case 3:
                                    if (g_btl_place_party == 0
                                        && g_btl_msg_speed != 2) {
                                        line = g_btl_ailment_lines[actor->ail_line];
                                        BtlOpenMessage(5, 1, line, 0x10, 0xC);
                                        g_btl_msg_timer
                                            = g_btl_msg_speed == 0 ? 0xB4
                                                                   : 0x1E;
                                    }
                                    /* Shares step six's bank call rather than
                                       repeating it: that copy is the one the
                                       image keeps. */
                                    goto open_bank;

                                case 0xE:
                                    break;
                                }
                                g_btl_step++;
                            } else {
                                g_btl_step = 3;
                            }
                        } else {
                            actor->pickable = 1;
                            if ((signed char)actor->c.status != 0) {
                                obj->mark->attr &= ~BTL_OBJ_HIDDEN;
                            }
                            if (g_btl_pack_ready == 0) {
                                BtlReadyTurnNow();
                            }
                            action = actor->action;
                            switch (obj->kind) {
                            case 0xB1:
                                obj->children = 0x15;
                                break;
                            case 0x95:
                            case 0x96:
                            case 0xBA:
                                obj->children = 0x10;
                                break;
                            case 0x97:
                                obj->children = 0xE;
                                break;
                            case 0xB5:
                                obj->children = 0xD;
                                break;
                            }
                            switch (action) {
                            case 4:
                                actor->flags |= 0x2000;
                                break;
                            case 0xE:
                                BtlSoundOpen(g_btl_slot_banks, 6,
                                             (actor->obj->tpage >> 1) - 5);
                                break;
                            case 2:
                                actor->move = 0;
                                actor->turn_move = 0;
                                BtlPersonaGuard(actor);
                                BtlOpenPackBank();
                                break;
                            case 6:
                                actor->turn_move = actor->move;
                                actor->turn_slot = actor->obj->spell_slot;
                                if (g_btl_place_party == 0) {
                                    BtlAimMove(actor);
                                }
                                BtlPersonaGuard(actor);
                                BtlOpenPackBank();
                                if (g_btl_place_party != 0) {
                                    actor->action = 0xFF;
                                }
                                break;
                            case 5:
                                BtlOpenPackBank();
                                break;
                            }
                            g_btl_step++;
                        }
                    } else {
                        g_btl_step = 1;
                        g_btl_turn++;
                    }
                } else {
                    g_btl_delay = 0;
                    g_btl_step = 4;
                }
            }
            break;
        }
        case 2: {
            if (g_btl_delay == 0 && SsVabTransCompleted(0) != 0) {
                if (g_btl_actor_turn < BTL_PARTY) {
                    obj->motion = action;
                    g_btl_se_off = 0;
                } else {
                    obj->motion = action;
                }
                action = obj->motion;
                g_btl_step++;
            }
            break;
        }
        case 3:
        case 8: {
            int slot;

            BtlHoldForMarkers();
            if (obj->motion == 0
                && (obj->attr & BTL_OBJ_BUSY_MASK) != BTL_OBJ_BUSY) {
                BtlDrawFrame();
                BtlDrawFrame();
                g_btl_actors[g_btl_actor_turn].unkCC = 0;
                g_btl_actors[g_btl_actor_turn].counter = 0;
                if (g_btl_effect_obj != NULL) {
                    BtlObjSetMotion(g_btl_effect_obj, 4);
                    BtlObjSetPhase(g_btl_effect_obj, 0);
                    j = 0;
                    g_btl_effect_obj = NULL;
                    g_btl_scene_rgb[0] = 0x80;
                    g_btl_scene_rgb[1] = 0x80;
                    g_btl_scene_rgb[2] = 0x80;
                    do {
                        j++;
                        BtlDrawFrame();
                    } while (j < 0x78);
                }

                switch ((short)(g_btl_encounter - 2)) {
                case 0:
                    if (obj->kind == 2 && g_btl_scene_pending != 0) {
                        BtlPlayScene(0, g_btl_line_enc02);
                        g_btl_scene_pending = 0;
                    }
                    break;
                case 1:
                    if (obj->kind == 1) {
                        BtlPlayScene(3, g_btl_line_enc03a);
                    }
                    if (obj->kind == 4) {
                        BtlPlayScene(3, g_btl_line_enc03b);
                        j = 0;
                        do {
                            j++;
                            BtlDrawFrame();
                        } while (j < 60);
                        BtlPlayScene(0, g_btl_line_enc03c);
                    }
                    if (obj->kind == 0x6E) {
                        BtlPlayScene(3, g_btl_line_enc03d);
                    }
                    break;
                case 2:
                    if (obj->mark_num == 6) {
                        BtlPlayScene(7, g_btl_line_enc04);
                    }
                    break;
                case 3:
                    if (g_btl_place_party != 0) {
                        if ((signed char)g_btl_turn == 0) {
                            BtlPlayScene(0x27, g_btl_line_enc05a);
                            j = 0;
                            do {
                                j++;
                                BtlDrawFrame();
                            } while (j < 60);
                            BtlPlayScene(5, g_btl_line_enc05b);
                        }
                        if ((signed char)g_btl_turn == 1) {
                            BtlPlayScene(0x27, g_btl_line_enc05c);
                            j = 0;
                            do {
                                j++;
                                BtlDrawFrame();
                            } while (j < 60);
                            BtlPlayScene(0x29, g_btl_line_enc05d);
                            j = 0;
                            do {
                                j++;
                                BtlDrawFrame();
                            } while (j < 60);
                            BtlPlayScene(4, g_btl_line_enc05e);
                            j = 0;
                            do {
                                j++;
                                BtlDrawFrame();
                            } while (j < 60);
                            BtlPlayScene(0x29, g_btl_line_enc05f);
                        }
                        if ((signed char)g_btl_turn == 2) {
                            BtlPlayScene(0x29, g_btl_line_enc05g);
                            goto placed;
                        }
                    }
                    break;
                case 6:
                    if (g_btl_place_party != 0) {
                        if ((signed char)g_btl_turn == 0) {
                            BtlPlayScene(4, g_btl_line_enc08);
                        }
                    placed:
                        g_btl_place_party = 0;
                        BtlRefreshMarkers();
                    }
                    break;
                case 15:
                case 16:
                    if ((signed char)g_btl_turn == 1) {
                        BtlPlayScene(8, g_btl_line_enc11a);
                    }
                    if ((signed char)g_btl_turn == 4) {
                        BtlPlayScene(7, g_btl_line_enc11b);
                    }
                    if (g_btl_encounter == 0x11 && (signed char)g_btl_turn == 5
                        && (BtlPlayScene(3, g_btl_line_enc11c),
                            g_btl_encounter == 0x12)
                        && (signed char)g_btl_turn == 6) {
                        BtlPlayScene(7, g_btl_line_enc11d);
                    }
                }

                slot = g_btl_actor_turn;
                if (slot < BTL_PARTY) {
                    g_btl_marker_obj[slot]->attr &= ~BTL_MARK_CHOSEN;
                    if (actor->marker != 0 && (actor->flags & 0x2000) == 0
                        && g_btl_act_kind == 0 && actor->unkD8 == 0) {
                        BtlShowMarker(slot, 0, actor->c.unk5D & 0xF);
                        actor->marker = 0;
                    }
                    actor->unkD8 = 0;
                }
                BtlBuildMarkers();
                BtlPartyResetGfx();
                if (g_btl_encounter != 0xF) {
                    BtlEnemiesResetGfx();
                }
                BtlSoundClose(6);
                BtlSoundClose(4);
                if (action != 0 && action != 4 && action != 0xE) {
                    BtlSoundClose(3);
                }
                if (g_btl_act_kind == 0) {
                    g_btl_turn++;
                } else {
                    g_btl_actors[g_btl_act_actor].flags &= ~0x10000000;
                }
                if (g_btl_act_kind != 3) {
                    g_btl_act_kind = 0;
                }
                if (g_btl_encounter == 7) {
                    g_btl_step = 6;
                } else {
                    g_btl_step = 1;
                }
            } else if (g_btl_scene_wanted != 0) {
                BtlObjSetAttr(g_btl_hud_obj, BTL_OBJ_HIDDEN);
                j = 0;
                BtlHudLoad();
                BtlHudShow();
                do {
                    j++;
                    BtlDrawFrame();
                } while (j < 60);
                BtlSeqPlay(g_btl_seq_hud_up);
                BtlSeqRun();
                BtlHudHide();
                g_btl_scene_hud = 0;
                g_btl_scene_wanted = 0;
            }
            break;
        }
        case 4: {
            int slot;
            int t;
            BtlObj **objp;

            if (g_btl_delay != 0 || BtlMarkersHidden() == 0) {
                break;
            }
            /* The two fights whose boss leaves the field rather than dying. */
            if ((g_btl_encounter == 0x1D && g_btl_enemies[6].c.key != 0x9A)
                || (g_btl_encounter == 0x1E
                    && g_btl_enemies[6].c.key != 0x9B)) {
                BtlSoundOpen(g_btl_slot_banks, 6,
                             (g_btl_enemies[0].obj->tpage >> 1) - 5);
                i = BTL_PARTY;
                SsVabTransCompleted(1);
                slot = BTL_PARTY;
                do {
                    i++;
                    if (g_btl_actors[slot].c.key != 0
                        && (signed char)g_btl_actors[slot].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                        g_btl_actors[slot].c.hp = 0;
                        /* The enemy's object through a pointer of its own,
                           so the walk has one induction variable and steps
                           its two addresses in the image's order. */
                        objp = &g_btl_enemies[slot - BTL_PARTY].obj;
                        (*objp)->motion = 8;
                        (*objp)->phase = 0;
                    }
                    slot++;
                } while (i < 0xB);
                j = 0;
                do {
                    j++;
                    BtlDrawFrame();
                } while (j < 60);
                BtlSoundClose(6);
            }

            /* Whoever is lowest is singled out. */
            i = 0;
            if (g_btl_pick_slowest != 0) {
                n = 100;
                do {
                    if (g_btl_combatants[i].c.key != 0
                        && g_btl_combatants[i].c.level < n) {
                        n = g_btl_combatants[i].c.level;
                        slowest = i;
                    }
                    i++;
                } while (i < BTL_ENEMIES);
                if ((g_btl_combatants[slowest].flags & 0x40) != 0) {
                    g_btl_combatants[slowest].unkDE = 1;
                    g_btl_combatants[slowest].unkD4 = 0;
                } else {
                    g_btl_combatants[slowest].unkD4 = 1;
                }
            }

            i = 0;
            g_btl_actor_turn = -1;
            n = 7;
            /* Walked by hand beside the index: the liveness tests ask of the
               index, everything else of the pointer. */
            actor = g_btl_actors;
            do {
                /* A palsy drains a share of the whole reserve. The two tests
                   below each ask whether the record is used again rather than
                   sharing one - which is what the image does. */
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                    && (signed char)actor->c.status == 0x0E) {
                    int sp;
                    int clamped;

                    sp = actor->c.sp - actor->c.sp_max / 16;
                    actor->c.sp = sp;
                    if (sp >= 0) {
                        clamped = sp;
                        if (clamped > 999) {
                            clamped = 999;
                        }
                    } else {
                        clamped = 0;
                    }
                    actor->c.sp = clamped;
                }

                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                    && (actor->unkD4 != 0
                        || (signed char)actor->c.status == 0x0D
                        || (signed char)actor->c.status == 0x10
                        || (((signed char)actor->c.status == 4
                             || (signed char)actor->c.status == 5)
                            && (signed char)actor->c.ail_level == CHAR_AIL_LEVEL_MAX)
                        || (actor->flags & 0x34C0000) != 0)) {
                    if (i < BTL_PARTY) {
                        if (n < 0xC) {
                            BtlSoundOpen(g_btl_banks, n, actor->c.key);
                        }
                    } else if (n < 0xC) {
                        BtlSoundOpen(g_btl_slot_banks, n,
                                     (actor->obj->tpage >> 1) - 5);
                    }
                    SsVabTransCompleted(1);

                    /* Tested the way the image tests it: each in turn, and the
                       first that is set wins. */
                    if (actor->unkD4 != 0) {
                        amount = actor->c.hp;
                        kind = 7;
                    } else if ((actor->flags & 0x400000) != 0) {
                        kind = 7;
                        amount = actor->c.hp / 2;
                    } else if ((actor->flags & 0x40000) != 0) {
                        switch (actor->persona_rank) {
                        case 0:
                        case 1:
                        case 2:
                            amount = (actor->unkCF != 0 ? actor->c.hp_max
                                                        : actor->c.hp) / 8;
                            kind = 7;
                            break;
                        case 4:
                            amount = (actor->unkCF != 0 ? actor->c.hp_max
                                                        : actor->c.hp) / 2;
                            kind = 7;
                            break;
                        case 3:
                            kind = 0;
                            if (actor->unkCF == 0) {
                                kind = 0x0F;
                                amount = actor->c.hp / 8;
                            }
                            break;
                        }
                    } else if ((actor->flags & 0x80000) != 0) {
                        /* The same three ranks, the other way up. */
                        switch (actor->persona_rank) {
                        case 0:
                        case 1:
                        case 2:
                            amount = (actor->unkCF != 0 ? actor->c.hp_max
                                                        : actor->c.hp) / 8;
                            kind = 7;
                            break;
                        case 3:
                            amount = (actor->unkCF != 0 ? actor->c.hp_max
                                                        : actor->c.hp) / 2;
                            kind = 7;
                            break;
                        case 4:
                            kind = 0;
                            if (actor->unkCF == 0) {
                                kind = 0x0F;
                                amount = actor->c.hp / 8;
                            }
                            break;
                        }
                    } else {
                        switch ((signed char)actor->c.status) {
                        case 0x10:
                            amount = actor->c.hp_max / 8;
                            kind = 7;
                            break;
                        case 4:
                            if ((signed char)actor->c.ail_level == CHAR_AIL_LEVEL_MAX) {
                                kind = 7;
                                amount = actor->c.hp_max / 16;
                            }
                            break;
                        case 5:
                            if ((signed char)actor->c.ail_level == CHAR_AIL_LEVEL_MAX) {
                                kind = 7;
                                amount = actor->c.hp_max / 16;
                            }
                            break;
                        case 0x0D:
                            kind = 7;
                            amount = actor->c.hp_max / 16;
                            break;
                        default:
                            if ((actor->flags & BTL_ACTOR_WOUND) != 0) {
                                t = actor->wound + 1;
                                actor->wound = t;
                                if (actor->wound != 0) {
                                    if ((u_char)t > 0x7F) {
                                        t = 0x7F;
                                        actor->wound = t;
                                    }
                                } else {
                                    t = 1;
                                    actor->wound = t;
                                }
                                amount = actor->wound;
                                kind = 7;
                            } else {
                                kind = 0;
                                if ((actor->flags & BTL_ACTOR_F5) != 0) {
                                    kind = 0x0F;
                                    amount = actor->c.hp_max / 8;
                                }
                            }
                            break;
                        }
                    }

                    switch (kind) {
                    case 0x0F:
                        BtlSePlay(2, 0xB);
                        actor->hit_amount = amount;
                        actor->c.hp = amount + actor->c.hp;
                        t = actor->c.hp;
                        if (actor->c.hp_max < t) {
                            t = actor->c.hp_max;
                        }
                        actor->c.hp = t;
                        actor->obj->motion = 0x0F;
                        break;
                    case 7:
                        if (amount != 0) {
                            actor->c.hp -= amount;
                            actor->hit_amount = amount;
                            if (actor->c.hp < 1) {
                                actor->c.hp = 0;
                                actor->obj->motion = 8;
                                if (n < 0xC) {
                                    BtlSePlay(n, 1);
                                }
                            } else {
                                actor->obj->motion = 7;
                                actor->obj->children = 0;
                                if (n < 0xC) {
                                    BtlSePlay(n, 0);
                                }
                            }
                        }
                        break;
                    }
                    n++;
                    BtlDrawFrame();
                    BtlDrawFrame();
                    BtlDrawFrame();
                    BtlDrawFrame();
                }
                i++;
                actor++;
            } while (i < BTL_ACTORS);

            g_btl_delay = 0x20;
            do {
                BtlDrawFrame();
            } while (g_btl_delay != 0);

            for (i = 7; i < n; i++) {
                BtlSoundClose(i);
            }
            i = 0;
            BtlHoldForMarkers();
            do {
                g_btl_actors[i].flags &= 0xF7FC5FFF;
                i++;
            } while (i < BTL_ACTORS);

            i = 0;
            do {
                if ((g_btl_actors[i].c.key == 0
                     || (signed char)g_btl_actors[i].c.status
                            == BTL_STATUS_DOWN
                     || (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0)
                    && g_btl_actors[i].marker != 0) {
                    g_btl_actors[i].marker = 0;
                    BtlShowMarker(i, 0, g_btl_actors[i].c.unk5D & 0xF);
                }
                i++;
            } while (i < BTL_PARTY);

            /* The way out of the round. The turn and the scene step both end
               the round the same way, and share this block in the image. */
            if (BtlAnyStanding() == 0
                || (g_btl_debug_hud != 0 && (g_btl_pad1 & g_btl_key_end) != 0)
                || BtlBattleOutcome() == 0) {
                g_btl_step = 6;
                break;
            }
            if ((u_int)((u_short)g_btl_encounter - 0x1D) < 2) {
                BtlLoadPackBank(0xC5);
            }
            BtlPackEnemyGrid();
            BtlAfterTalk();
            g_btl_step++;
            break;
        }
        case 5: {
            if (BtlMarkersHidden() != 0 && BtlActorsIdle() != 0) {
                BtlSoundClose(3);
                BtlBuildMarkers();
                BtlHideMarkers();
                if (cancelled) {
                    g_btl_talk_outcome = 0;
                }
                g_btl_step = 9;
            }
            break;
        }
        case 6: {
            BtlHideMarkers();
            g_btl_step++;
            BtlRoundOverScene();
            BtlLastEnemyScene();
            break;
        }
        case 7: {
            if (BtlMarkersHidden() != 0) {
                g_btl_stage++;
                return;
            }
            break;
        }
        case 9: {
            if (BtlMarkersIdle() != 0 && BtlMarkersHidden() != 0) {
                i = 0;
                BtlBoxDismiss();
                BtlDrawFrame();
                BtlDrawFrame();
                BtlDrawFrame();
                bzero(g_btl_debug_grid_cells, 0x4B);
                g_btl_boss22_shown = 0;
                g_btl_boss20_shown = 0;
                do {
                    g_btl_actors[i].revive_mark = 0;
                    g_btl_actors[i].revive_slot = 0;
                    i++;
                } while (i < BTL_PARTY);
                BtlCountDownRound();
                BtlTalkResume();
                BtlPartyResetGfx();
                BtlEnemiesResetGfx();
                BtlShowReadyMarkers();
                /* Three ways a negotiation can leave the round, and the
                   default is the one that hands back to the menu. */
                switch (g_btl_talk_outcome) {
                case 1:
                    BtlTalkersStay();
                    g_btl_step = 0;
                    break;
                case 2:
                    BtlTalkersLeaveField();
                    g_btl_step = 0;
                    break;
                case 3:
                    BtlTalkersJoin();
                default:
                    BtlPickRefresh();
                    g_btl_stage = BTL_STAGE_COMMAND;
                    return;
                }
            }
        }
        }

        g_btl_msg_timer--;
        if ((short)g_btl_msg_timer < 0) {
            g_btl_msg_timer = 0;
            BtlCloseMessage(0);
        }
        BtlDrawFrame();
    }
}

/* One enemy's turn, chosen fresh each time it comes round.
 *
 * Three things can take the turn before the move list is even looked at: a
 * boss whose script is to change shape, standing still, and running. Only
 * then are the six spells the fighter knows tested for whether they could be
 * cast at all, and whatever survives is weighed against the mood's odds.
 *
 * The ailment mask is also a word of rodata of its own, between the round's
 * jump tables and this routine's. Nothing reads it, but the image has it, so
 * the unit defines it here - which is where gcc puts it, between the two.
 */
#define AI_SKIP_AILMENTS 0x0060C0FCUL

const u_long g_btl_ai_skip_ailments = AI_SKIP_AILMENTS;

int BtlChooseEnemyMove(BtlActor *a)
{
    SVECTOR unused;
    const u_char *odds;
    u_char (*weights)[8][8];
    BtlActor *q;
    u_char *ok;
    u_char *p;
    int chance;
    int i;
    int n;
    int live;
    int roll;
    u_char enabled;
    u_char spell;

    /* Cleared backwards, from the last of the six spells down to the plain
       attack at the front. */
    for (i = 8, p = &g_btl_move_ok[8]; i >= 0; i--, p--) {
        *p = 0;
    }

    /* The second test reads Char.status and Char.ail_level as one word, which
       is why it is written through the name that runs up to them: charmed at
       full depth and the fighter does nothing at all. */
    if (((1 << (signed char)a->c.status) & AI_SKIP_AILMENTS) == 0
        && (*(u_long *)&a->c.name[9] & 0xFFFF00) != 0x30100) {
        if (g_btl_encounter == BTL_ENCOUNTER_NEXT && g_btl_boss22_shown == 0) {
            if (g_btl_boss22_shape == 0) {
                if ((rand() & 7) == 0) {
                    g_btl_boss22_shape = 1;
                    g_btl_boss22_shown = 1;
                    a->form = 0x25;
                    return BTL_MOVE_MORPH;
                }
            } else {
                if ((rand() & 1) == 0) {
                    g_btl_boss22_shown = 1;
                    g_btl_boss22_shape = 0;
                    a->form = 0x24;
                    return BTL_MOVE_MORPH;
                }
            }
        }

        if (g_btl_encounter == BTL_ENCOUNTER_PLACE_A
            && g_btl_boss20_shown == 0) {
            if (g_btl_boss20_shape == 0) {
                if ((rand() & 1) == 0) {
                    g_btl_boss20_shape = rand() % 2 + 1;
                    g_btl_boss20_shown = 1;
                    a->form = g_btl_boss20_shape == 1 ? 0x2B : 0x2D;
                    return BTL_MOVE_MORPH;
                }
            } else {
                if ((rand() & 1) == 0) {
                    g_btl_boss20_shown = 1;
                    a->form = g_btl_boss20_shape == 1 ? 0x2C : 0x2E;
                    g_btl_boss20_shape = 0;
                    return BTL_MOVE_MORPH;
                }
            }
        }

        /* Running, and then standing still. */
        chance = g_btl_enemy_ai[a->c.key][g_btl_ai_set].flee;
        if (chance != 0 && g_btl_debug_no_flee == 0
            && g_btl_ai_set != BTL_AI_SET_TAME && g_btl_no_flee == 0) {
            if (g_btl_enemy_level - g_btl_party_level < -9) {
                chance <<= 1;
            }
            if ((rand() & 0xFF) <= chance) {
                return BTL_MOVE_FLEE;
            }
        }

        chance = g_btl_enemy_ai[a->c.key][g_btl_ai_set].idle;
        if (g_btl_debug_idle != 0
            || (chance != 0 && (rand() & 0xFF) <= chance)) {
            a->flags |= BTL_ACTOR_IDLE;
            return BTL_MOVE_IDLE;
        }

        /* The plain attack, and then the six spells. */
        if (BtlPickAiTarget(a, g_spell_data[0].target) >= 0) {
            g_btl_move_ok[0] = 1;
        }

        if ((signed char)a->c.ail_level != 2
            || (u_int)(a->c.status - 8) >= 2) {
            i = 0;
            enabled = 1;
            ok = &g_btl_move_ok[1];
            do {
                /* Held as a byte, not widened once into an int: the
                   image re-masks it at each test, which is what a u_char in
                   an int context costs. */
                spell = a->spell[i];

                if (spell == 0) {
                    goto next;
                }
                if ((spell >= 0x75 && spell < 0x8C)
                    || ((u_char)(spell + 0x5D) < 0x41 && spell != 0xDB)) {
                    if (spell == 0xE1) {
                        /* only worth calling for help when there is
                           almost none left */
                        n = 0;
                        live = 0;
                        do {
                            if (g_btl_combatants[n].c.key != 0) {
                                live++;
                            }
                            n++;
                        } while (n < BTL_ENEMIES);
                        if (live >= 2) {
                            goto next;
                        }
                        *ok = enabled;
                    } else if (spell != 0xE2
                               && BtlPickAiTarget(a, g_spell_data[spell].target)
                                      >= 0) {
                        *ok = enabled;
                    }
                    goto next;
                }

                if (spell < SPELL_FREE_FIRST
                    && a->c.sp < (int)g_spell_data[spell].cost) {
                    goto next;
                }

                if ((u_char)(spell - 0x5F) < 6) {
                    n = 0;
                    do {
                        if (g_btl_combatants[n].c.key != 0
                            && g_btl_combatants[n].c.hp
                                   < g_btl_combatants[n].c.hp_max) {
                            *ok = enabled;
                        }
                        n++;
                    } while (n < BTL_ENEMIES);
                } else if ((u_char)(spell + 0x74) < 4) {
                    n = 0;
                    do {
                        if (g_btl_combatants[n].c.key != 0
                            && (g_btl_combatants[n].flags & BTL_ACTOR_WARDS) == 0) {
                            *ok = enabled;
                        }
                        n++;
                    } while (n < BTL_ENEMIES);
                } else {
                    /* Keep successful writes inside each scan. Sharing them
                       with a goto changes gcc's saved-register allocation. */
                    switch (spell) {
                    case 0xA2:
                        if (g_btl_ai_set != BTL_AI_SET_TAME) {
                            *ok = enabled;
                        } else {
                            break;
                        }
                    case 0x56:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0) {
                                int flags = q[n].stage[0];
                                flags |= q[n].stage[1];
                                flags |= q[n].stage[2];
                                if (flags != 0) {
                                    *ok = enabled;
                                    break;
                                }
                            }
                            n++;
                        }
                    case 0x5B:
                        n = 0;
                        while (n < BTL_PARTY) {
                            if (g_btl_actors[n].c.key != 0) {
                                int flags = g_btl_actors[n].stage[3];
                                flags |= g_btl_actors[n].stage[4];
                                flags |= g_btl_actors[n].stage[5];
                                flags |= g_btl_actors[n].stage[6];
                                if (flags != 0) {
                                    *ok = enabled;
                                    break;
                                }
                            }
                            n++;
                        }
                        break;
                    case 0x5D:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0 && (q[n].flags & BTL_ACTOR_5D) == 0) {
                                *ok = enabled;
                                break;
                            }
                            n++;
                        }
                        break;
                    case 0x5E:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0 && (q[n].flags & BTL_ACTOR_5E) == 0) {
                                *ok = enabled;
                                break;
                            }
                            n++;
                        }
                        break;
                    case 0x67:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0
                                && (signed char)q[n].c.status == 0xD) {
                                *ok = enabled;
                                break;
                            }
                            n++;
                        }
                        break;
                    case 0x68:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0
                                && (signed char)q[n].c.status == 0xE) {
                                *ok = enabled;
                                break;
                            }
                            n++;
                        }
                        break;
                    case 0x69:
                        n = 0;
                        q = g_btl_combatants;
                        while (n < BTL_ENEMIES) {
                            if (q[n].c.key != 0
                                && (signed char)q[n].c.status == 0xF) {
                                *ok = enabled;
                                break;
                            }
                            n++;
                        }
                        break;
                    /* Thirteen moves the AI never has to test for. They
                       are written one to a line rather than stacked: stacked
                       they share a label, gcc folds the adjacent ones into a
                       single case node, and twelve nodes over a range of 160
                       is sparse enough that it drops the jump table for a
                       compare tree. */
                    case 0x41: break;
                    case 0x42: break;
                    case 0x43: break;
                    case 0x44: break;
                    case 0x45: break;
                    case 0x46: break;
                    case 0x6B: break;
                    case 0x6C: break;
                    case 0x6D: break;
                    case 0x6E: break;
                    case 0xA0: break;
                    case 0xA1: break;
                    case 0xE1: break;
                    default:
                        {
                            u_char kind = g_spell_data[spell].kind
                                          & SPELL_KIND_MASK;

                            switch (kind) {
                            default:
                                if (BtlAnyMemberTargetable() == 0) {
                                    break;
                                }
                            case 0x1C:
                            case 0x1E:
                            case 0x1A:
                            case 0x32:
                                *ok = enabled;
                            }
                        }
                    }
                }
            next:
                i++;
                ok++;
            } while (i < 6);
        }
    }

    /* Whatever the mood lists, in its order, that survived. */
    i = 0;
    n = 0;
    do {
        BtlEnemyAi (*ai)[BTL_AI_MOODS] = g_btl_enemy_ai;
        /* Keep the row address separate: gcc adds the mood offset first. */
        u_long row = (u_long)ai[a->c.key];
        p = (u_char *)(g_btl_ai_set * sizeof(BtlEnemyAi) + row);
        if (g_btl_move_ok[p[i]] != 0) {
            g_btl_move_choices[n] = p[i];
            n++;
        }
        i++;
    } while (i < BTL_AI_MOVES);

    i = 0;
    roll = rand() % 255 + 1;
    spell = g_btl_ai_set;
    chance = g_btl_enemy_ai[a->c.key][spell].odds;
    if (n > 0) {
        u_long row;
        /* A row of eight per length, the first being for a list of one -
           so the count lands one row past its own. */
        live = (u_char)roll;
        weights = g_btl_move_odds;
        /* As above, the integer row address preserves the add's operands. */
        row = (u_long)weights[chance];
        p = (u_char *)(n * 8 + row);
        odds = p - 8;
        do {
            if ((u_int)live <= *odds) {
                return g_btl_move_choices[i];
            }
            i++;
            odds++;
        } while (i < n);
    }

    g_btl_boss22_shown = 1;
    g_btl_boss20_shown = 1;
    a->flags |= BTL_ACTOR_IDLE;
    return BTL_MOVE_IDLE;
}
