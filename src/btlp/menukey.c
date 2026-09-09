/* Persona 1 (JP) - the one key a menu acts on this frame.  BTLP only.
 *   0x8009B5E8 BtlMenuKey
 *
 * A menu wants a single key rather than the whole pad, and it wants a held
 * direction to keep moving. So the twelve keys are walked in a fixed order -
 * the four directions, then the four buttons, then the shoulders - and the
 * first one newly pressed is handed back on its own, with the repeat counter
 * armed at fifteen frames.
 *
 * With nothing newly pressed the same walk runs over what is held instead,
 * counting the frames down; when the count runs past zero the key is handed
 * back again and the counter re-armed at three. So a held key repeats after a
 * quarter of a second and then four times a second.
 *
 * One counter serves all twelve, which is why the held walk decrements it for
 * whichever key it reaches first and stops there.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>

/* PadRead's bits, in the order the walk takes them. */
#define PAD_UP       0x1000
#define PAD_DOWN     0x4000
#define PAD_LEFT     0x8000
#define PAD_RIGHT    0x2000
#define PAD_TRIANGLE 0x0010
#define PAD_CROSS    0x0040
#define PAD_SQUARE   0x0080
#define PAD_CIRCLE   0x0020
#define PAD_L1       0x0004
#define PAD_L2       0x0001
#define PAD_R1       0x0008
#define PAD_R2       0x0002

/* Frames before a held key repeats, and between repeats after that. */
#define MENU_KEY_DELAY  15
#define MENU_KEY_REPEAT 3

extern signed char g_btl_menu_key_delay;

/* The counter is a signed byte: it is let run past zero and the sign is what
   says the wait is over. */
#define MENU_KEY_HELD(key)                                                    \
    if ((g_btl_pad1 & (key)) != 0) {                                          \
        g_btl_menu_key_delay--;                                               \
        if (g_btl_menu_key_delay < 0) {                                       \
            g_btl_menu_key_delay = MENU_KEY_REPEAT;                           \
            return (key);                                                     \
        }                                                                     \
    }

#define MENU_KEY_EDGE(key)                                                    \
    if ((g_btl_pad1_edge & (key)) != 0) {                                     \
        g_btl_menu_key_delay = MENU_KEY_DELAY;                                \
        return (key);                                                         \
    }

int BtlMenuKey(void)
{
    MENU_KEY_EDGE(PAD_UP)
    MENU_KEY_EDGE(PAD_DOWN)
    MENU_KEY_EDGE(PAD_LEFT)
    MENU_KEY_EDGE(PAD_RIGHT)
    MENU_KEY_EDGE(PAD_TRIANGLE)
    MENU_KEY_EDGE(PAD_CROSS)
    MENU_KEY_EDGE(PAD_SQUARE)
    MENU_KEY_EDGE(PAD_CIRCLE)
    MENU_KEY_EDGE(PAD_L1)
    MENU_KEY_EDGE(PAD_L2)
    MENU_KEY_EDGE(PAD_R1)
    MENU_KEY_EDGE(PAD_R2)

    MENU_KEY_HELD(PAD_UP)
    MENU_KEY_HELD(PAD_DOWN)
    MENU_KEY_HELD(PAD_LEFT)
    MENU_KEY_HELD(PAD_RIGHT)
    MENU_KEY_HELD(PAD_TRIANGLE)
    MENU_KEY_HELD(PAD_CROSS)
    MENU_KEY_HELD(PAD_SQUARE)
    MENU_KEY_HELD(PAD_CIRCLE)
    MENU_KEY_HELD(PAD_L1)
    MENU_KEY_HELD(PAD_L2)
    MENU_KEY_HELD(PAD_R1)
    MENU_KEY_HELD(PAD_R2)

    return 0;
}
