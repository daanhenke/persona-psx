/* Persona 1 (JP) - the Personas a battle offers.
 *
 * Three records, and the code that touches them agrees on three fields: a mask
 * whose lowest set bit ranks the offer, a short that is zero while the slot is
 * free, and the id of the Persona the offer would hand over. Whether the party
 * can take it is a separate question - BtlStockHolds asks whether they already
 * have that one, and BtlStockHasRoom whether there is a slot for it.
 */
#ifndef PERSONA_BTLP_OFFER_H
#define PERSONA_BTLP_OFFER_H

#include <types.h>

typedef struct {
    /* 0x00 */ u_long kinds;    /* the lowest set bit ranks the offer */
    /* 0x04 */ u_char pad04[3];
    /* 0x07 */ u_char pad07[9]; /* a byte per enemy slot; BtlOfferPickTalkers
                                   reads both of these and throws the values
                                   away, so what they hold is still open   */
    /* 0x10 */ u_char pad10[9];
    /* 0x19 */ u_char level;   /* measured against g_btl_talk_level    */
    /* 0x1A */ u_short talkers; /* of those, the ones with no ailment  */
    /* 0x1C */ u_short used;    /* enemies this offer involves; zero
                                   while the slot is free              */
    /* 0x1E */ u_char pad1E[0x12];
    /* 0x30 */ u_char persona;  /* what it would hand over            */
    /* 0x31 */ u_char pad31[0xB];
    /* 0x3C */ u_short flags;   /* bit 1 keeps a sum off a round hundred */
    /* 0x3E */ short  mood[4];  /* what `kinds` is a summary of       */
    /* 0x46 */ u_char pad46[2];
} BtlOffer;                     /* 0x48 bytes */

#define BTL_OFFERS   3
#define BTL_MOODS    4
#define BTL_NO_OFFER 0xFFFF

/* The two levels the gauges are read at. Bits 0..3 of `kinds` say which gauges
   have reached the first and bits 4..7 which have reached the second, so the
   lowest set bit says how well the best gauge is doing. */
#define BTL_MOOD_STRONG 0x5F
#define BTL_MOOD_WEAK   0x46

extern BtlOffer g_btl_offer[];

#endif
