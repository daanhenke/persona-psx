/* Persona 1 (JP) - S2D's copy of PadSetPageButtons.  S2D @ 0x8007585C.
 *
 * The worldmap keeps its work area 0x20000 above the one DNG and ADV share,
 * so the two masks live at different addresses and this needs its own copy.
 */
#include <types.h>

/* Shoulder buttons, in the order libetc reports them. */
#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008

extern u_short g_btn_page_back_s2d;
extern u_short g_btn_page_fwd_s2d;

/* Layout 0 pages with the two left shoulders against the two right ones;
   layout 1 puts both on the right hand. Any other value leaves the current
   masks alone. */
void PadSetPageButtons(u_char config)
{
    switch (config) {
    case 0:
        g_btn_page_back_s2d = PAD_L1 | PAD_L2;
        g_btn_page_fwd_s2d = PAD_R1 | PAD_R2;
        break;
    case 1:
        g_btn_page_back_s2d = PAD_R1;
        g_btn_page_fwd_s2d = PAD_R2;
        break;
    }
}
