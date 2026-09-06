/* Persona 1 (JP) - getting the command menu out of the way.  BTLP only.
 *   0x8006A170 BtlMenuAsideToggle
 *
 * The command menu covers most of the lower screen, so cancel pushes it aside
 * to leave the fight visible and confirm brings it back. Which button is read
 * depends on which way the menu is already sitting, and the flag only flips on
 * a press - the two halves are otherwise the same shape.
 *
 * The indicator changes with it: a bar while the menu is away, an icon while
 * it is up.
 */
#include <types.h>

/* The sequencer state each direction leaves behind, and how long it takes. */
#define BTL_SEQ_MENU_BACK  4
#define BTL_SEQ_MENU_ASIDE 6
#define BTL_SEQ_MENU_TIME  0xC

extern int    g_btl_menu_aside;
extern u_short g_btl_key_confirm;
extern u_short g_btl_key_cancel;

extern u_long BtlInputKeys(void);
extern void   BtlIndicatorIcon(void);
extern void   BtlIndicatorBar(void);
extern void   BtlSeqSetState(int state, int frames);
extern void   BtlMenuReturn(void);
extern void   BtlMenuPushAside(void);

void BtlMenuAsideToggle(void)
{
    if (g_btl_menu_aside != 0) {
        if ((BtlInputKeys() & g_btl_key_confirm) == 0) {
            return;
        }
        BtlIndicatorIcon();
        BtlSeqSetState(BTL_SEQ_MENU_BACK, BTL_SEQ_MENU_TIME);
        BtlMenuReturn();
    } else {
        if ((BtlInputKeys() & g_btl_key_cancel) == 0) {
            return;
        }
        BtlIndicatorBar();
        BtlSeqSetState(BTL_SEQ_MENU_ASIDE, BTL_SEQ_MENU_TIME);
        BtlMenuPushAside();
    }
    g_btl_menu_aside ^= 1;
}
