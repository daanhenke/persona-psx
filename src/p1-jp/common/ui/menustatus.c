/* Persona 1 (JP) - the field menu's status page.  ADV, DNG and S2D.
 *   ADV 0x80071ED0, DNG 0x80080C08, S2D 0x80070E88
 *
 * The menu loop runs one page per frame by g_menu_sel and this is its case 5.
 * The page has three states of its own: the build runs once and steps itself
 * on, then the chooser walks the party, and accepting a member reads that
 * character's picture off the disc and draws the sheet, which the third state
 * owns until it is cancelled.
 */
#include <types.h>

/* The page's own state, within the menu's. */
#define STATUS_BUILD  0
#define STATUS_CHOOSE 1
#define STATUS_SHEET  2

extern short g_menu_subsel;

extern void MenuStatusOpen(void);
extern void MenuStatusSelect(void);
extern void MenuStatusView(void);

void MenuStatusTick(void)
{
    switch (g_menu_subsel) {
    case STATUS_BUILD:
        MenuStatusOpen();
        g_menu_subsel++;
        break;
    case STATUS_CHOOSE:
        MenuStatusSelect();
        break;
    case STATUS_SHEET:
        MenuStatusView();
        break;
    }
}
