/* Persona 1 (JP) - readying a TMD for libgs.  DNG only.
 *   0x8006FAD8 FieldMapTmd
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Maps a TMD's modelling data to absolute addresses, skipping its id word,
   and returns its object list - past the id, the flags and the count. */
u_long *FieldMapTmd(u_long *tmd)
{
    tmd++;
    GsMapModelingData(tmd);
    return tmd + 2;
}
