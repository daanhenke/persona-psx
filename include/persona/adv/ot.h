#ifndef PERSONA_ADV_OT_H
#define PERSONA_ADV_OT_H

/* Persona 1 (JP) - the field's ordering tables.  ADV only.
 *
 * Double-buffered: g_ot_index is the one being built this frame, and every
 * GsSort* call in the overlay sorts into g_ot[g_ot_index]. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern GsOT g_ot[];
extern int  g_ot_index;

#endif
