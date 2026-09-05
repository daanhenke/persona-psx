/* Persona 1 (JP) - accept-button checks with their click sounds.
 *
 * Compiled into DNG and ADV rather than called across the boundary:
 *   DNG @ 0x80092B80 / 0x80092BFC
 *   ADV @ 0x8008EAE0 / 0x8008EB5C
 *   S2D @ 0x80083094 / 0x80083110
 * S2D reads its own pad mask, which has a name of its own rather than sitting
 * at a fixed offset, so the TARGET_S2D guard below picks it.
 *
 * MenuPollInput calls both: A is the "confirm" edge and B the "repeat while
 * held" one. The `kind` argument only selects which click to play - the return
 * value is 1 whenever the button is down, sound or no sound.
 */
#include <types.h>
#include <persona/common/pad.h>

/* Which buttons count as accept this frame, ANDed against the live pad state.
   Both are slots of the binding table PadLoadBindings fills. */
/* newly-pressed edge; see PadPoll. S2D reads its own, at its own name. */
#ifdef TARGET_S2D
extern int     g_pad_pressed_s2d[];
#define g_pad_pressed g_pad_pressed_s2d
#else
extern int     g_pad_pressed[];
#endif

extern void SoundPlaySeq(u_short slot, u_short seq, short vab);

/* `kind` is a u_char here even though MenuPollInput was built against a
   prototype taking an int - the mask on the argument is in this function, not
   at the call site. */
u_char InputCheckAcceptA(u_char kind)
{
    int pressed;

    pressed = 0;
    if (g_pad_bindings[BIND_ACCEPT_A].mask & g_pad_pressed[0]) {
        switch (kind) {
        case 1:
            SoundPlaySeq(0x18, 2, 1);
            break;
        case 2:
            SoundPlaySeq(0x18, 4, 1);
            break;
        }
        pressed++;
    }
    return pressed;
}

/* The second mask, with its own pair of clicks. */
u_char InputCheckAcceptB(u_char kind)
{
    int pressed;

    pressed = 0;
    if (g_pad_bindings[BIND_ACCEPT_B].mask & g_pad_pressed[0]) {
        switch (kind) {
        case 1:
            SoundPlaySeq(0x18, 0, 1);
            break;
        case 2:
            SoundPlaySeq(0x18, 5, 1);
            break;
        }
        pressed++;
    }
    return pressed;
}
