/* Persona 1 (JP) - the move whose one record stands in the middle of the
 * field.  BTLP only.
 *   0x800BD330 BtlFxStart72
 *   0x800BD3A8 BtlFxStep72
 *   0x800BD628 BtlFxStep6E
 *
 * A start handler out of g_btl_spell_fx. One record on g_btl_fx_centre - the
 * middle of the field, sixty-four units toward the camera - drawn and stepped
 * from the frame it is taken and carrying only the bit that says it has no
 * shadow. Everything else about it comes from its script.
 *
 * The step is the other half of the move that sends the enemy side away, and
 * it is written the same way BtlFxStepE8 is. Once the record's script has run
 * out it whitens the opening object over the whole field, shuts the field's
 * music down and reopens the voices, and rolls: one time in four the enemies
 * leave, and a fight that may not be run from or a mood of nought never does.
 * Every enemy still standing goes on the leaving motion two frames apart from
 * the one before with the trail bit raised, and the record then holds while
 * they go, takes the white back down, and closes the five voice slots.
 *
 * Move 0x6E's step is in this unit rather than beside its own start handler,
 * which is where the image has it. It is a different effect entirely: the head
 * grows to full size while it spins, opens a single piece of artwork above
 * itself out of the record's second script and walks that piece up out of
 * grey, and then the pair fade out together and the hit is armed. The piece
 * itself sinks onto the field and then opens a spark every fourth frame on one
 * of the eight cells around the middle, and each spark frees itself once its
 * own script has played out.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The page the field is drawn through while the white is up, and how fast the
   opening object walks to it. */
#define FX_72_PAGE 0x20
#define FX_72_FADE 8

/* The music slot the field's own track is on, and the five voice slots. */
#define FX_72_MUSIC  3
#define FX_72_VOICE  7
#define FX_72_VOICES 5

/* One roll in this many sends the enemies away. */
#define FX_72_ODDS 4

/* The motion an enemy leaves on, the bit it is given as it goes, and how much
   later each one starts. */
#define FX_72_MOTION  0xE
#define FX_72_GOING   0x800
#define FX_72_STAGGER 2

/* How long the record holds while they go, and how long the white then takes
   to come back down. */
#define FX_72_HOLD   0x3C
#define FX_72_SETTLE 0x1E

BtlObj *BtlFxStart72(void)
{
    BtlObj *o;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_centre, FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->attr |= BTL_OBJ_NO_SHADOW;
    return o;
}

/* 99.88%: the two induction pointers the enemy walk is reduced to are
   incremented in the other order - the image steps the enemy's before the
   actor's, and this steps the actor's first.

   The loop dump says why. The walk has two induction variables, `slot` and
   `k`, and loop optimisation gives each its own class: the actor addresses
   are reduced against `slot` and the enemy ones against `k`. Each class's
   update is planted in front of its own variable's increment, so the order
   of the two updates is the order of `slot++` and `k++` in the source -
   while the order the two are *set up* in is the class list, which is the
   reverse of that. The image has both in the same order, which one class
   would give and two never can: writing the enemy side against `slot` as
   well does put them in step, but then the actor addresses are combined onto
   the enemy side's own multiply and lose the 0x49C their register starts at.
   BtlFxStepE8's walk is the same shape and stops in the same place. */
#ifdef NON_MATCHING
void BtlFxStep72(BtlObj *o)
{
    BtlObj *obj;
    int     i;
    int     k;
    int     slot;

    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        /* Taken into a local first: the page below is a global too, and
           storing to it would otherwise make the object be read again for
           every field. */
        obj = g_btl_intro_obj;
        g_btl_tpage[0]  = FX_72_PAGE;
        obj->fade       = FX_72_FADE;
        obj->rgb[0]     = 0;
        obj->rgb[1]     = 0;
        obj->rgb[2]     = 0;
        obj->rgb_to[0]  = FX_WHITE;
        obj->rgb_to[1]  = FX_WHITE;
        obj->rgb_to[2]  = FX_WHITE;
        g_btl_intro_obj->attr &= ~BTL_OBJ_HIDDEN;
        BtlSoundClose(FX_72_MUSIC);
        BtlFxReopenVoices();
        o->children = rand() % FX_72_ODDS;
        if (g_btl_ai_set == 0 || g_btl_no_escape != 0) {
            o->children = 1;
        }
        if (o->children == 0) {
            for (slot = BTL_PARTY, i = 0, k = 0; slot < BTL_ACTORS;
                 slot++, k++) {
                if (g_btl_actors[slot].c.key == 0) {
                    continue;
                }
                if ((signed char)g_btl_actors[slot].c.status
                        == BTL_STATUS_DOWN) {
                    continue;
                }
                if ((g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0) {
                    continue;
                }
                g_btl_enemies[k].obj->motion = FX_72_MOTION;
                g_btl_enemies[k].obj->timer  = i * FX_72_STAGGER;
                i++;
                g_btl_enemies[k].obj->attr |= FX_72_GOING;
            }
        }
        o->timer = FX_72_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        g_btl_intro_obj->rgb_to[0] = 0;
        g_btl_intro_obj->rgb_to[1] = 0;
        g_btl_intro_obj->rgb_to[2] = 0;
        o->timer = FX_72_SETTLE;
        o->phase++;
        break;
    case 2:
        if (o->timer != 0) {
            break;
        }
        g_btl_intro_obj->attr |= BTL_OBJ_HIDDEN;
        for (slot = 0; slot < FX_72_VOICES; slot++) {
            BtlSoundClose(slot + FX_72_VOICE);
        }
        o->motion = 0;
        o->phase  = 0;
        break;
    default:
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell72", BtlFxStep72);
#endif


/* How fast the head grows and how big it stops - the two scales are unity at
   0x100, so it grows an eighth of a turn's worth a frame and ends about forty
   times life size. */
#define FX_6E_GROW 0x80
#define FX_6E_FULL 10000

/* What the head turns through each frame it is stepped. */
#define FX_6E_SPIN 0x20

/* The piece the head opens above itself: what it starts as, how far above the
   head it stands, and the two slots whose page it borrows. */
#define FX_6E_PIECE_ATTR (BTL_OBJ_STATIC | 3)
#define FX_6E_ABOVE      0x200000
#define FX_6E_SLOT       0x1D
#define FX_6E_SLOT2      0x1E

/* What the piece carries: the mark that tells it apart, the motion it runs,
   and how fast it walks up out of black. */
#define FX_6E_PIECE_MARK   1
#define FX_6E_PIECE_MOTION 2
#define FX_6E_PIECE_FADE   1

/* How long the pair stand there, and how fast they fade out again. */
#define FX_6E_STAND 0xB4
#define FX_6E_FADE  4

/* How far the piece sinks each frame on its way onto the field. */
#define FX_6E_DROP 0x8000

/* One spark every fourth frame, on one of eight cells, and what a spark
   starts as and is marked with. */
#define FX_6E_EVERY      3
#define FX_6E_CELLS      7
#define FX_6E_SPARK_ATTR BTL_OBJ_NO_SHADOW
#define FX_6E_SPARK_MARK 2

/* A cell's column and row are worth the same as the ring's are, and the grid
   stands the same distance left of and above the middle; the sparks are sixty
   more units toward the camera than the field itself. */
#define FX_6E_COL_W 15
#define FX_6E_ROW_H 20
#define FX_6E_LEFT  0x3C
#define FX_6E_HIGH  0x8C
#define FX_6E_DEPTH -0x480000

void BtlFxStep6E(BtlObj *o)
{
    BtlObj *piece;
    long    pos[3];
    long    z;

    switch (o->mark_num) {
    case 0:
        switch (o->phase) {
        case 0:
            o->scale_x += FX_6E_GROW;
            o->scale_y += FX_6E_GROW;
            if (o->scale_x < FX_6E_FULL) {
                break;
            }
            o->phase++;
            break;
        case 1:
            g_btl_fx_def2.attr    = FX_6E_PIECE_ATTR;
            g_btl_fx_def2.scripts = (const u_long **)o->scripts[1];
            pos[0] = o->x;
            pos[1] = o->y;
            /* The depth is taken before the page below is written: that store
               is to a global too, and leaving it in front would make the
               record be read again for it. */
            z = o->z;
            g_btl_tpage[FX_6E_SLOT2] =
                g_btl_tpage[FX_6E_SLOT] & ~BTL_TPAGE_BLEND;
            pos[2] = z + FX_6E_ABOVE;
            piece = BtlObjAlloc(&g_btl_fx_def2, FX_OBJ_GROUP, 0, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
            o->unk54 = (long)piece;
            piece->kind = o->kind;
            ((BtlObj *)o->unk54)->scripts   = o->scripts;
            ((BtlObj *)o->unk54)->mark_num  = FX_6E_PIECE_MARK;
            ((BtlObj *)o->unk54)->fade      = FX_6E_PIECE_FADE;
            ((BtlObj *)o->unk54)->motion    = FX_6E_PIECE_MOTION;
            ((BtlObj *)o->unk54)->rgb_to[0] = FX_GREY;
            ((BtlObj *)o->unk54)->rgb_to[1] = FX_GREY;
            ((BtlObj *)o->unk54)->rgb_to[2] = FX_GREY;
            ((BtlObj *)o->unk54)->rgb[0]    = 0;
            ((BtlObj *)o->unk54)->rgb[1]    = 0;
            ((BtlObj *)o->unk54)->rgb[2]    = 0;
            o->timer = FX_6E_STAND;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            ((BtlObj *)o->unk54)->rgb_to[0] = 0;
            ((BtlObj *)o->unk54)->rgb_to[1] = 0;
            ((BtlObj *)o->unk54)->rgb_to[2] = 0;
            o->fade = FX_6E_FADE;
            ((BtlObj *)o->unk54)->fade = FX_6E_FADE;
            o->phase++;
            break;
        case 3:
            if (o->rgb[0] != 0) {
                break;
            }
            BtlObjFree((BtlObj *)o->unk54);
            o->phase = FX_STEP_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            BtlArmHitChain();
            break;
        default:
            BtlFxFinish01(o);
            break;
        }
        o->rot.vz += FX_6E_SPIN;
        break;
    case FX_6E_PIECE_MARK:
        switch (o->phase) {
        case 0:
            o->z -= FX_6E_DROP;
            if (o->z > 0) {
                break;
            }
            o->z = 0;
            o->phase++;
            o->attr &= ~BTL_OBJ_STATIC;
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            if ((g_btl_tick & FX_6E_EVERY) != 0) {
                break;
            }
            g_btl_fx_def2.attr    = FX_6E_SPARK_ATTR;
            g_btl_fx_def2.scripts = (const u_long **)o->scripts[2];
            /* The doubling is load-bearing as a shift: written as a
               multiply it costs eight bytes of frame nothing reaches. */
            pos[0] = (g_btl_fx_ring_cells2[(o->steps & FX_6E_CELLS) << 1]
                          * FX_6E_COL_W - FX_6E_LEFT) << 16;
            pos[1] = (g_btl_fx_ring_cells2[((o->steps & FX_6E_CELLS) << 1) + 1]
                          * FX_6E_ROW_H - FX_6E_HIGH) << 16;
            pos[2] = FX_6E_DEPTH;
            piece = BtlObjAlloc(&g_btl_fx_def2, FX_OBJ_GROUP, 0, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
            o->unk54 = (long)piece;
            piece->kind = o->kind;
            ((BtlObj *)o->unk54)->mark_num = FX_6E_SPARK_MARK;
            o->steps++;
            break;
        default:
            break;
        }
        break;
    case FX_6E_SPARK_MARK:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        BtlObjFree(o);
        break;
    default:
        break;
    }
}
