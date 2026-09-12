/* Persona 1 (JP) - the move drawn as three of another move's effect in a row.
 * BTLP only.
 *   0x800BC830 BtlFxStart56
 *
 * A start handler out of g_btl_spell_fx that builds nothing of its own: it
 * calls move 0x33's handler three times and threads the three records onto
 * one another through `attached`, so the whole row is freed together. They are
 * laid out in depth two units apart, each one marked and started a little
 * later than the last, and the nearest of the three is the one the row is
 * scaled through and the one the table is answered with.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* How far apart in depth the three records stand - 16.16, so twenty units.
   The last one opened stands on the field and the two before it behind it. */
#define FX_56_GAP 0x140000

/* How much later each record behind the first one starts, in frames. */
#define FX_56_STAGGER 4

/* How wide and how deep the row is drawn: unity is 0x100 across and 0x1000
   through, so it stands thirty-one times its own width. */
#define FX_56_SCALE   0x1F40
#define FX_56_SCALE_Z 0x1000

/* How many records the row is made of, counted down. */
#define FX_56_LAST 2

BtlObj *BtlFxStart56(void)
{
    BtlObj *o;
    BtlObj *prev;
    int     i;
    int     wait;

    i = FX_56_LAST;
    prev = NULL;
    do {
        o = BtlFxStart33();
        o->attached = prev;
        prev = o;
        o->z = -i * FX_56_GAP;
        wait = i * FX_56_STAGGER;
        o->mark_num = i + FX_MARK_HEAD;
        o->timer = wait;
    } while (--i >= 0);
    BtlObjSetScale(o, FX_56_SCALE, FX_56_SCALE, FX_56_SCALE_Z);
    return o;
}
