/* Persona 1 (JP) - the battle's choice box.  BTLP only.
 *   0x8007D8F8 BtlMenuDismiss   0x8007D924 BtlMenuReturn
 *   0x8007D944 BtlMenuPushAside 0x8007D97C BtlMenuChoice
 *   0x8007D98C BtlMenuState     0x8007D99C BtlMenuHide
 *
 * A short vertical list the player picks from - fight or run, yes or no. It is
 * drawn on one line of its own, at 0xAC, and g_btl_menu_slide displaces it from
 * there: BtlMenuUpdate walks that four pixels a frame, so 0x30 either way takes
 * twelve frames to put the box off the working area or bring it back.
 *
 * The four calls here are the ways out of the box and back into it. Callers
 * poll BtlMenuState until the box has settled and BtlMenuChoice for the answer,
 * which stays at BTL_MENU_WAITING until the player gives one.
 */
#include <types.h>

/* 3, 4 and 5 are the three slides; each ends in the resting phase it is
   heading for. */
#define BTL_MENU_GONE   0
#define BTL_MENU_IDLE   1   /* on its line, ignoring the pad */
#define BTL_MENU_LIVE   2   /* on its line, taking the pad   */
#define BTL_MENU_RETURN 3   /* sliding back to its line      */
#define BTL_MENU_ASIDE  4   /* sliding down out of the way   */
#define BTL_MENU_OFF    5   /* sliding up and away           */

/* Four pixels a frame over 0x30 - twelve frames either way. */
#define BTL_MENU_SLIDE_FRAMES 0xC

extern int g_btl_menu_choice;
extern int g_btl_menu_slide_frames;
extern int g_btl_menu_state;

extern void   BtlCursorShow(int on);
extern u_long BtlInputKeys(void);
extern void   BtlDrawFrame(void);
extern void   BtlSePlay(int bank, int sound);

/* Nothing to choose from, only an acknowledgement: hold the screen until the
   player presses something, then the confirm sound and five more frames so it
   is heard before whatever comes next. */
void BtlWaitAnyKey(void)
{
    int i;

    /* The count is set up here rather than beside its loop, and that is
       load-bearing: gcc 2.6 will not carry the constant across the wait, so
       from down there it has to decrement and test rather than fold. */
    i = 5;
    while (BtlInputKeys() == 0) {
        BtlDrawFrame();
    }
    BtlSePlay(1, 1);
    while (--i != -1) {
        BtlDrawFrame();
    }
}

void BtlMenuDismiss(void)
{
    g_btl_menu_state = BTL_MENU_OFF;
    BtlCursorShow(0);
}

/* Nothing reads the frame count - BtlMenuUpdate watches the displacement
   instead - but both slides that end on the box's own line still set it. */
void BtlMenuReturn(void)
{
    g_btl_menu_state = BTL_MENU_RETURN;
    g_btl_menu_slide_frames = BTL_MENU_SLIDE_FRAMES;
}

void BtlMenuPushAside(void)
{
    g_btl_menu_state = BTL_MENU_ASIDE;
    g_btl_menu_slide_frames = BTL_MENU_SLIDE_FRAMES;
    BtlCursorShow(0);
}

int BtlMenuChoice(void)
{
    return g_btl_menu_choice;
}

int BtlMenuState(void)
{
    return g_btl_menu_state;
}

void BtlMenuHide(void)
{
    g_btl_menu_state = BTL_MENU_GONE;
    BtlCursorShow(0);
}
