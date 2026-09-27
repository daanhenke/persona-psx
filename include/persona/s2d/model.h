/* Persona 1 (JP) - S2D's registered TMDs (modelmap.c). */
#ifndef PERSONA_S2D_MODEL_H
#define PERSONA_S2D_MODEL_H

#include <decomp/types.h>

typedef struct {
    u_short n_vert;
    u_short n_normal;
    u_short n_prim;
    u_short pad;
} ModelCounts;

typedef struct {
    /* 0x00 */ u_long     *objs;
    /* 0x04 */ u_long      n_obj;
    /* 0x08 */ ModelCounts counts[16];
} Model;                                /* 0x88 */

extern Model g_models[];

#endif
