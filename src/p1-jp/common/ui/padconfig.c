/* Persona 1 (JP) - the buttons that page a list.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x80085420   ADV 0x800768A4   S2D 0x8007585C
 *
 * Applied together with the binding table (0x8008BFF8) everywhere the pad
 * configuration is set or restored. The reader ands g_btn_page_back with the
 * held buttons and negates the step, so back and forward are what the two
 * masks mean.
 */
#include <types.h>

/* Shoulder buttons, in the order libetc reports them. */
#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008

extern u_short g_btn_page_back;
extern u_short g_btn_page_fwd;

/* Layout 0 pages with the two left shoulders against the two right ones;
   layout 1 puts both on the right hand. Any other value leaves the current
   masks alone. */
void PadSetPageButtons(u_char config)
{
    switch (config) {
    case 0:
        g_btn_page_back = PAD_L1 | PAD_L2;
        g_btn_page_fwd = PAD_R1 | PAD_R2;
        break;
    case 1:
        g_btn_page_back = PAD_R1;
        g_btn_page_fwd = PAD_R2;
        break;
    }
}
