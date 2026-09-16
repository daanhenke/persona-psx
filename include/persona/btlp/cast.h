#ifndef PERSONA_BTLP_CAST_H
#define PERSONA_BTLP_CAST_H

/* Persona 1 (JP) - calling a Persona up to make a move, which both sides do:
 * a member's spell in BtlMemberMotion06 (memberact.c) and an enemy's Persona
 * move in BtlEnemyPersonaMove (enemymove.c).
 *
 * The cast reads the summon circle's artwork, puts the circle up under the
 * caster and dims everyone else, reads the Persona's own artwork in and stands
 * it on the field at a sixteenth of its size, then plays the move and takes
 * all of the Persona's records back off.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* The summon circle's artwork in the Persona pack's sector table, and the page
   and slot its tim goes to. */
#define CAST_FILE     0x171
#define CAST_TIM_PAGE 0x1C
#define CAST_TIM_SLOT 0x11

/* What the rest of the field is taken down to while the Persona is out, what
   the arena drops to and comes back to, and the step it fades by. */
#define CAST_DIM          0x20
#define SUMMON_SCENE_DIM  0x40
#define SUMMON_SCENE_LIT  0x80
#define CAST_ARENA_FADE   1

/* The Persona: how it is put up, and how many records it is made of. */
#define CAST_PERSONA_ATTR   0x20000000
#define CAST_SCALE_XY       0x100
#define CAST_SCALE_Z        0x1000
#define CAST_PERSONA_PIECES 6

/* The Persona standing on the field, and whether it has finished coming out;
   the Persona motions wait on the flag. memberact.c keeps the flag. */
extern BtlObj *g_btl_persona_obj;
extern u_char  g_btl_persona_ready;

/* The two ways the Persona plays a move out once it is standing, which
   BtlPersonaMotion02 picks between by the move: the plain one, and the spell
   that walks the caster's target mask, hit by hit. personaact.c. */
extern void BtlPersonaPlayMove(BtlObj *o);
extern void BtlPersonaSpellMove(BtlObj *o);

/* The one arm of that spell's walk long enough to stand on its own.
   personaspell15.c. */
extern void BtlPersonaSpell15(BtlObj *o);

/* Stands a Persona's records up at a cell of the grid. personaspawn.c. */
extern BtlObj *BtlSpawnPersona(int gfx, int col, int row, int motion);

#endif
