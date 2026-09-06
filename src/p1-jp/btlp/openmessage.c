/* Persona 1 (JP) - putting a message up with its box.  BTLP only.
 *   0x8007F258 BtlOpenMessage
 *
 * Two layouts out of one routine: a twelve-column box, or the seventeen-column
 * one that is as wide as there is a frame tile for. The box sits 0x68 or 0x90
 * below the x it is given and the text 0x18 below it either way, and both take
 * the same y.
 *
 * The narrow layout is centred as well. BtlTextOpen hands back how many
 * characters wide the message came out; sixteen pixels each, taken off 0xA0
 * and halved, is the nudge that centres it, and that is what the window's dx
 * is for.
 *
 * How the box opens follows from how it will close: one that slides shut ramps
 * its height open, and one that collapses grows wide and then tall. Either way
 * the fast-animation setting skips it and goes straight to full size.
 */
#include <types.h>
#include <persona/btlp/box.h>
#include <persona/btlp/window.h>

/* Bits of the first argument. */
#define MSG_NARROW  0x1  /* the twelve-column box rather than the wide one */
#define MSG_SLIDE   0x2  /* the box slides shut rather than collapsing     */
#define MSG_NEWPAGE 0x4  /* start the text page over                       */

/* The two boxes, and where each sits below the x it is given. */
#define MSG_NARROW_COLS 0xC
#define MSG_NARROW_DROP 0x68
#define MSG_WIDE_COLS   BTL_BOX_COLS_MAX
#define MSG_WIDE_DROP   0x90

/* The text sits here, and the y both share is this far down. */
#define MSG_TEXT_DROP 0x18
#define MSG_Y_DROP    8

/* What the narrow message is centred in, and how wide a character is. */
#define MSG_CENTRE_W 0xA0
#define MSG_CHAR_W   4    /* as a shift: sixteen pixels */

extern BtlWindow g_btl_text;
extern int       g_btl_text_page;
extern u_char    g_btl_fast_anim;

extern int  BtlTextOpen(const u_char *script, short x, short y);
extern void BtlBoxOpen(short cols, short x, short y, int style);

void BtlOpenMessage(int flags, short style, const u_char *script, short x,
                    short y)
{
    int cols;
    int mode;
    /* Eight bytes the original reserves and never touches. gcc 2.6 allocates
       a declared local whether or not it is used, and without these the frame
       comes out eight short. */
    int unused[2];

    /* The flags are copied before anything else so the copy survives the two
       calls; testing the argument directly leaves it in a call-clobbered
       register and costs a saved one. */
    mode = flags;
    if ((mode & MSG_NEWPAGE) != 0) {
        g_btl_text_page = 0;
    }
    if ((mode & MSG_NARROW) != 0) {
        cols = BtlTextOpen(script, x + MSG_TEXT_DROP, y + MSG_Y_DROP);
        BtlBoxOpen(MSG_NARROW_COLS, x + MSG_NARROW_DROP, y + MSG_Y_DROP,
                   style);
        /* The nudge is narrowed before it is added, not after. */
        g_btl_text.dx +=
            (u_short)((MSG_CENTRE_W - (cols << MSG_CHAR_W)) >> 1);
    } else {
        /* The wide layout wants a block of its own. Without the do/while the
           two calls share the narrow arm's and the registers come out
           differently. */
        do {
            BtlTextOpen(script, x + MSG_TEXT_DROP, y + MSG_Y_DROP);
            BtlBoxOpen(MSG_WIDE_COLS, x + MSG_WIDE_DROP, y + MSG_Y_DROP,
                       style);
        } while (0);
    }

    if ((mode & MSG_SLIDE) != 0) {
        g_btl_box_step = BTL_BOX_OPEN;
        g_btl_box_flags |= BTL_BOX_SLIDE;
    } else {
        g_btl_box_step = BTL_BOX_ZOOM;
        g_btl_box_flags &= ~BTL_BOX_SLIDE;
    }
    if (g_btl_fast_anim != 0) {
        g_btl_box_step = BTL_BOX_OPEN_NOW;
    }
}
