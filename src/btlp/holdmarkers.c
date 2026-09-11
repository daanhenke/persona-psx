/* Persona 1 (JP) - the pause a scripted fight takes mid-round.  BTLP only.
 *   0x8009A034 BtlHoldForMarkers
 *
 * Nothing at all unless something has raised the flag. When it has, the
 * markers are pulled in, two seconds pass, the HUD is taken out of the
 * drawing pass, and four of the six encounters that can reach this put a line
 * up - one of them shared by two of them, and one of the six has no line at
 * all. The markers then go back up, and the routine waits out whatever the
 * HUD object was still doing before handing the round back.
 *
 * The switch is on the encounter biased by eight and read as a short, which
 * is what puts the range test on sltiu and keeps the six arms on a jump
 * table; the arm with no line is still written out, because leaving it out
 * would move the bound.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* Frames the field is left alone once the markers are in, and again once
   they are back. */
#define HOLD_DELAY  0x78
#define HOLD_SETTLE 60

/* Where the encounters with a line of their own start. */
#define HOLD_FIRST_ENCOUNTER 8

/* Nobody speaks any of them. */
#define HOLD_VOICE_NOBODY 0

extern u_char  g_btl_line_hold08[];
extern u_char  g_btl_line_hold09[];
extern u_char  g_btl_line_hold10[];
extern u_char  g_btl_line_hold13[];

/* The object the pause waits on, and the flag it raises for whatever comes
   next. */
extern BtlObj *D_800F5B18;
extern u_char  D_800CCA31;


void BtlHoldForMarkers(void)
{
    int i;

    if (g_btl_hold_markers == 0) {
        return;
    }

    BtlRetractMarkers();
    g_btl_delay = HOLD_DELAY;
    while (g_btl_delay != 0) {
        BtlDrawFrame();
    }
    if (g_btl_hud_obj != NULL) {
        BtlObjSetAttr(g_btl_hud_obj, BTL_OBJ_HIDDEN);
    }

    switch ((short)(g_btl_encounter - HOLD_FIRST_ENCOUNTER)) {
    case 0:
        BtlPlayScene(HOLD_VOICE_NOBODY, g_btl_line_hold08);
        break;
    case 1:
        BtlPlayScene(HOLD_VOICE_NOBODY, g_btl_line_hold09);
        break;
    case 2:
    case 3:
        BtlPlayScene(HOLD_VOICE_NOBODY, g_btl_line_hold10);
        break;
    case 4:
        break;
    case 5:
        BtlPlayScene(HOLD_VOICE_NOBODY, g_btl_line_hold13);
        break;
    }

    BtlRefreshMarkers();
    i = 0;
    do {
        i++;
        BtlDrawFrame();
    } while (i < HOLD_SETTLE);

    D_800F5B18->phase = 0;
    D_800CCA31 = 1;
    g_btl_hold_markers = 0;
    while (D_800F5B18->motion != 0) {
        BtlDrawFrame();
    }
}
