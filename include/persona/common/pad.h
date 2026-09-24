#ifndef PERSONA_COMMON_PAD_H
#define PERSONA_COMMON_PAD_H

/* Persona 1 (JP) - the button masks the game tests against the pad.
 *
 * Fifteen actions, each a mask that is anded with the live pad state.
 * PadLoadBindings fills them from a row of the binding table, chosen by the
 * saved pad configuration, so a player who changes the layout changes these
 * and nothing else. They live in the save-game work area, four bytes apart,
 * and only the low halfword of each slot is ever touched. */
#include <decomp/types.h>

#define PAD_ACTIONS 15

typedef struct {
    /* 0x0 */ u_short mask;
    /* 0x2 */ u_short pad2;   /* nothing reads or writes this half */
} PadBinding;

extern PadBinding g_pad_bindings[];

/* The two the accept checks use; the rest are not identified yet. */
#define BIND_ACCEPT_A 0
#define BIND_ACCEPT_B 1

/* The controller page: each layout gives twenty buttons an action apiece,
   one row of PAD_LAYOUT_ENTRIES action indices a layout, and an action index
   picks its name. Action 0 is no action. Each overlay carries its own copy. */
#define PAD_LAYOUT_ENTRIES 20

extern u_char        g_pad_layout_actions[];
extern u_char *g_pad_action_names[];

void PadDrawLayout(u_char layout);

#endif
