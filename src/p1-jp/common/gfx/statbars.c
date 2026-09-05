/* Persona 1 (JP) - the five stat bars on the status screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                           DNG         ADV         S2D
 *   DrawCharStatBars        0x800914F0  0x8008D420  0x80081A04
 *   DrawPersonaStatBars     0x80091878  0x8008D7A8  0x80081D8C
 *   DrawPersonaDataStatBars 0x80091B14  0x8008DA44  0x80082028
 *
 * A character's bar is drawn in two colours - stat_base as the solid part and
 * stat above it as the highlight - while a Persona's is one colour, which is
 * why the two per-bar routines are separate. Both run at quarter-cell
 * resolution out of the same glyph strip.
 */
#include <types.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

extern void DrawCharStatBar(Char *rec, u_char stat);
extern void DrawPersonaStatBar(Persona *p, u_char stat);
extern void DrawPersonaDataStatBar(short id, u_char stat);

void DrawCharStatBars(Char *rec)
{
    DrawCharStatBar(rec, 0);
    DrawCharStatBar(rec, 1);
    DrawCharStatBar(rec, 2);
    DrawCharStatBar(rec, 3);
    DrawCharStatBar(rec, 4);
}

void DrawPersonaStatBars(Persona *p)
{
    DrawPersonaStatBar(p, 0);
    DrawPersonaStatBar(p, 1);
    DrawPersonaStatBar(p, 2);
    DrawPersonaStatBar(p, 3);
    DrawPersonaStatBar(p, 4);
}

/* The stock list shows a Persona out of the reference table before it belongs
   to anybody, so its bars come from there rather than from a live record. */
void DrawPersonaDataStatBars(short id)
{
    DrawPersonaDataStatBar(id, 0);
    DrawPersonaDataStatBar(id, 1);
    DrawPersonaDataStatBar(id, 2);
    DrawPersonaDataStatBar(id, 3);
    DrawPersonaDataStatBar(id, 4);
}
