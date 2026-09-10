/* Persona 1 (JP) - one frame of the choice box.  BTLP only.
 *   0x8007D9C4 BtlMenuUpdate
 *
 * Only the live state takes the pad: up and down walk the entries and wrap,
 * confirm answers with the entry, cancel answers with -1 and leaves the box
 * where it is. Everything else the caller sees comes back in
 * g_btl_menu_choice, which is put back to BTL_MENU_WAIT every frame so a
 * choice is read exactly once.
 *
 * The other three states are the slides, four pixels a frame: in from the
 * left, out to the side by 0x30 so a message can be read past the box, and
 * away again for good.
 */
#include <decomp/types.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* Where the cursor sits on an entry: the cell's column in eight-pixel steps
   from the box's left edge, and its row straight down from the box's top. */
#define MENU_CURSOR_X0   0x10
#define MENU_CURSOR_STEP 8
#define MENU_CURSOR_Y0   0xAE

/* Four pixels a frame, and how far aside the box goes. */
#define MENU_SLIDE_STEP  4
#define MENU_SLIDE_ASIDE 0x30

/* The clicks. */
#define MENU_SE_BANK    1
#define MENU_SE_MOVE    0
#define MENU_SE_CONFIRM 1
#define MENU_SE_CANCEL  2

void BtlMenuUpdate(void)
{
    /* Each of these is worked on through a pointer of its own, which is what
       puts its address in a register for the whole block. */
    int* choice;
    int* up;
    int* down;
    int* slide;

    g_btl_menu_choice = BTL_MENU_WAIT;
    switch (g_btl_menu_state)
    {
        case BTL_MENU_SHUT:
            break;

        case BTL_MENU_IDLE:
            break;

        case BTL_MENU_LIVE:
            if ((BtlInputKeys() & g_btl_key_up) != 0)
            {
                BtlSePlay(MENU_SE_BANK, MENU_SE_MOVE);
                up = &g_btl_menu_index;
                (*up)--;
                if (*up < 0)
                {
                    *up = g_btl_menu_count - 1;
                }
            }
            if ((BtlInputKeys() & g_btl_key_down) != 0)
            {
                BtlSePlay(MENU_SE_BANK, MENU_SE_MOVE);
                down = &g_btl_menu_index;
                (*down)++;
                if (*down >= g_btl_menu_count)
                {
                    *down = 0;
                }
            }
            choice  = &g_btl_menu_choice;
            *choice = BTL_MENU_WAIT;
            if ((g_btl_pad1_edge & g_btl_key_cancel) != 0)
            {
                BtlSePlay(MENU_SE_BANK, MENU_SE_CANCEL);
                *choice = -1;
            }
            else if ((g_btl_pad1_edge & g_btl_key_confirm) != 0)
            {
                BtlCursorShow(0);
                BtlSePlay(MENU_SE_BANK, MENU_SE_CONFIRM);
                g_btl_menu_state = BTL_MENU_IDLE;
                *choice          = g_btl_menu_index;
            }
            BtlCursorPlace(g_btl_menu_cells[g_btl_menu_index].x * MENU_CURSOR_STEP + MENU_CURSOR_X0,
                           g_btl_menu_cells[g_btl_menu_index].y + MENU_CURSOR_Y0);
            break;

        case BTL_MENU_SLIDE_IN:
            slide   = &g_btl_menu_slide;
            *slide -= MENU_SLIDE_STEP;
            if (*slide < 0)
            {
                *slide                  = 0;
                g_btl_menu_state        = BTL_MENU_LIVE;
                g_btl_menu_slide_frames = 0;
                BtlCursorShow(1);
            }
            break;

        case BTL_MENU_SLIDE_ASIDE:
            slide   = &g_btl_menu_slide;
            *slide += MENU_SLIDE_STEP;
            if (*slide >= MENU_SLIDE_ASIDE)
            {
                *slide           = MENU_SLIDE_ASIDE;
                g_btl_menu_state = BTL_MENU_IDLE;
            }
            break;

        case BTL_MENU_SLIDE_OUT:
            slide   = &g_btl_menu_slide;
            *slide -= MENU_SLIDE_STEP;
            if (*slide < -(MENU_SLIDE_ASIDE - 1))
            {
                g_btl_menu_state = BTL_MENU_SHUT;
            }
            break;
    }
}
