#ifndef PERSONA_BTLP_FACE_H
#define PERSONA_BTLP_FACE_H

/* Persona 1 (JP) - the portrait of whoever is talking.
 *
 * BtlFaceLoad reads a character's picture in - a second call for the same
 * character is free unless `always` is set - and BtlFaceOpen puts it up at a
 * position and a 4.12 scale. BtlFaceClose, which takes it down, sits with the
 * rest of the battle's closers in battle.h.
 */
#include <decomp/types.h>

extern void BtlFaceLoad(int who, int always);
extern void BtlFaceOpen(short x, short y, short scale);

#endif
