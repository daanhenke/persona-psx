#ifndef PERSONA_BTLP_LOAD_H
#define PERSONA_BTLP_LOAD_H

#include <decomp/types.h>

/* What the loader leaves behind at 0x80140000: one address per run it read,
   each reached by a name of its own. A file read straight into the buffer
   leaves its own table there instead - a member's model file puts its TIM
   first and the start and end of its model data after it. entry.c reaches
   the first word by its literal address and names the rest itself. */
extern u_char *g_load_stage;
extern u_char *g_load_stage_1;
extern u_char *g_load_stage_2;

/* The buffer itself, which the battle's artwork reads all land in. */
#define BTL_LOAD_STAGE ((u_long *)0x80140000)

#endif
