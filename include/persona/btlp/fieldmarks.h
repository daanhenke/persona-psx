#ifndef PERSONA_BTLP_FIELDMARKS_H
#define PERSONA_BTLP_FIELDMARKS_H

/* Persona 1 (JP) - the short-lived marks a blow or a cast puts on the field.
 *
 * fieldmarks.c spawns all five, and each hands back the record it made so the
 * caller can place it further or remember it.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* The two strike sets entry.c leaves packed behind the backdrop: a TIM and
   the artwork its models are cut from, one pair each. */
extern u_char *g_btl_strike_tim0;
extern u_char *g_btl_strike_tim1;
extern u_char *g_btl_strike_gfx0;
extern u_char *g_btl_strike_gfx1;

/* The kinds the marks are given. A number put up as HIT_NUMBER_STILL shares
   its kind with the miss word and stays where it is put; any other is given a
   shift and a timer and takes MARK_KIND_RISING. */
#define MARK_KIND_STILL  0xD
#define MARK_KIND_RISING 0x12
#define MARK_KIND_IMPACT 0x15
#define HIT_NUMBER_STILL 0x20

extern BtlObj *BtlSpawnStrike(int set, int model, const long *pos);
extern BtlObj *BtlSpawnCastCircle(int side, int col2, int row);

/* Puts `value` up over a fighter as a number of its own, up to four digits,
   centred on `pos`. */
extern BtlObj *BtlSpawnHitNumber(u_int value, const long *pos, int kind);

extern BtlObj *BtlSpawnMiss(const long *pos);
extern BtlObj *BtlSpawnImpact(int alt, const long *pos);

#endif
