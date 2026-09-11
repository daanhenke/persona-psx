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
#define BTL_STATUS_POISON 0x0D
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
        if (g_btl_step == 0 && g_btl_intro_dist == BTL_INTRO_SETTLED) {
            if (g_btl_encounter < BTL_ENCOUNTER_SCRIPTED && g_btl_debug_skip != 0) {
                g_btl_stage = BTL_STAGE_CLOSE;
                g_btl_step = 0;
                return;
            }

            if (g_btl_encounter == BTL_ENCOUNTER_CHAIN) {
                g_btl_chain_battle = 1;
            }

            D_800F4D5C = 0;
            g_btl_round = 0;
            D_800F4E20 = 0;
            D_800F4AA4 = 0;
            D_800F5A60 = 0;
            D_800F5D58 = 0;
            D_800F586C = 0;

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

#ifdef NON_MATCHING
/* 99.62% by objdiff, and one instruction short of the image: it loads the
   ailment into the same register the key check used, so it has to copy the value
   out before the range test overwrites that register - `addu v1,v0,zero`, in the
   delay slot of the POISON branch. This loads straight into the register the
   range test wants and needs no copy, which is better code and four bytes
   shorter. Everything structural is settled; the copy is an allocator
   coin-flip, so it is the permuter's job. Do not chase it by routing a value
   through one of the loop counters - that is what the permuter proposes and it
   breaks the loop.

   Same shape as BtlStageOpen: the tail of the loop is the frame, and every
   step that is not finished simply falls out of the switch to draw one. Only
   the last step returns, and it is the one that hands the stage on - past the
   end of the table, which is what ends the battle. */
void BtlStageClose(void)
{
    SVECTOR unused;
    BtlObj *obj;
    int st;
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
        /* Taken into a local first: the step decides the block and the ailment
           below shares the register it lands in, which is what the image
           does - switching on the global straight costs the match. */
        st = g_btl_step;
        switch (st) {
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
                                /* POISON stands on its own and SICK and DOWN
                                   are tested as the one range they are - a
                                   three-case switch makes the compiler walk all
                                   three instead of folding the pair. */
                                /* POISON stands on its own and SICK and DOWN
                                   are tested as the one range they are - a
                                   three-case switch makes the compiler walk all
                                   three instead of folding the pair. */
                                st = (signed char)g_btl_actors[n].c.status;
                                if (st != BTL_STATUS_POISON
                                    && (u_int)(st - BTL_STATUS_SICK) >= 2) {
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
                        func_80097A50();
                        func_80098834();
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

#else
INCLUDE_ASM("btlp/nonmatchings/roundflow", BtlStageClose);
#endif

#ifdef NON_MATCHING
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
 */
void BtlStageRound(void)
{
    /* Thirty-two bytes the image reserves and never writes. */
    SVECTOR unused[4];

    /* Only these four outlive the step that sets them, which is why they are
       the only ones the routine puts on the stack. The cancel flag comes first:
       it takes the slot right above the reserved bytes in the image. */
    int cancelled;
    BtlActor *actor;
    BtlObj *obj;
    u_char action;

    cancelled = 0;
    for (;;) {
        if ((g_btl_pad1 & g_btl_key_cancel) != 0) {
            cancelled = 1;
        }

        switch (g_btl_step) {
        case 0: {
            int first;
            int last;
            int slot;
            int i;
            int n;
            int hit;
            int amount;
            int r;
            /* Whose stats are worked out again: one side, or both. Written as
               a switch, which is what the image tests - it jumps to each arm
               rather than branching past it. */
            switch (g_btl_battle_kind) {
            case 1:
                first = 0;
                last = BTL_PARTY;
                break;

            case 2:
                first = BTL_PARTY;
                last = BTL_ACTORS;
                break;

            default:
                first = 0;
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
                if (first < last) {
                    do {
                        if (g_btl_actors[first].c.key != 0
                            && (signed char)g_btl_actors[first].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[first].flags & BTL_ACTOR_OUT)
                                   == 0) {
                            BtlDeriveBattleStats(&g_btl_actors[first]);
                            g_btl_turn_order[n] = first;
                            n++;
                            /* Mostly the third stat, sometimes the fourth,
                               and on a rare roll both added. */
                            r = rand();
                            if ((u_int)(r & 0xFF) < 0xCE) {
                                i = g_btl_actors[first].stat[3];
                            } else if ((u_int)(r & 0xFF) < 0xEF) {
                                i = g_btl_actors[first].stat[4];
                            } else {
                                i = g_btl_actors[first].stat[3]
                                    + g_btl_actors[first].stat[4];
                            }
                            g_btl_actors[first].initiative = i;
                            g_btl_actors[first].action = 0xFF;
                        }
                        first++;
                    } while (first < last);
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
            slot = 0;
            hit = 0;
            g_btl_act_kind = 0;
            g_btl_act_actor = 0;
            g_btl_step++;
            i = 0;
            bzero(g_btl_debug_grid_cells, 0x4B);
            do {
                if (g_btl_actors[slot].c.key != 0
                    && (signed char)g_btl_actors[slot].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                    n = BtlActorPersona(slot);
                    if (g_btl_personas[n].slots > 5
                        && g_btl_actors[slot].c.blocked == 0
                        && ((g_persona_defs[g_btl_personas[n].key].unk1C
                             >> ((g_btl_actors[slot].c.key - 1) * 2))
                            & 3) == 3) {
                        amount = 0;
                        if ((g_btl_personas[n].unk41 & 0xF0) == 0x10) {
                            r = g_btl_actors[slot].c.hp_max;
                            if (r / 16 < g_btl_actors[slot].c.hp) {
                                if (r / 8 < g_btl_actors[slot].c.hp) {
                                    if (g_btl_actors[slot].c.hp <= r / 4) {
                                        amount = 0x10;
                                    }
                                } else {
                                    amount = 8;
                                }
                            } else {
                                amount = 4;
                            }
                            if ((signed char)g_btl_actors[slot].c.status
                                    != BTL_STATUS_NOINPUT
                                && amount != 0
                                && (rand() & (amount - 1)) == 0) {
                                g_btl_act_kind = 1;
                                hit = 1;
                                g_btl_act_actor = slot;
                            }
                        } else if ((g_btl_personas[n].unk41 & 0xF0) == 0x30) {
                            r = g_btl_actors[slot].c.hp_max;
                            if (r / 16 < g_btl_actors[slot].c.hp) {
                                if (g_btl_actors[slot].c.hp <= r / 8) {
                                    amount = 8;
                                }
                            } else {
                                amount = 4;
                            }
                            if ((signed char)g_btl_actors[slot].c.status
                                    != BTL_STATUS_NOINPUT
                                && amount != 0
                                && (rand() & (amount - 1)) == 0) {
                                g_btl_act_kind = 2;
                                hit = 1;
                                g_btl_act_actor = slot;
                            }
                        }
                    }
                }
                slot++;
            } while (slot < BTL_PARTY && hit == 0);

            if (g_btl_place_party != 0) {
                g_btl_act_kind = 0;
            }
            g_btl_round++;
            /* and straight on into the turn it just settled */

        }
        case 1: {
            int member;
            int slot;
            int i;
            int n;
            if (g_cd_busy == -1) {
                BtlShowAilmentMarks(0);
                if (BtlAnyStanding() == 0
                    || (g_btl_debug_hud != 0
                        && (g_btl_pad1 & g_btl_key_end) != 0)
                    || BtlBattleOutcome() == 0) {
                    goto round_over;
                } else if ((int)g_btl_turn < (int)(u_int)g_btl_turns) {
                    /* Whose turn this is, and how it came to be theirs. */
                    switch (g_btl_act_kind) {
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
                        g_btl_act_targets = actor->targets;
                        g_btl_act_speed = actor->order;
                        actor->order = BtlSlowestOrder() + 5;
                        actor->targets = BtlPickableMask();
                        actor->move = 0x6E;
                        break;
                    case 0:
                        slot = g_btl_turn_order[g_btl_turn];
                        actor = &g_btl_actors[slot];
                        g_btl_actor_turn = g_btl_turn_order[g_btl_turn];
                        member = actor->c.key;
                        obj = actor->obj;
                        break;
                    case 2:
                        slot = g_btl_act_actor;
                        g_btl_actor_turn = slot;
                        member = g_btl_actors[slot].c.key;
                        obj = g_btl_actors[slot].obj;
                        g_btl_act_move = g_btl_actors[slot].move;
                        g_btl_actors[slot].action = 6;
                        i = (u_char)g_btl_actor_turn;
                        g_btl_act_speed = g_btl_actors[slot].order;
                        g_btl_act_targets = g_btl_actors[slot].targets;
                        g_btl_actors[slot].flags |= 0x10000000;
                        g_btl_actors[slot].order = i;
                        i = 0x61;
                        actor = &g_btl_actors[slot];
                        actor->move = i;
                        actor->targets = 1 << (g_btl_actor_turn & 0x1F);
                        break;
                    case 3:
                        slot = g_btl_act_actor;
                        g_btl_actor_turn = slot;
                        member = g_btl_actors[slot].c.key;
                        obj = g_btl_actors[slot].obj;
                        g_btl_act_move = g_btl_actors[slot].move;
                        g_btl_actors[slot].action = 6;
                        i = (u_char)g_btl_actor_turn;
                        g_btl_act_speed = g_btl_actors[slot].order;
                        g_btl_act_targets = g_btl_actors[slot].targets;
                        g_btl_actors[slot].flags |= 0x10000000;
                        g_btl_actors[slot].order = i;
                        i = 0x6C;
                        actor = &g_btl_actors[slot];
                        actor->move = i;
                        actor->targets = 1 << (g_btl_actor_turn & 0x1F);
                        break;
                    }

                    if (g_btl_act_kind == 0) {
                        slot = g_btl_actor_turn;
                        if (g_btl_actors[slot].c.key == 0
                            || (signed char)g_btl_actors[slot].c.status
                                   == BTL_STATUS_DOWN
                            || (g_btl_actors[slot].flags & BTL_ACTOR_OUT)
                                   != 0) {
                            g_btl_step = 1;
                            g_btl_turn++;
                            break;
                        }
                    }

                    BtlDeriveBattleStats(actor);
                    if (g_btl_actor_turn < BTL_PARTY) {
                        if (g_btl_act_kind == 0
                            && (actor->flags & 0x22000) != 0) {
                            g_btl_step = 3;
                            break;
                        }
                        actor->pickable = 1;
                        if (g_btl_act_kind == 0) {
                            g_btl_marker_obj[g_btl_actor_turn]->attr
                                |= 0x1000000;
                            if (actor->c.status != 0) {
                                obj->mark->attr &= ~BTL_OBJ_HIDDEN;
                            }
                            if (g_btl_act_kind != 0) {
                                goto read_bank;
                            }
                            if (g_btl_pack_ready == 0) {
                                BtlRefreshEnemyAttacks();
                            }
                        } else {
                        read_bank:
                            BtlReadPackEntry(
                                1, g_btl_actors[g_btl_actor_turn].c.key);
                        }
                        action = actor->action;
                        switch (action) {
                        case 2:
                            if (actor->unkD5 != 0 && g_btl_msg_speed != 2) {
                                BtlOpenMessage(1, 1, g_btl_msg_ailment, 0x10,
                                               0xC);
                                g_btl_msg_timer = 0x1E;
                                if (g_btl_msg_speed == 0) {
                                    g_btl_msg_timer = 0xB4;
                                }
                            }
                            if (actor->script_pick == 2) {
                                BtlReloadMemberGfx(member, g_btl_actor_turn);
                                if (actor->c.equip[0] == 0
                                    || actor->c.equip[0] == 0xA3
                                    || actor->c.equip[0] == 0xAF) {
                                    actor->script_pick = 0;
                                } else {
                                    actor->script_pick = 1;
                                }
                                BtlObjSetScript(
                                    obj,
                                    (BtlSeqStep *)obj->scripts
                                        [g_btl_member_scripts
                                             [actor->script_pick * 10
                                              + member * 0x28]]);
                            }
                            break;
                        case 3:
                            if (g_btl_place_party == 0
                                && g_btl_msg_speed != 2) {
                                BtlOpenMessage(
                                    5, 1, g_btl_ailment_lines[actor->ail_line],
                                    0x10, 0xC);
                                g_btl_msg_timer = 0x1E;
                                if (g_btl_msg_speed == 0) {
                                    g_btl_msg_timer = 0xB4;
                                }
                            }
                            break;
                        case 6:
                            break;
                        case 0xC:
                            if (actor->script_pick != 2) {
                                BtlReloadMemberGfx(member + 10,
                                                   g_btl_actor_turn);
                                actor->script_pick = 2;
                                BtlObjSetScript(
                                    obj,
                                    (BtlSeqStep *)obj->scripts
                                        [g_btl_morph_scripts[member * 0x28]]);
                            }
                            n = 0x22;
                            if (obj->kind == 3) {
                                n = 0x24;
                            }
                            obj->children = n;
                            break;
                        case 0xE:
                        default:
                            goto step_on;
                        }
                        BtlStartAction();
                    } else {
                        actor->pickable = 1;
                        if (actor->c.status != 0) {
                            obj->mark->attr &= ~BTL_OBJ_HIDDEN;
                        }
                        if (g_btl_pack_ready == 0) {
                            BtlRefreshEnemyAttacks();
                        }
                        action = actor->action;
                        switch (obj->kind) {
                        case 0x95:
                        case 0x96:
                        case 0xBA:
                            n = 0x10;
                            break;
                        case 0x97:
                            n = 0xE;
                            break;
                        case 0xB1:
                            n = 0x15;
                            break;
                        case 0xB5:
                            n = 0xD;
                            break;
                        default:
                            goto enemy_action;
                        }
                        obj->children = n;
                    enemy_action:
                        switch (action) {
                        case 2:
                            actor->move = 0;
                            actor->unkBD = 0;
                            BtlAimEnemyMove(actor);
                        case 5:
                            BtlOpenPackBank();
                            break;
                        case 4:
                            actor->flags |= 0x2000;
                            break;
                        case 6:
                            actor->unkBD = actor->move;
                            actor->unkBA = actor->obj->unkD3;
                            if (g_btl_place_party == 0) {
                                BtlAimMove(actor);
                            }
                            BtlAimEnemyMove(actor);
                            BtlOpenPackBank();
                            if (g_btl_place_party != 0) {
                                actor->action = 0xFF;
                            }
                            break;
                        case 0xE:
                            BtlSoundOpen(g_btl_slot_banks, 6,
                                         (actor->obj->unkCD >> 1) - 5);
                        }
                    }
                step_on:
                    g_btl_step++;
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
            int i;
            int n;
            BtlHoldForMarkers();
            if (obj->motion != 0
                || (obj->attr & BTL_OBJ_BUSY_MASK) != BTL_OBJ_BUSY) {
                if (g_btl_scene_wanted != 0) {
                    i = 0;
                    BtlObjSetAttr(g_btl_hud_obj, BTL_OBJ_HIDDEN);
                    BtlHudLoad();
                    BtlHudShow();
                    do {
                        i++;
                        BtlDrawFrame();
                    } while (i < 60);
                    BtlSeqPlay(g_btl_seq_hud_up);
                    BtlSeqRun();
                    BtlHudHide();
                    g_btl_scene_hud = 0;
                    g_btl_scene_wanted = 0;
                }
                break;
            }
            BtlDrawFrame();
            BtlDrawFrame();
            g_btl_actors[g_btl_actor_turn].unkCC = 0;
            g_btl_actors[g_btl_actor_turn].unkD5 = 0;
            if (g_btl_effect_obj != NULL) {
                BtlObjSetMotion(g_btl_effect_obj, 4);
                i = 0;
                BtlObjSetPhase(g_btl_effect_obj, 0);
                g_btl_effect_obj = NULL;
                g_btl_scene_rgb[0] = 0x80;
                g_btl_scene_rgb[1] = 0x80;
                g_btl_scene_rgb[2] = 0x80;
                do {
                    i++;
                    BtlDrawFrame();
                } while (i < 0x78);
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
                    i = 0;
                    do {
                        i++;
                        BtlDrawFrame();
                    } while (i < 60);
                    BtlPlayScene(0, g_btl_line_enc03c);
                }
                if (obj->kind == 0x6E) {
                    BtlPlayScene(3, g_btl_line_enc03d);
                }
                break;
            case 2:
                if (obj->unkD2 == 6) {
                    BtlPlayScene(7, g_btl_line_enc04);
                }
                break;
            case 3:
                if (g_btl_place_party != 0) {
                    if (g_btl_turn == 0) {
                        BtlPlayScene(0x27, g_btl_line_enc05a);
                        i = 0;
                        do {
                            i++;
                            BtlDrawFrame();
                        } while (i < 60);
                        BtlPlayScene(5, g_btl_line_enc05b);
                    }
                    if (g_btl_turn == 1) {
                        BtlPlayScene(0x27, g_btl_line_enc05c);
                        i = 0;
                        do {
                            i++;
                            BtlDrawFrame();
                        } while (i < 60);
                        BtlPlayScene(0x29, g_btl_line_enc05d);
                        i = 0;
                        do {
                            i++;
                            BtlDrawFrame();
                        } while (i < 60);
                        BtlPlayScene(4, g_btl_line_enc05e);
                        i = 0;
                        do {
                            i++;
                            BtlDrawFrame();
                        } while (i < 60);
                        BtlPlayScene(0x29, g_btl_line_enc05f);
                    }
                    if (g_btl_turn == 2) {
                        BtlPlayScene(0x29, g_btl_line_enc08);
                        goto placed;
                    }
                }
                break;
            case 6:
                if (g_btl_place_party != 0) {
                    if (g_btl_turn == 0) {
                        BtlPlayScene(4, g_btl_line_enc08);
                    }
                placed:
                    g_btl_place_party = 0;
                    BtlRefreshMarkers();
                }
                break;
            case 15:
            case 16:
                if (g_btl_turn == 1) {
                    BtlPlayScene(8, g_btl_line_enc11a);
                }
                if (g_btl_turn == 4) {
                    BtlPlayScene(7, g_btl_line_enc11b);
                }
                if (g_btl_encounter == 0x11 && g_btl_turn == 5
                    && (BtlPlayScene(3, g_btl_line_enc11c),
                        g_btl_encounter == 0x12)
                    && g_btl_turn == 6) {
                    BtlPlayScene(7, g_btl_line_enc11d);
                }
            }

            slot = g_btl_actor_turn;
            if (slot < BTL_PARTY) {
                g_btl_marker_obj[slot]->attr &= ~0x1000000;
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
            if (g_btl_encounter != 7) {
                g_btl_step = 1;
                break;
            }
            goto round_over;

        }
        case 4: {
            int slowest;
            int slotsnd;
            int slot;
            int i;
            int n;
            int amount;
            int kind;
            if (g_btl_delay != 0 || BtlMarkersHidden() == 0) {
                break;
            }
            /* The two fights whose boss leaves the field rather than dying. */
            if ((g_btl_encounter == 0x1D && g_btl_actors[11].c.key != 0x9A)
                || (g_btl_encounter == 0x1E
                    && g_btl_actors[11].c.key != 0x9B)) {
                slotsnd = BTL_PARTY;
                BtlSoundOpen(g_btl_slot_banks, 6,
                             (g_btl_actors[BTL_PARTY].obj->unkCD >> 1) - 5);
                SsVabTransCompleted(1);
                slot = BTL_PARTY;
                do {
                    slotsnd++;
                    if (g_btl_actors[slot].c.key != 0
                        && (signed char)g_btl_actors[slot].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                        g_btl_actors[slot].c.hp = 0;
                        g_btl_actors[slot].obj->motion = 8;
                        g_btl_actors[slot].obj->phase = 0;
                    }
                    slot++;
                } while (slotsnd < 0xB);
                i = 0;
                do {
                    i++;
                    BtlDrawFrame();
                } while (i < 60);
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
                if ((g_btl_combatants[slowest].flags & 0x40) == 0) {
                    g_btl_combatants[slowest].unkD4 = 1;
                } else {
                    g_btl_combatants[slowest].padDD[1] = 1;
                    g_btl_combatants[slowest].unkD4 = 0;
                }
            }

            slot = 0;
            g_btl_actor_turn = -1;
            slotsnd = 7;
            do {
                actor = &g_btl_actors[slot];
                {
                    /* A palsy drains a share of the whole reserve. The two
                       tests below each ask whether the record is used again
                       rather than sharing one - which is what the image
                       does. */
                    if (actor->c.key != 0
                        && (signed char)actor->c.status != BTL_STATUS_DOWN
                        && (actor->flags & BTL_ACTOR_OUT) == 0
                        && (signed char)actor->c.status == 0x0E) {
                        amount = actor->c.sp - actor->c.sp_max / 16;
                        actor->c.sp = amount;
                        n = amount;
                        if (n < 0) {
                            n = 0;
                        } else if (n > 999) {
                            n = 999;
                        }
                        actor->c.sp = n;
                    }

                    if (actor->c.key != 0
                        && (signed char)actor->c.status != BTL_STATUS_DOWN
                        && (actor->flags & BTL_ACTOR_OUT) == 0
                        && (actor->unkD4 != 0
                            || (signed char)actor->c.status == 0x0D
                            || (signed char)actor->c.status == 0x10
                            || ((u_int)((signed char)actor->c.status - 4) < 2
                                && actor->c.ail_level == 2)
                            || (actor->flags & 0x34C0000) != 0)) {
                        if (slot < BTL_PARTY) {
                            if (slotsnd < 0xC) {
                                BtlSoundOpen(g_btl_banks, slotsnd,
                                             actor->c.key);
                            }
                        } else if (slotsnd < 0xC) {
                            BtlSoundOpen(g_btl_slot_banks, slotsnd,
                                         (actor->obj->unkCD >> 1) - 5);
                        }
                        SsVabTransCompleted(1);

                        kind = 0;
                        amount = 0;
                        /* Tested the way the image tests it: each in turn,
                           and the first that is set wins. */
                        if (actor->unkD4 != 0) {
                            amount = actor->c.hp;
                            kind = 7;
                        } else if ((actor->flags & 0x400000) != 0) {
                            kind = 7;
                            amount = actor->c.hp / 2;
                        } else if ((actor->flags & 0x40000) != 0) {
                            n = actor->persona_rank;
                            if (n == 3) {
                                kind = 0;
                                if (actor->padCD[2] == 0) {
                                    kind = 0x0F;
                                    amount = actor->c.hp / 8;
                                }
                            } else if (n < 4) {
                                if (actor->padCD[2] != 0) {
                                    amount = actor->c.hp_max / 8;
                                } else {
                                    amount = actor->c.hp / 8;
                                }
                                kind = 7;
                            } else if (n == 4) {
                                if (actor->padCD[2] == 0) {
                                    amount = actor->c.hp / 2;
                                } else {
                                    amount = actor->c.hp_max / 2;
                                }
                                kind = 7;
                            }
                        } else if ((actor->flags & 0x80000) != 0) {
                            /* The same three ranks, the other way up. */
                            n = actor->persona_rank;
                            if (n == 3) {
                                if (actor->padCD[2] == 0) {
                                    amount = actor->c.hp / 2;
                                } else {
                                    amount = actor->c.hp_max / 2;
                                }
                                kind = 7;
                            } else if (n < 4) {
                                if (actor->padCD[2] == 0) {
                                    amount = actor->c.hp / 8;
                                } else {
                                    amount = actor->c.hp_max / 8;
                                }
                                kind = 7;
                            } else if (n == 4) {
                                kind = 0;
                                if (actor->padCD[2] == 0) {
                                    kind = 0x0F;
                                    amount = actor->c.hp / 8;
                                }
                            }
                        } else {
                            i = (signed char)actor->c.status;
                            if (i == 5 || i == 4) {
                                if (actor->c.ail_level == 2) {
                                    kind = 7;
                                    amount = actor->c.hp_max / 16;
                                }
                            } else if (i == 0x0D) {
                                kind = 7;
                                amount = actor->c.hp_max / 16;
                            } else if (i == 0x10) {
                                kind = 7;
                                amount = actor->c.sp_max / 8;
                            } else if ((actor->flags & 0x1000000) == 0) {
                                kind = 0;
                                if ((actor->flags & 0x2000000) != 0) {
                                    kind = 0x0F;
                                    amount = actor->c.hp_max / 8;
                                }
                            } else {
                                n = actor->padCD[4] + 1;
                                actor->padCD[4] = n;
                                if (actor->padCD[4] == 0) {
                                    actor->padCD[4] = 1;
                                } else if ((u_char)n > 0x7F) {
                                    actor->padCD[4] = 0x7F;
                                }
                                amount = actor->padCD[4];
                                kind = 7;
                            }
                        }

                        switch (kind) {
                        case 7:
                            if (amount != 0) {
                                actor->c.hp -= amount;
                                actor->unk84 = amount;
                                if (actor->c.hp < 1) {
                                    actor->c.hp = 0;
                                    actor->obj->motion = 8;
                                    if (slotsnd < 0xC) {
                                        BtlSePlay(slotsnd, 1);
                                    }
                                } else {
                                    actor->obj->motion = 7;
                                    actor->obj->children = 0;
                                    if (slotsnd < 0xC) {
                                        BtlSePlay(slotsnd, 0);
                                    }
                                }
                            }
                            break;
                        case 0x0F:
                            BtlSePlay(2, 0xB);
                            actor->unk84 = amount;
                            amount += actor->c.hp;
                            actor->c.hp = amount;
                            n = amount;
                            if (actor->c.hp_max < n) {
                                n = actor->c.hp_max;
                            }
                            actor->c.hp = n;
                            actor->obj->motion = 0x0F;
                            break;
                        }
                        slotsnd++;
                        BtlDrawFrame();
                        BtlDrawFrame();
                        BtlDrawFrame();
                        BtlDrawFrame();
                    }
                }
                slot++;
            } while (slot < BTL_ACTORS);

            g_btl_delay = 0x20;
            do {
                BtlDrawFrame();
            } while (g_btl_delay != 0);

            i = 7;
            if (slotsnd > 7) {
                do {
                    BtlSoundClose(i);
                    i++;
                } while (i < slotsnd);
            }
            i = 0;
            BtlHoldForMarkers();
            do {
                i++;
                g_btl_actors[i - 1].flags &= 0xF7FC5FFF;
            } while (i < BTL_ACTORS);

            slot = 0;
            i = 0;
            do {
                if ((g_btl_actors[slot].c.key == 0
                     || (signed char)g_btl_actors[slot].c.status
                            == BTL_STATUS_DOWN
                     || (g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0)
                    && g_btl_actors[slot].marker != 0) {
                    g_btl_actors[slot].marker = 0;
                    BtlShowMarker(slot, 0, g_btl_actors[slot].c.unk5D & 0xF);
                }
                slot++;
            } while (slot < BTL_PARTY);

            if (BtlAnyStanding() == 0
                || (g_btl_debug_hud != 0 && (g_btl_pad1 & g_btl_key_end) != 0)
                || BtlBattleOutcome() == 0) {
                goto round_over;
            }
            if ((u_int)(g_btl_encounter - 0x1D) < 2) {
                BtlLoadPackBank(0xC5);
            }
            BtlPackEnemyGrid();
            BtlAfterTalk();
            goto step_on;

            /* The way out of the round. It is written here rather than where
               it is decided: the image puts the block at this end of the
               switch, and both the turn and the tally jump forward to it. */
        round_over:
            g_btl_step = 6;
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
            int slot;
            int i;

            if (BtlMarkersIdle() != 0 && BtlMarkersHidden() != 0) {
                i = 0;
                BtlBoxDismiss();
                BtlDrawFrame();
                BtlDrawFrame();
                BtlDrawFrame();
                bzero(g_btl_debug_grid_cells, 0x4B);
                slot = 0;
                g_btl_boss22_shown = 0;
                g_btl_boss20_shown = 0;
                do {
                    g_btl_actors[slot].unkDC = 0;
                    g_btl_actors[slot].unkDB = 0;
                    slot++;
                } while (slot < BTL_PARTY);
                BtlClearTalkMarks();
                BtlTalkResume();
                BtlPartyResetGfx();
                BtlEnemiesResetGfx();
                BtlShowReadyMarkers();
                /* Three ways a negotiation can leave the round, and the
                   default is the one that hands back to the menu. */
                switch (g_btl_talk_outcome) {
                case 2:
                    BtlTalkersLeaveField();
                    g_btl_step = 0;
                    break;
                case 1:
                    BtlTalkersStay();
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
#else
INCLUDE_ASM("btlp/nonmatchings/roundflow", BtlStageRound);
#endif

/* One enemy's turn, chosen fresh each time it comes round.
 *
 * Three things can take the turn before the move list is even looked at: a
 * boss whose script is to change shape, standing still, and running. Only
 * then are the six spells the fighter knows tested for whether they could be
 * cast at all, and whatever survives is weighed against the mood's odds.
 *
 * The unused copy of the ailment mask before the jump table remains in the
 * overlay's raw rodata; this unit emits the switch table itself.
 */
#define AI_SKIP_AILMENTS 0x0060C0FCUL

u_char BtlChooseEnemyMove(BtlActor *a)
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
                            && (g_btl_combatants[n].flags & 0x1E00) == 0) {
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
                                int flags = q[n].offered;
                                flags |= q[n].unkE1[0];
                                flags |= q[n].unkE1[1];
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
                                int flags = g_btl_actors[n].unkE1[2];
                                flags |= g_btl_actors[n].unkE1[3];
                                flags |= g_btl_actors[n].unkE1[4];
                                flags |= g_btl_actors[n].unkE1[5];
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
                            if (q[n].c.key != 0 && (q[n].flags & 0x80) == 0) {
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
                            if (q[n].c.key != 0 && (q[n].flags & 0x100) == 0) {
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
