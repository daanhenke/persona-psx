/* Persona 1 (JP) - pointing a libgs coordinate system at a rotation.
 *
 *   DNG 0x8006F9D8   ADV 0x8008AFA8
 *
 * The translation already in the coordinate is kept and only the rotation is
 * rebuilt, so a caller that has moved an object can turn it without recomputing
 * where it stands. Clearing flg is what makes libgs recompose the matrix on the
 * next sort.
 */
#include <types.h>
#include <libgs.h>
#include <libgte.h>

void CoordSetRot(SVECTOR *rot, GsCOORDINATE2 *coord)
{
    MATRIX  m;
    SVECTOR r;

    m = GsIDMATRIX;
    m.t[0] = coord->coord.t[0];
    m.t[1] = coord->coord.t[1];
    m.t[2] = coord->coord.t[2];
    r = *rot;
    RotMatrix(&r, &m);
    coord->coord = m;
    coord->flg = 0;
}
