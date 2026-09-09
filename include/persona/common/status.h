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
#include <decomp/types.h>

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

/* Five slots, one byte each, 0xFF for empty. The byte indexes the 0x60-byte
   character records at 0x801F1BCC rather than being the character itself. It
   is the same address in every overlay, so one source covers all three. */
#define g_party ((u_char *)0x801F256C)
#define PARTY_EMPTY 0xFF

/* Selector for CharBelowMax: 0 is current HP against its maximum. */
#define BELOW_HP 0

/* Highest occupied party slot, cached from PartyLastSlot() when the overlay
   starts up. Each overlay keeps its own copy in its own work area. */
extern u_char g_party_last;

/* These four are one unit in the image, split across two objects by a routine
   between them that is not worked out yet. The prototypes live here because
   the prototype is what decides how the arguments are converted. */
extern u_char CharHasStatus(u_char slot, u_char status);
extern u_char CharBelowMax(u_char slot, u_char kind);
extern u_char ItemUsableOn(u_char slot, short item);
extern u_char PartyAnyStatus(u_char status);
extern u_char PartyAnyBelowMax(u_char kind);

#endif
