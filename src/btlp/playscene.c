/* Persona 1 (JP) - one scripted line, portrait and all.  BTLP only.
 *   0x80099444 BtlPlayScene
 *
 * The HUD is brought up, a second is let pass, and whoever is speaking is put
 * beside the line: the first ten portraits belong to the party and are
 * reached by character, everything past them is a file of its own. A `who` of
 * zero is nobody, and then no portrait is opened or closed at all.
 *
 * The script itself is a sequence, and the routine does not return until it
 * has run out.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>

/* Frames the HUD is left standing before the line starts. */
#define SCENE_SETTLE 60

/* Portraits below this belong to the party and are looked up by character. */
#define SCENE_FACE_PARTY 10

/* Where the portrait is put, and how big it grows. */
#define SCENE_FACE_X     0x3C
#define SCENE_FACE_Y     0x70
#define SCENE_FACE_SCALE 0x1000

extern void BtlFaceLoad(int who, int always);
extern void BtlFaceLoadFile(int file);
extern void BtlFaceOpen(short x, short y, short scale);


void BtlPlayScene(int who, u_char *script)
{
    int i;

    BtlHudLoad();
    BtlHudShow();
    i = 0;
    do {
        i++;
        BtlDrawFrame();
    } while (i < SCENE_SETTLE);

    if (who != 0) {
        if (who < SCENE_FACE_PARTY) {
            BtlFaceLoad(who, 1);
        } else {
            BtlFaceLoadFile(who);
        }
        BtlFaceOpen(SCENE_FACE_X, SCENE_FACE_Y, SCENE_FACE_SCALE);
    }

    BtlSeqPlay(script);
    BtlSeqRun();

    if (who != 0) {
        BtlFaceClose();
    }
    BtlHudHide();
}
