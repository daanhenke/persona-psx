#ifndef PERSONA_COMMON_STATUS_H
#define PERSONA_COMMON_STATUS_H

/* Persona 1 (JP) - status ailment codes.
 *
 * Char.status holds one of these, and g_status_names is the table the status
 * screen draws from: seventeen 8-byte records of packed glyph bytes, each
 * 0xFF-terminated unless it fills all eight. The names are Latin, so
 * tools/glyphs.py reads them straight out and the code order is simply the
 * table order - index 0 is the "no ailment" entry, which reads GOOD.
 *
 *   tools/glyphs.py adv 800B89C8 136
 */

#define STATUS_GOOD    0    /* no ailment */
#define STATUS_HAPPY   1
#define STATUS_PANIC   2
#define STATUS_CHARM   3
#define STATUS_FREEZE  4
#define STATUS_SHOCK   5
#define STATUS_BIND    6
#define STATUS_SLEEP   7
#define STATUS_CLOSE   8
#define STATUS_BLIND   9
#define STATUS_UNLUCK  10
#define STATUS_TERROR  11
#define STATUS_GUILT   12
#define STATUS_POISON  13
#define STATUS_PALYZE  14   /* spelt this way in the table */
#define STATUS_STONE   15
#define STATUS_SICK    16

#define STATUS_COUNT   17
#define STATUS_NAME_W  8    /* bytes per record in g_status_names */

#endif
