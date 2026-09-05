/* Persona 1 (JP) - BTLP overlay @ 0x800812A8
 *
 * Uploads a TIM from main memory into VRAM and records what the GPU will need
 * to draw with it.
 *
 * The caller names a page rather than a position: pages 0..15 sit along the
 * top of VRAM at 64-pixel intervals, and page 0x10 and up are the same columns
 * 256 lines further down. An image wider than one page needs a second tpage
 * word to reach past 0x80, and one wider than two needs a third, so those land
 * two and four entries further along g_btl_tpage - a caller uploading a wide
 * image has to leave those pages free.
 *
 * CLUTs go in the bottom two rows of VRAM, at 0x1E0 + (slot & 0x1F), sixteen
 * pixels apart across. The caller says how many consecutive rows the file
 * holds; each one's GetClut word is recorded and the rectangle walks down.
 *
 * Returns where the CLUT was read from, which is zero for a file with none.
 */
#include <types.h>
#include <libgpu.h>

extern short   g_btl_tpage[];
extern u_short g_btl_clut[];

#define PAGE_W      64          /* pixels across one page              */
#define PAGE_BOTTOM 0x10        /* first page in the lower half        */
#define CLUT_ROW0   0x1E0       /* first VRAM row CLUTs are kept in    */
#define CLUT_X0     0xF0        /* leftmost CLUT column past the first */

u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, short y,
                     int nclut)
{
    TIM_IMAGE t;
    RECT r;             /* declared and never used; the frame is 8 bytes
                           larger than the code needs, so leave it */
    u_short *c;
    int i;

    OpenTIM(tim);
    ReadTIM(&t);

    if (t.paddr != 0) {
        t.prect->x = (page & 0xF) * PAGE_W;
        t.prect->y = (page & PAGE_BOTTOM) * 0x10 + y;
        LoadImage(t.prect, t.paddr);
        g_btl_tpage[page] = GetTPage(t.mode & 3, abr, t.prect->x, t.prect->y);
        if (t.prect->w > 0x80) {
            g_btl_tpage[page + 2] =
                GetTPage(t.mode & 3, abr, t.prect->x + 0x80, t.prect->y);
        }
        if (t.prect->w > 0x100) {
            g_btl_tpage[page + 4] =
                GetTPage(t.mode & 3, abr, t.prect->x + 0x100, t.prect->y);
        }
    }

    if (t.caddr != 0) {
        if (slot >> 5 != 0) {
            t.crect->x = (slot >> 5) * 16 + CLUT_X0;
        } else {
            t.crect->x = 0;
        }
        /* The do/while(0) gives this its own basic block, which is what puts
           the loop's counter and pointer in the registers the original uses. */
        do {
            t.crect->y = (slot & 0x1F) + CLUT_ROW0;
            LoadImage(t.crect, t.caddr);
            i = 0;
        } while (0);
        if (nclut > 0) {
            c = &g_btl_clut[slot];
            do {
                *c = GetClut(t.crect->x, t.crect->y);
                i++;
                t.crect->y++;
                c++;
            } while (i < nclut);
        }
    }
    return t.caddr;
}
