/* Persona 1 (JP) - the room picture's cell index.  ADV only.
 *   0x800831D4 ImageIndexInit
 *
 * The room picture is drawn as a character map (imagecells.c lays out the
 * cells); this writes the index that says which cell goes where. A narrow
 * picture - 24 cells, kinds 0 and 1 - sits in a 26-wide map with a blank
 * column either side and blank rows under its sixteen.
 */
#include <decomp/types.h>

#define IMG_INDEX   ((u_short *)0x800EB788)
#define IMG_INDEX_W 26
#define IMG_INDEX_H 17
#define IMG_BLANK   0x17F

#define PANEL_INDEX ((u_short (*)[10])0x800EF580)

void ImageIndexInit(short kind)
{
    int i;
    int j;
    int row;
    int col;

    /* One bound at a time: `kind < 2 && kind >= 0` folds to one unsigned
       compare, which the original does not have. */
    if (kind < 2) {
        if (kind < 0) {
            return;
        }
        for (i = 0; i < IMG_INDEX_W * IMG_INDEX_H; i++) {
            row = i / IMG_INDEX_W;
            col = i - row * IMG_INDEX_W;
            if (col == 0 || col == IMG_INDEX_W - 1 || row >= 16) {
                IMG_INDEX[i] = IMG_BLANK;
            } else {
                IMG_INDEX[i] = row * 24 + col - 1;
            }
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 4; j++) {
                PANEL_INDEX[i][j] = i * 4 + j;
            }
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                PANEL_INDEX[i][4 + j] = i * 8 + j + 0x20;
            }
        }
    }
}
