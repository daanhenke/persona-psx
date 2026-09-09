/* Persona 1 (JP) - running every object's animation script on.  BTLP only.
 *   0x8008A524 BtlStepObjScripts
 *
 * BtlDrawFrame calls this between BtlStepCluts and BtlWaveMesh. All six groups
 * are walked; a list head, or a record carrying 0x20000000 or 0x8, is left
 * alone. Everything else is marked running and its script stepped until an
 * opcode holds the frame instead of acting.
 *
 * The opcode is the high byte of a step's flags. Six of them act and move
 * straight on to the next step; the rest either stop the walk or, in the
 * common case, hold the object on this step until its frame counter reaches
 * the count the step names.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* The opcodes, in the order the walk tests them. */
#define SEQ_OP     0xFF00  /* the high byte is what names the opcode */
#define SEQ_SOUND  0xFC00  /* play bank and number from the two bytes */
#define SEQ_PLACE  0xFB00  /* put the attached record beside this one */
#define SEQ_HIDE   0xFA00  /* and hide it again                       */
#define SEQ_SPAWN  0xF900  /* start a child from the child template   */
#define SEQ_JUMP   0xFE00  /* the step's word is the next script      */
#define SEQ_FREE   0xFD00  /* free the object and its shadow          */
#define SEQ_STATIC 0xFF00  /* leave the object standing still         */

/* A step whose second word carries this is one the walk keeps going past. */
#define SEQ_HOLD 0x8000

/* Attribute bits this walk works with. */
#define BTL_OBJ_RUNNING 0x10000000
#define BTL_OBJ_STILL   0x20000000
#define BTL_OBJ_SKIP    0x00000008
#define BTL_OBJ_HIDDEN  0x40000000
#define BTL_OBJ_SOUNDED 0x04000000
#define BTL_OBJ_JUMPED  0x08000000
#define BTL_OBJ_NOSHIFT 0x00080000
#define BTL_OBJ_QUICK   0x00002000

/* Which of a script's own tables the child comes out of, and the group and
   arguments it is allocated with. */
#define SEQ_CHILD_A3   4
#define BTL_OBJ_PARENT 3

extern BtlObjDef  g_btl_obj_child_def;
extern u_char     g_btl_half_rate;
extern u_char     g_btl_se_off;
extern u_char     g_btl_seq_catchup;


#ifdef NON_MATCHING
void BtlStepObjScripts(void)
{
    BtlObj     *o;
    BtlSeqStep *s;
    int         group;
    int         more;
    int         fast;
    const u_long ***child;
    u_long      attr;
    u_long      nosound;
    u_long      nojump;

    group = 0;
    child = &g_btl_obj_child_def.scripts;
    do {
        for (o = (BtlObj *)((char *)g_btl_obj_pool
                            + g_btl_obj_first[group] * sizeof(BtlObj));
             o != 0; o = o->next) {
            if ((o->kind & BTL_OBJ_HEAD) != 0) {
                continue;
            }
            /* One read of the attributes serves both the skip test and the
               update below, and one write puts them back. */
            attr = o->attr;
            if ((attr & (BTL_OBJ_STILL | BTL_OBJ_SKIP)) != 0) {
                continue;
            }
            s = o->script;
            /* The two clear masks go through locals of their own: that is what
               builds both of them before the set, the way the original has it.
               Written inline gcc materialises each one where it is used. */
            nosound = ~BTL_OBJ_SOUNDED;
            nojump = ~BTL_OBJ_JUMPED;
            attr |= BTL_OBJ_RUNNING;
            attr &= nosound;
            attr &= nojump;
            o->attr = attr;
            do {
                more = 1;
                if ((s->flags & SEQ_OP) == SEQ_SOUND) {
                    if (g_btl_se_off == 0) {
                        o->attr |= BTL_OBJ_SOUNDED;
                        BtlSePlay(s->arg0, (short)s->arg1);
                    }
                    s++;
                } else if ((s->flags & SEQ_OP) == SEQ_PLACE) {
                    o->attached->attr &= ~BTL_OBJ_HIDDEN;
                    o->attached->x = (s->arg0 << 16) + o->x;
                    o->attached->y = (s->arg1 << 16) + o->y;
                    s++;
                } else if ((s->flags & SEQ_OP) == SEQ_HIDE) {
                    o->attached->attr |= BTL_OBJ_HIDDEN;
                    s++;
                } else if ((s->flags & SEQ_OP) == SEQ_SPAWN) {
                    *child = (const u_long **)o->scripts[o->children];
                    BtlObjAlloc((BtlObjDef *)(child - 1), o->group, o,
                                SEQ_CHILD_A3, 0, &o->x, o->unkCD, o->unkCE);
                    if (o->kind == BTL_OBJ_PARENT) {
                        o->children++;
                    }
                    s++;
                } else {
                    more = 0;
                    /* The remaining opcodes are matched whole rather than by
                       their high byte, which is what puts them in a switch of
                       their own. */
                    switch (s->flags) {
                    case SEQ_STATIC:
                        o->attr = (o->attr | BTL_OBJ_STILL) & ~BTL_OBJ_RUNNING;
                        break;

                    case SEQ_FREE:
                        BtlObjFree(o->shadow);
                        BtlObjFree(o);
                        break;

                    case SEQ_JUMP:
                        o->attr |= BTL_OBJ_JUMPED;
                        s = (BtlSeqStep *)s->value;
                        more = 1;
                        if ((s->flags & SEQ_HOLD) == 0) {
                            o->last = s->value;
                        }
                        o->step = 0;
                        break;

                    default:
                        if ((u_short)o->step + 1 >= (u_short)s->flags) {
                            s++;
                            if ((s->flags & SEQ_HOLD) == 0) {
                                o->last = s->value;
                                if ((o->attr & BTL_OBJ_NOSHIFT) == 0) {
                                    o->shift = s->arg1 << 16;
                                    o->shift_x = s->arg0 << 16;
                                }
                            } else {
                                more = 1;
                            }
                            fast = g_btl_half_rate != 0;
                            o->step = 0;
                            if (fast) {
                                if ((o->attr & BTL_OBJ_QUICK) != 0) {
                                    o->step = 1;
                                } else if (g_btl_seq_catchup != 0) {
                                    o->step = 1;
                                }
                            }
                        } else {
                            fast = g_btl_half_rate != 0;
                            o->last = s->value;
                            if (fast) {
                                o->step++;
                            }
                            o->step++;
                        }
                        break;
                    }
                }
            } while (more);
            o->script = s;
        }
        group++;
    } while (group <= BTL_OBJ_GROUPS - 1);
}
#else
INCLUDE_ASM("btlp/nonmatchings/stepscripts", BtlStepObjScripts);
#endif

