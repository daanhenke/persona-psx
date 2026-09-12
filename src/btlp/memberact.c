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
#include <decomp/libc.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/model.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stats.h>
#include <persona/common/item.h>
#include <persona/btlp/text.h>
#include <persona/btlp/round.h>

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



/* Stride of g_btl_member_scripts, by model and by the actor's script_pick,
   and the two of the ten a summon takes: the pose the Persona comes out of,
   and the one the member is left standing in. */
#define MEMBER_SCRIPT_MODEL 0x28
#define MEMBER_SCRIPT_PICK  10
#define SCRIPT_SUMMON 3
#define SCRIPT_STAND  4

/* Set on a member whose Persona is already out, which is what makes the
   motion give up before it starts. */
#define SUMMON_DONE 0x200000

/* Where the pack the summon's two files live in starts. Not the base
   BtlSummonActorPersona reads a Persona's own artwork with. */
extern int D_800F4BA0;

/* The last entries of the Persona pack's sector table are the summon's own
   artwork rather than a Persona's, and entry 0xFD of the sound pack is the
   fanfare that plays under it. */
#define SUMMON_FILE 0x16F
#define SUMMON_BGM  0xFD

/* Where the artwork is read to, and the two addresses the loader leaves at
   the head of it: the tim in the first and the graphics in the second. */
#define SUMMON_STAGE ((u_long *)0x80140000)
extern u_char *g_load_stage;
extern u_char *D_80140004;

/* Where the graphics are staged and bound, and the page the tim goes to. */
#define SUMMON_GFX_STAGE 0x801D9400
#define SUMMON_GFX_BYTES 0x1000
#define SUMMON_GFX_KIND  3
#define SUMMON_GFX_INDEX 0xFE
#define SUMMON_TIM_PAGE  0x1A
#define SUMMON_TIM_SLOT  0x10

extern u_char   *g_btl_fx_gfx;
extern u_char   *g_btl_unused_gfx;
extern BtlObjDef g_btl_summon_def;

/* What the arena drops to while the Persona comes out, and what it is put
   back to afterwards. */
#define SUMMON_SCENE_DIM  0x40
#define SUMMON_SCENE_LIT  0x80
#define SUMMON_ARENA_FADE 2

/* The trail: eight copies of the member's own record, each a further
   sixty-fourth of the wave tables round, all on the swing of motion 0x12. */
#define TRAIL_RECORDS 8
#define TRAIL_TURN    6
#define TRAIL_MOTION  0x12
#define TRAIL_FADE    0xFF
#define TRAIL_RGB     0x60
#define TRAIL_RGB_B   0xC0
#define TRAIL_ATTR    0x20000001

/* What the one record the template builds is put out as. */
#define SUMMON_GROUP 2
#define SUMMON_DRAW  5

/* Frames the Persona is left standing before the member takes over again. */
#define SUMMON_HOLD 0x5A

/* Written as each copy is made and read by nothing else in the overlay. */
extern BtlObj *D_800F4894;

extern void BtlApplyPersona(BtlActor *a);
extern void BtlRefreshAttacks(void);
extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern int  BtlBindGfx(u_int kind, int index, u_char **image);
extern void BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                         int upload);
extern short g_btl_scene_rgb[];

/* Of the ten script indices a shape carries, the swing takes four: the run
   in at 0, the one the blow itself is struck from at 1, and the two the
   fighter is left standing in - 2 for an ordinary turn and 0 when the record
   has been marked. */
#define SCRIPT_RUN_IN 0
#define SCRIPT_STRIKE 1
#define SCRIPT_AFTER  2

/* The ailment that stops a swing outright, and the slot codes the walk skips
   over. */
#define SWING_AIL_0C 0x0C

/* Raised on a fighter the swing may not land on. */
#define SWING_SPARED 0x4000

/* Slots the target walk runs through: the five of the party and the nine
   behind them. */
#define SWING_SLOTS 0xE

/* Frames the run in and the run back each take, and how far up the field a
   member that has changed places lands. */
#define SWING_FRAMES 0x14
#define SWING_STAND  0xD20000
#define SWING_LIFT   0x3C0000

/* Cleared as the swing is set up: the bit that says the record is being
   carried rather than standing. */
#define SWING_CARRIED 0x20000

/* The two kinds the swing poses differently for, and the message it waits
   behind when there is nothing left to hit. */
#define SWING_KIND_A 2
#define SWING_KIND_B 8

/* Where the party stops and the field's own slots start. */
#define SWING_FIRST_ENEMY 5

/* Where the grid puts a member back, written the way the image writes it -
   the row biased by ten and the whole thing pulled back by 0x8C, which comes
   to the same place memberobj.c reaches by adding 0x3C. */
#define SWING_X_STEP   15
#define SWING_X_BASE   (-0x3C)
#define SWING_ROW_BIAS 10
#define SWING_Y_STEP   20
#define SWING_Y_BASE   (-0x8C)

/* The weapon record the swing is reading. */
extern ItemDef *g_btl_swing_item;

/* The order and targets a member that swapped places had before it did, put
   back as the turn ends. */
extern u_char  D_800F4BA4;
extern u_short D_800F5AAC;

/* The line that goes up when the swing finds nothing to land on. */
extern u_char D_800CFA00;

/* Two bytes 0x1E into the same row of g_btl_member_scripts: which voice bank
   the fighter speaks the swing from, one per hand. */
extern u_char g_btl_member_voice[];

extern int  BtlMarkMoveArea(BtlActor *a, int area, int spread);
extern int  BtlSlowestOrder(void);
extern int  func_80094E60(int hits);
extern void BtlReadVoiceBank(int entry);
extern void BtlOpenVoiceBank(void);
extern void BtlMemberStrike(BtlObj *o);

#ifdef NON_MATCHING
/* The swing: entry 2 of g_btl_member_motion, and entry 0x0C as well.
 *
 * Nought settles what is being swung - which hand, which weapon record, which
 * voice - and works out the run in; one carries the record there; two opens
 * the voice and either finds a target or puts the "nothing to hit" line up.
 * Three is the walk that picks the next slot out of the acting fighter's
 * target mask, four sounds the swing, five hands over to BtlMemberStrike, and
 * six closes that hit and goes back to three for the next one. Seven and
 * eight carry the record home; nine and ten are the two ways a turn that
 * never landed anything ends.
 */
void BtlMemberMotion02(BtlObj *o)
{
    /* Thirty-two bytes of frame nothing here writes; the routine is the
       wrong length without them. */
    long          scratch[8];
    BtlActor     *a;
    const u_char *scripts;
    u_short       walk;
    u_short       kept;
    int           done;
    int           slot;
    int           i;

    a = o->actor;
    switch (o->phase) {
    case 0:
        o->attr &= ~SWING_CARRIED;
        if (*(signed char *)&a->c.status == SWING_AIL_0C) {
            o->phase = 0xA;
            break;
        }
        if (a->script_pick != 2) {
            g_btl_swing_item = &g_item_defs[a->c.equip[0]];
            BtlReadVoiceBank(g_btl_member_voice[a->c.key
                                                * MEMBER_SCRIPT_MODEL]);
        } else {
            g_btl_swing_item = &g_item_defs[a->c.equip[1]];
            BtlReadVoiceBank(g_btl_member_voice[a->c.key * MEMBER_SCRIPT_MODEL
                                                + 1]);
        }
        g_btl_hit_slot = a->order;
        if (g_btl_actors[g_btl_hit_slot].c.key == 0) {
            if (BtlMarkMoveArea(a, g_btl_swing_item->area,
                                g_btl_swing_item->swing)
                < 0) {
                o->phase = 0xA;
                break;
            }
            slot = BtlSlowestOrder();
            g_btl_hit_slot = slot;
            if ((short)slot < 0) {
                o->phase = 0xA;
                break;
            }
            g_btl_hit_slot = slot + SWING_FIRST_ENEMY;
            a->order = g_btl_hit_slot;
        }
        g_btl_hits_left = func_80094E60(g_btl_swing_item->hits);
        scripts = &g_btl_member_scripts[SCRIPT_RUN_IN
                                        + o->kind * MEMBER_SCRIPT_MODEL];
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
            scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        if (o->motion == 0xC) {
            o->step_x = (g_btl_actors[g_btl_hit_slot].obj->x - o->x)
                        / SWING_FRAMES;
            o->step_y = (SWING_STAND - o->y) / SWING_FRAMES;
        } else {
            o->step_x = (g_btl_actors[g_btl_hit_slot].obj->x - o->x)
                        / SWING_FRAMES;
            o->step_y = (g_btl_actors[g_btl_hit_slot].obj->y - o->y
                         + SWING_LIFT)
                        / SWING_FRAMES;
        }
        o->steps = SWING_FRAMES;
        o->phase++;
        break;
    case 1:
        if (g_btl_hit_slot < SWING_FIRST_ENEMY || a->script_pick == 0
            || (a->script_pick != 2 && o->kind != SWING_KIND_A
                && o->kind != SWING_KIND_B)) {
            o->x += o->step_x;
            o->y += o->step_y;
            if (--o->steps != 0) {
                return;
            }
            o->x2 = o->x;
            o->y2 = o->y;
            scripts = &g_btl_member_scripts[SCRIPT_STRIKE
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            o->steps = SWING_FRAMES;
            o->phase++;
        } else {
            scripts = &g_btl_member_scripts[SCRIPT_STRIKE
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            o->phase++;
        }
        break;
    case 2:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        if (g_cd_busy != -1) {
            return;
        }
        BtlOpenVoiceBank();
        if (g_btl_hits_left == 0) {
            if (g_btl_msg_speed != 2) {
                BtlOpenMessage(1, 1, &D_800CFA00, 8, 0xC);
                if (g_btl_msg_speed == 0) {
                    i = 0x3C;
                } else {
                    i = 0x1E;
                }
                g_btl_msg_timer = i;
            }
            o->timer = 0x3C;
            o->phase = 9;
            break;
        }
        g_btl_hit_walk = -1;
        g_btl_hit_mask = 1;
        a->targets &= ~(1 << g_btl_hit_slot);
        o->phase += 2;
        break;
    case 3:
        do {
            kept = g_btl_hits_left;
            done = 0;
            if ((g_btl_swing_item->swing & 1) != 0
                || g_btl_swing_item->swing == 8) {
                if (g_btl_actors[g_btl_hit_slot].c.key == 0
                    || *(signed char *)&g_btl_actors[g_btl_hit_slot].c.status
                           == BTL_STATUS_DOWN
                    || (done = 1,
                        (g_btl_actors[g_btl_hit_slot].flags & SWING_SPARED)
                            != 0)) {
                    g_btl_hits_left = 0;
                    done = 1;
                }
            } else {
                walk = g_btl_hit_walk;
                if ((short)g_btl_hit_walk < SWING_SLOTS) {
                    do {
                        if ((a->targets & g_btl_hit_mask) != 0
                            && g_btl_actors[(short)walk].c.key != 0
                            && *(signed char *)&g_btl_actors[(short)walk]
                                    .c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[(short)walk].flags & SWING_SPARED)
                                   == 0) {
                            g_btl_hit_slot = walk;
                            done = 1;
                            break;
                        }
                        walk = g_btl_hit_walk + 1;
                        g_btl_hit_mask <<= 1;
                        g_btl_hit_walk = walk;
                    } while ((short)walk < SWING_SLOTS);
                    if ((short)g_btl_hit_walk < SWING_SLOTS) {
                        continue;
                    }
                }
                done = 1;
                if ((g_btl_swing_item->swing & 2) == 0) {
                    g_btl_hits_left = 0;
                    done = 1;
                } else {
                    i = 0;
                    g_btl_hits_left = 0;
                    a->targets |= 1 << a->order;
                    do {
                        if (((a->targets >> (i & 0x1F)) & 1) != 0
                            && g_btl_actors[i].c.key != 0
                            && *(signed char *)&g_btl_actors[i].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[i].flags & SWING_SPARED) == 0) {
                            done = 0;
                            g_btl_hits_left = kept;
                            g_btl_hit_walk = 0;
                            g_btl_hit_mask = 1;
                        }
                        i++;
                    } while (i < SWING_SLOTS);
                }
            }
        } while (done == 0);
        o->phase++;
        /* fallthrough */
    case 4:
        if (g_btl_hits_left > 0) {
            if (g_btl_hit_slot < SWING_FIRST_ENEMY) {
                BtlSoundOpen(g_btl_banks, 6,
                             g_btl_actors[g_btl_hit_slot].c.key);
            } else {
                BtlSoundOpen(g_btl_slot_banks, 6,
                             (g_btl_actors[g_btl_hit_slot].obj->unkCD >> 1)
                                 - SWING_FIRST_ENEMY);
            }
            o->phase++;
            break;
        }
        if (g_btl_place_party != 0) {
            a->action = 0xFF;
        }
        if (a->unkD5 != 0) {
            a->unkD8 = 1;
            a->unkD5 = 0;
            a->order = D_800F4BA4;
            a->targets = D_800F5AAC;
            if (a->padD6[1] != 0) {
                a->action = 0;
            } else {
                a->action = 0xFF;
            }
        }
        BtlRefreshAttacks();
        BtlSoundClose(6);
        if ((o->attr & SWING_CARRIED) != 0) {
            scripts = &g_btl_member_scripts[SCRIPT_RUN_IN
                                            + o->kind * MEMBER_SCRIPT_MODEL];
        } else {
            scripts = &g_btl_member_scripts[SCRIPT_AFTER
                                            + o->kind * MEMBER_SCRIPT_MODEL];
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
            scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        o->phase = 7;
        break;
    case 5:
        BtlMemberStrike(o);
        break;
    case 6:
        if (g_btl_actors[g_btl_hit_slot].obj->motion != 0) {
            return;
        }
        if (o->timer != 0) {
            return;
        }
        BtlSoundClose(6);
        if ((g_btl_swing_item->swing & 4) == 0) {
            g_btl_hits_left--;
        }
        if ((short)g_btl_hit_walk < 0) {
            i = 1;
        } else {
            i = g_btl_hit_mask << 1;
        }
        g_btl_hit_mask = i;
        g_btl_hit_walk++;
        o->phase = 3;
        break;
    case 7:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->timer = 0x1E;
        o->phase++;
        break;
    case 8:
        if (o->timer != 0) {
            return;
        }
        if (g_btl_hit_slot < SWING_FIRST_ENEMY || a->script_pick == 0
            || (a->script_pick != 2 && o->kind != SWING_KIND_A
                && o->kind != SWING_KIND_B)) {
            o->x -= o->step_x;
            o->y -= o->step_y;
            if (--o->steps != 0) {
                return;
            }
            o->x = (o->col2 * SWING_X_STEP + SWING_X_BASE) << 16;
            o->y = ((*(u_char *)&o->row + SWING_ROW_BIAS) * SWING_Y_STEP
                    + SWING_Y_BASE)
                   << 16;
            o->motion = 0;
            o->x2 = o->x;
            o->attr &= ~SWING_CARRIED;
            o->y2 = o->y;
        } else {
            o->motion = 0;
            o->attr &= ~SWING_CARRIED;
        }
        break;
    case 9:
        if (o->timer != 0) {
            return;
        }
        BtlCloseMessage(0);
        o->phase = 4;
        break;
    case 0xA:
        if (g_cd_busy != -1) {
            return;
        }
        if (a->unkD5 != 0) {
            a->unkD8 = 1;
            a->unkD5 = 0;
            a->order = D_800F4BA4;
            a->targets = D_800F5AAC;
        }
        BtlRefreshAttacks();
        BtlSoundClose(6);
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberMotion02);
#endif

INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberStrike);

/* The ailments that stop a cast before it starts. None of them is named
   anywhere else in the tree, so they are spelt by their codes: 0x0C once it
   has been driven past the first level, and 0x09 at the deepest - the second
   tested as one masked word, because status and ail_level are adjacent bytes
   and the original reads the pair together. 0x08 is the one that may turn the
   cast into a stumble instead, which is a message and a wait rather than a
   way out. */
#define CAST_AIL_0C      0x0C
#define CAST_AIL_MASK    0xFFFF00
#define CAST_AIL_09_DEEP 0x20900
#define CAST_AIL_08      8
#define CAST_AIL_DEEPEST 2

/* Where the cast's own files sit at the tail of the Persona pack's sector
   table, where the moves' artwork starts in the same table, and the base
   sector BtlSummonActorPersona reads a Persona's artwork with. */
#define CAST_FILE       0x171
#define MOVE_FILE_FIRST 0x72
extern int D_800F4BAC;

/* The page the cast's tim goes to, and the one step the arena fades by. */
#define CAST_TIM_PAGE   0x1C
#define CAST_TIM_SLOT   0x11
#define CAST_ARENA_FADE 1

/* Set on a fighter whose turn a script is driving; the cast then neither
   poses the member nor sounds the swing. */
#define CAST_SCRIPTED 0x10000000

/* Raised on a fighter that is already dimmed, and what the rest of the field
   is taken down to while the Persona is out. */
#define CAST_DIMMED 0x4000
#define CAST_DIM    0x20

/* How far a record is lifted off the field for the cast. */
#define CAST_LIFT (-0x300000)

/* The Persona: how it is put up, and how many records it is made of. */
#define CAST_PERSONA_ATTR   0x20000000
#define CAST_HUD_ATTR       0x40000000
#define CAST_SCALE_XY       0x100
#define CAST_SCALE_Z        0x1000
#define CAST_PERSONA_PIECES 6

/* Which enemy the fourth encounter's detour is for: the one whose
   voice bank is swapped in behind the cast. */
#define CAST_BOSS 3

extern u_char   g_btl_seq_catchup;
extern u_char   g_btl_persona_ready;
extern u_char   g_btl_act_speed;
extern u_char   g_btl_act_move;
extern u_char  *g_btl_move_lines[];
extern u_char   D_800CF7EC[];
extern u_char   D_800CFA10[];
extern u_char   D_8004E264;
extern u_char   D_800E49BF;

extern BtlObj *g_btl_persona_obj;
extern BtlObj *BtlSpawnPersona(int gfx, int col, int row, int motion);
extern void    BtlRecalcStats(BtlActor *a);
extern void    BtlEnemyDeriveStats(BtlStats *s);
extern int     BtlActorSlotByKey(int key);
extern void    func_800A6D3C(BtlActor *a, int move);
extern BtlObj *func_80084E10(int kind, long *pos);
extern BtlObj *func_80084A14(int kind, int col, int row);
extern void    func_80097158(BtlStats *s);

#ifdef NON_MATCHING
/* The cast: what a member's turn runs through when the move is a spell and
 * the Persona has to come out to make it.
 *
 * The phases fall into four runs. Nought settles what is being cast and puts
 * the line up; one to three read the Persona in and put it on the field; four
 * to seven read the move's own artwork and voice, play it, then take the
 * Persona back off and give the member the stats it left behind. Eight to ten
 * are the three ways out - the ordinary one, the one a fumble takes, and the
 * one that only tidies up. Everything from eleven to nineteen is an arm of the
 * jump table that nothing reaches. Twenty and twenty-one are the demon's own
 * bank, which the fourth encounter takes a detour through.
 *
 * Phase ten is also where a fighter that cannot act at all lands, which is why
 * nought answers a charmed or bound one by going straight there.
 */
void BtlMemberMotion06(BtlObj *o)
{
    CdlLOC       loc;
    BtlSoundBank bank;
    /* Eight bytes of frame nothing here writes; the routine is the
       wrong length without them. */
    long          scratch[2];
    BtlActor     *a;
    BtlObj       *p;
    const u_char *scripts;
    const u_char *line;
    int           i;
    int           speed;
    int           status;
    /* The ailment code and the message's last argument are the same number,
       and the image holds it across the whole arm rather than building it
       again for the call. */
    int           stop;

    a = o->actor;
    switch (o->phase) {
    case 0:
        stop = CAST_AIL_0C;
        status = *(signed char *)&a->c.status;
        if ((status == stop && *(signed char *)&a->c.ail_level > 0)
            || (*(u_long *)&a->c.name[9] & CAST_AIL_MASK) == CAST_AIL_09_DEEP
            || status == BTL_STATUS_LIFTED) {
            o->phase = 0xA;
            break;
        }
        g_btl_hit_slot = a->order;
        func_800A6D3C(a, a->move);
        if (o->phase == 0xA) {
            return;
        }
        if (g_btl_msg_speed != 2) {
            if (g_btl_place_party == 0 && g_btl_act_kind == 0) {
                line = g_btl_move_lines[a->move];
            } else if (g_btl_act_kind != 0) {
                line = D_800CF7EC;
            } else {
                goto spoken;
            }
            BtlOpenMessage(1, 1, line, 0x10, stop);
            if (g_btl_msg_speed == 0) {
                speed = 0xB4;
            } else {
                speed = 0x1E;
            }
            g_btl_msg_timer = speed;
        }
    spoken:
        i = 0;
        o->actor->padCB[0] = g_btl_personas[BtlActorPersona(o->mark_num)].key;
        do {
            if (g_btl_personas[BtlActorPersona(o->mark_num)].raw[i]
                == a->move) {
                o->unkD3 = i;
                break;
            }
            i++;
        } while (i < BTL_STATS_SPELLS);
        if (i >= BTL_STATS_SPELLS) {
            o->unkD3 = 0;
        }
        scripts = &g_btl_member_scripts[SCRIPT_SUMMON
                                        + o->kind * MEMBER_SCRIPT_MODEL];
        if ((a->flags & CAST_SCRIPTED) == 0) {
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            BtlSePlay(3, rand() % 3 + 3);
        } else if (g_btl_act_kind != 3) {
            func_80084E10(0, &o->x)->z += CAST_LIFT;
        }
        if (*(signed char *)&g_btl_actors[g_btl_actor_turn].c.status == CAST_AIL_08
            && ((rand() & 1) != 0
                || *(signed char *)&g_btl_actors[g_btl_actor_turn].c.ail_level
                       == CAST_AIL_DEEPEST)) {
            BtlOpenMessage(1, 1, D_800CFA10, 8, 0xC);
            o->timer = 0x78;
            o->phase = 9;
            break;
        }
        i = 0;
        do {
            if (g_btl_actors[i].c.key != 0
                && *(signed char *)&g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & CAST_DIMMED) == 0
                && g_btl_actors[i].c.key != o->kind) {
                g_btl_actors[i].obj->rgb_to[0] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[1] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[2] = CAST_DIM;
                BtlObjSetFade(g_btl_actors[i].obj, 4);
            }
            i++;
        } while (i < BTL_PARTY);
        CdIntToPos(g_btl_persona_sectors[CAST_FILE] + D_800F4BA0, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[CAST_FILE + 1]
                                  - g_btl_persona_sectors[CAST_FILE],
                              SUMMON_STAGE);
        o->phase++;
        break;
    case 1:
        if (g_cd_busy != -1) {
            return;
        }
        BtlUploadTim((u_long *)g_load_stage, CAST_TIM_PAGE, CAST_TIM_SLOT, 1,
                     0, 1);
        CdIntToPos(g_btl_persona_sectors[o->actor->padCB[0]] + D_800F4BAC,
                   &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[o->actor->padCB[0] + 1]
                                  - g_btl_persona_sectors[o->actor->padCB[0]],
                              SUMMON_STAGE);
        o->phase++;
        break;
    case 2:
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            return;
        }
        o->rgb_to[0] = 0xFF;
        o->rgb_to[1] = 0xFF;
        o->rgb_to[2] = 0xFF;
        g_btl_scene_rgb[0] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[1] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[2] = SUMMON_SCENE_DIM;
        o->fade = CAST_ARENA_FADE;
        g_btl_arena_fade = CAST_ARENA_FADE;
        g_btl_hud_obj = func_80084A14(0, o->col2, o->row);
        if (g_btl_act_kind == 3) {
            BtlObjSetAttr(g_btl_hud_obj, CAST_HUD_ATTR);
        }
        BtlSePlay(5, 0);
        o->timer = 0x3C;
        o->phase++;
        break;
    case 3:
        if (o->timer != 0) {
            return;
        }
        if (g_cd_busy != -1) {
            return;
        }
        if ((a->flags & CAST_SCRIPTED) == 0) {
            scripts = &g_btl_member_scripts[SCRIPT_SUMMON
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK] + 1]);
            if (g_btl_encounter == 3 && o->kind != 3) {
                func_80084E10(
                    0, &g_btl_actors[BtlActorSlotByKey(3)].obj->x)
                    ->z += CAST_LIFT;
            }
        } else if ((g_btl_encounter == 0 && o->kind == 1)
                   || (g_btl_encounter == 2 && o->kind == 2)) {
            i = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && g_btl_actors[i].c.key != o->kind) {
                    func_80084E10(1, &g_btl_actors[i].obj->x)->z += CAST_LIFT;
                }
                i++;
            } while (i < BTL_PARTY);
        }
        BtlCloseMessage(0);
        g_btl_seq_catchup = 1;
        *(int *)&o->actor->pad68[8] += 1;
        D_800F5A60++;
        if (D_8004E264 == 0 && g_btl_act_kind == 0) {
            a->c.sp -= g_btl_personas[BtlActorPersona(o->mark_num)].unk29;
        }
        g_btl_persona_ready = 0;
        g_btl_persona_obj = BtlSpawnPersona(a->padCB[0], o->col2, o->row,
                                            o->unkD3);
        g_btl_persona_obj->actor = o->actor;
        BtlObjSetAttr(g_btl_persona_obj, CAST_PERSONA_ATTR);
        BtlObjSetScale(g_btl_persona_obj, CAST_SCALE_XY, CAST_SCALE_XY,
                       CAST_SCALE_Z);
        BtlObjSetMotion(g_btl_persona_obj, 3);
        g_btl_half_rate = 1;
        o->phase++;
        break;
    case 4:
        CdIntToPos(g_btl_persona_sectors[MOVE_FILE_FIRST + o->actor->move]
                       + D_800F4BA0, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[MOVE_FILE_FIRST
                                                    + o->actor->move + 1]
                                  - g_btl_persona_sectors[MOVE_FILE_FIRST
                                                          + o->actor->move],
                              SUMMON_STAGE);
        o->phase++;
        break;
    case 5:
        if (g_cd_busy != -1) {
            return;
        }
        CdIntToPos(g_btl_pack_sectors[o->actor->move] + g_btl_voice_base,
                   &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_pack_sectors[o->actor->move + 1]
                                  - g_btl_pack_sectors[o->actor->move],
                              (u_long *)BTL_VOICE_BUFFER);
        o->phase++;
        break;
    case 6:
        if (g_cd_busy != -1) {
            return;
        }
        if (g_btl_encounter == 3) {
            BtlSoundClose(3);
            BtlReadPackBank(0, g_btl_enemies[CAST_BOSS].c.key);
            o->phase = 0x13;
        }
        bank.nsep = BTL_VOICE_SEPS;
        bank.vb = g_btl_voice_vb;
        bank.vh = g_btl_voice_vh;
        bank.seq = g_btl_voice_seq;
        BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        g_btl_persona_ready = 1;
        o->phase++;
        break;
    case 7:
        if (g_btl_persona_obj->motion != 0) {
            return;
        }
        i = 0;
        p = g_btl_persona_obj;
        g_btl_half_rate = 0;
        g_btl_scene_rgb[0] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[1] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[2] = SUMMON_SCENE_LIT;
        do {
            BtlObjFree(p);
            p = p->attached;
            i++;
        } while (i < CAST_PERSONA_PIECES);
        BtlSoundClose(BTL_BGM_SLOT);
        func_80097158(&g_btl_personas[BtlActorPersona(o->mark_num)]);
        BtlApplyPersona(a);
        BtlRecalcStats(a);
        BtlDeriveBattleStats(a);
        BtlEnemyDeriveStats(&g_btl_personas[BtlActorPersona(o->mark_num)]);
        if (g_btl_act_kind != 0) {
            a->order = g_btl_act_speed;
            a->targets = g_btl_act_targets;
            a->action = 0;
            a->move = g_btl_act_move;
        }
        BtlRefreshAttacks();
        o->phase++;
        break;
    case 8:
        if ((a->flags & CAST_SCRIPTED) == 0 && a->unkDF == 0) {
            scripts = &g_btl_member_scripts[SCRIPT_STAND
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        }
        a->unkDF = 0;
        BtlObjFree(g_btl_hud_obj);
        BtlObjFree(g_btl_hud_obj->attached);
        g_btl_hud_obj = 0;
        o->motion = 0;
        o->phase = 0;
        return;
    case 9:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        if (o->timer != 0) {
            return;
        }
        scripts = &g_btl_member_scripts[SCRIPT_STAND
                                        + o->kind * MEMBER_SCRIPT_MODEL];
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
            scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        o->phase++;
        break;
    case 0xA:
        if (g_cd_busy != -1) {
            return;
        }
        BtlBoxDismiss();
        BtlRefreshAttacks();
        BtlSoundClose(6);
        o->motion = 0;
        o->phase = 0;
        return;
    case 0x14:
        if (g_cd_busy != -1) {
            return;
        }
        BtlOpenPackBank();
        o->phase++;
        break;
    case 0x15:
        if (SsVabTransCompleted(0) == 0) {
            return;
        }
        BtlObjSetScript(g_btl_enemies[CAST_BOSS].obj,
                        (BtlSeqStep *)g_btl_enemies[CAST_BOSS].obj
                            ->scripts[D_800E49BF]);
        o->phase = 7;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/memberact", BtlMemberMotion06);
#endif

/* The Persona coming out: the file is read while the member takes the pose,
   the fanfare behind it, and then the trail is thrown round the member and
   the field held for ninety frames. A member already carrying its Persona
   gives up at the first phase and leaves the motion where it found it. */
void BtlMemberMotion05(BtlObj *o)
{
    CdlLOC       loc;
    BtlSoundBank bank;
    BtlObj        *trail;
    const u_char  *scripts;
    u_long         attr;
    u_char         motion;
    int            i;

    switch (o->phase) {
    case 0:
        if (o->actor->flags & SUMMON_DONE) {
            BtlRefreshAttacks();
            o->motion = 0;
            o->phase = 0;
        } else {
            o->actor->c.entry = o->actor->form;
            BtlApplyPersona(o->actor);
            scripts = &g_btl_member_scripts[SCRIPT_SUMMON
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            CdIntToPos(g_btl_persona_sectors[SUMMON_FILE] + D_800F4BA0, &loc);
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_persona_sectors[SUMMON_FILE + 1]
                                      - g_btl_persona_sectors[SUMMON_FILE],
                                  SUMMON_STAGE);
            g_btl_scene_rgb[0] = SUMMON_SCENE_DIM;
            g_btl_scene_rgb[1] = SUMMON_SCENE_DIM;
            g_btl_scene_rgb[2] = SUMMON_SCENE_DIM;
            g_btl_arena_fade = SUMMON_ARENA_FADE;
            o->phase++;
        }
        break;
    case 1:
        if (g_cd_busy != -1) {
            return;
        }
        CdIntToPos(g_btl_pack_sectors[SUMMON_BGM] + g_btl_voice_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_pack_sectors[SUMMON_BGM + 1]
                                  - g_btl_pack_sectors[SUMMON_BGM],
                              (u_long *)BTL_VOICE_BUFFER);
        o->phase++;
        break;
    case 2:
        if (g_cd_busy != -1) {
            return;
        }
        bank.nsep = BTL_VOICE_SEPS;
        bank.vb = g_btl_voice_vb;
        bank.vh = g_btl_voice_vh;
        bank.seq = g_btl_voice_seq;
        BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        o->phase++;
        break;
    case 3:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        if (SsVabTransCompleted(0) == 0) {
            return;
        }
        BtlSePlay(BTL_BGM_SLOT, 0);
        g_btl_fx_gfx = (u_char *)SUMMON_GFX_STAGE;
        memcpy((u_char *)SUMMON_GFX_STAGE, D_80140004, SUMMON_GFX_BYTES);
        BtlBindGfx(SUMMON_GFX_KIND, SUMMON_GFX_INDEX, &g_btl_fx_gfx);
        BtlUploadTim((u_long *)g_load_stage, SUMMON_TIM_PAGE, SUMMON_TIM_SLOT,
                     1, 0, 1);
        g_btl_summon_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
        g_btl_summon_def.attr = 1;
        BtlObjAlloc(&g_btl_summon_def, SUMMON_GROUP, 0, SUMMON_DRAW, 0, &o->x,
                    SUMMON_TIM_PAGE, SUMMON_TIM_SLOT);
        i = 0;
        do {
            /* Written out inside the loop, which is what keeps the two-word
               constant in a register for the whole of it rather than being
               built again each time round. */
            attr = TRAIL_ATTR;
            motion = TRAIL_MOTION;
            trail = BtlObjClone(o);
            trail->angle = i << TRAIL_TURN;
            trail->fade = TRAIL_FADE;
            trail->mark_num = 0;
            trail->motion = motion;
            trail->rgb_to[0] = TRAIL_RGB;
            trail->rgb_to[1] = TRAIL_RGB;
            trail->rgb_to[2] = TRAIL_RGB_B;
            D_800F4894 = trail;
            trail->attr |= attr;
            i++;
        } while (i < TRAIL_RECORDS);
        BtlRefreshAttacks();
        o->timer = SUMMON_HOLD;
        o->phase++;
        break;
    case 4:
        if (o->timer != 0) {
            return;
        }
        scripts = &g_btl_member_scripts[SCRIPT_STAND
                                        + o->kind * MEMBER_SCRIPT_MODEL];
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
            scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        g_btl_scene_rgb[0] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[1] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[2] = SUMMON_SCENE_LIT;
        BtlSoundClose(BTL_BGM_SLOT);
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
