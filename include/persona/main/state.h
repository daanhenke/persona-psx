/* Persona 1 (JP) - which scene the game is in, and getting to the next one.
 *
 * These live in the main executable and every overlay reaches them, which is
 * why they are here rather than beside whichever routine happens to set them.
 * An overlay hands control back by leaving a scene id in g_state_next and
 * returning; main reads it and loads that overlay next.
 */
#ifndef PERSONA_MAIN_STATE_H
#define PERSONA_MAIN_STATE_H

#include <decomp/types.h>

/* The scenes, as g_state_prev and g_state_next carry them. -1 is the one
   that is not a scene: nothing follows, which is how a lost battle leaves. */
#define GAME_STATE_DNG  0
#define GAME_STATE_BTL  1
#define GAME_STATE_S2D  2
#define GAME_STATE_ADV  3
#define GAME_STATE_NONE (-1)

/* Where the battle was entered from, and where it goes when it is over. */
extern int g_state_prev;
extern int g_state_next;

/* Which map the field will build when it is loaded again. */
extern short g_map_id;

/* The moon phase, kept for the whole game: the battle takes its own copy at
   the start and writes this one back on the way out. */
extern u_char g_moon;

/* Reading the next scene off the disc. The battle calls one of these on its
   way out so the load is already running while the last of it is drawn. */
extern void PreloadDng(void);
extern void PreloadS2d(void);
extern void PreloadAdv(void);

/* And the one for a battle that leads straight into another: it reads the
   field graphics for whatever g_map_id has just been set to. */
extern void PreloadBtlField(void);

#endif
