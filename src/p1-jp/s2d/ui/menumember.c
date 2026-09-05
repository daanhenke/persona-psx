/* Persona 1 (JP) - moving between party members on a menu screen.
 *
 *   S2D 0x8007BD08.  Its pad state is a second copy at 0x800FC000, which is
 *   the only reason this is not src/p1-jp/common/ui/menumember.c.
 *
 * The portraits sit two, two and one, so the vertical cursor swaps 0 with 2 and
 * 1 with 3 while the horizontal one walks 0-1, 2-3 and 3-4. Callers pass
 * g_party_last, which is what keeps the cursor off members the party does not
 * have; holding Square jumps straight to that last member.
 */
#include <types.h>
#include <persona/common/menuctx.h>

extern int  g_pad_held_s2d[];
extern void SoundPlaySeq(int seq, int a, int b);

#define PAD_SQUARE 0x8000
#define MOVE_SEQ   0x18

short MenuStepMember(int *sel, u_char last)
{
    short  moved;
    u_char dir;

    moved = 0;
    dir = 0;
    if (MenuStepCursor(&g_menu->list[2])) {
        dir = 1;
        moved = 1;
    }
    if (MenuStepCursor(&g_menu->list[3])) {
        dir = 2;
        moved = 1;
    }
    switch (dir) {
    case 1:
        SoundPlaySeq(MOVE_SEQ, 1, 1);
        switch (*sel) {
        case 0:
            if (last < 2) {
                return moved;
            }
            *sel = 2;
            break;
        case 1:
            if (last < 3) {
                return moved;
            }
            *sel = 3;
            break;
        case 2:
            *sel = 0;
            break;
        case 3:
            *sel = 1;
            break;
        }
        break;
    case 2:
        SoundPlaySeq(MOVE_SEQ, 1, 1);
        switch (*sel) {
        case 0:
            if (last == 0) {
                return moved;
            }
            *sel = 1;
            break;
        case 1:
            *sel = 0;
            break;
        case 2:
            if (g_pad_held_s2d[0] & PAD_SQUARE) {
                *sel = last;
                break;
            }
            if (last < 3) {
                return moved;
            }
            *sel = 3;
            break;
        case 3:
            if (g_pad_held_s2d[0] & PAD_SQUARE) {
                *sel = 2;
                break;
            }
            if (last < 4) {
                *sel = 2;
                break;
            }
            *sel = 4;
            break;
        case 4:
            if (g_pad_held_s2d[0] & PAD_SQUARE) {
                *sel = 3;
                break;
            }
            *sel = 2;
            break;
        }
        break;
    }
    return moved;
}
