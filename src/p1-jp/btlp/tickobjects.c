/* Persona 1 (JP) - one frame for every display object.  BTLP only.
 *   0x8008A910 BtlTickObjects
 *
 * Each of the six groups is a linked list with a head record at the front, and
 * each has a handler of its own in g_btl_obj_tick. The head is skipped - it is
 * the one record carrying BTL_OBJ_HEAD - and everything after it gets the
 * handler, an age up and a timer down, in that order.
 *
 * The tail of the function is the opening camera move: three coordinates walk
 * in together while the distance closes, and the move ends the frame the
 * distance arrives. The object it fades out is hidden as soon as its red has
 * reached zero, so it leaves rather than sitting there drawn black.
 */
#include <types.h>
#include <persona/btlp/object.h>

/* The phase the opening move runs in. */
#define BTL_INTRO_MOVING 4

/* Where the camera stops, and how fast it gets there. */
#define BTL_INTRO_END   0x200
#define BTL_INTRO_CLOSE 8
#define BTL_INTRO_STEP  0xD0

/* Hidden, the same bit every object is hidden with. */
#define BTL_OBJ_HIDDEN 0x40000000

extern BtlObj  *g_btl_obj_pool;
extern u_short  g_btl_obj_first[];
extern void   (*g_btl_obj_tick[])(BtlObj *obj);

extern u_char  g_btl_intro_step;
extern short   g_btl_intro_dist;
extern int     g_btl_intro_x;
extern int     g_btl_intro_y;
extern int     g_btl_intro_z;
extern BtlObj *g_btl_intro_obj;

void BtlTickObjects(void)
{
    void  (**tick)(BtlObj *obj);
    void  (**handler)(BtlObj *obj);
    u_short *first;
    BtlObj  *obj;
    int      group;
    /* Both of these are reached through a pointer: each is read and written
       in the same breath and the original works the address out once. */
    short   *dist;
    int     *x;

    group = 0;
    tick = g_btl_obj_tick;
    first = g_btl_obj_first;
    do {
        obj = (BtlObj *)((char *)g_btl_obj_pool + *first * sizeof(BtlObj));
        if (obj != 0) {
            handler = tick;
            do {
                if ((obj->kind & BTL_OBJ_HEAD) == 0) {
                    (*handler)(obj);
                    obj->age++;
                    if (obj->timer > 0) {
                        obj->timer--;
                    }
                }
                obj = obj->next;
            } while (obj != 0);
        }
        tick++;
        group++;
        first++;
    } while (group < BTL_OBJ_GROUPS);

    if (g_btl_intro_step == BTL_INTRO_MOVING) {
        if (g_btl_intro_obj->rgb[0] == 0) {
            g_btl_intro_obj->attr |= BTL_OBJ_HIDDEN;
        }
        dist = &g_btl_intro_dist;
        if (*dist == BTL_INTRO_END) {
            g_btl_intro_step = 0;
        } else {
            x = &g_btl_intro_x;
            *dist = *dist - BTL_INTRO_CLOSE;
            *x = *x - BTL_INTRO_STEP;
            g_btl_intro_y -= BTL_INTRO_STEP;
            g_btl_intro_z -= BTL_INTRO_STEP;
        }
    }
}
