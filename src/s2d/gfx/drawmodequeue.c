#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct {
    CVECTOR color;
    short pages[16];
    DR_MODE modes[2][16];
} DrawModeState;

extern DrawModeState D_800B56F4;

/* Queue each enabled texture page on its ordering-table entry. The two
 * primitive banks belong to the alternating drawing buffers. */
void func_80097CFC(GsOT *ot, int buffer)
{
    int i;
    int page;
    short *pages;
    DR_MODE *mode;

    i = 0;
    pages = D_800B56F4.pages;
    for (; i < 16; i++) {
        page = pages[i];
        if (page != -1) {
            mode = &D_800B56F4.modes[buffer][i];
            SetDrawMode(mode, 0, 0, page, 0);
            AddPrim(&ot->org[i], mode);
        }
    }
}
