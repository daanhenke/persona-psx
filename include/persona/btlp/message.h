#ifndef PERSONA_BTLP_MESSAGE_H
#define PERSONA_BTLP_MESSAGE_H

#include <decomp/types.h>

/* Message lookup and substitution for the battle sequencer. */
extern const u_char* BtlMessage(int group, int index);
extern void          BtlSetInsert(int slot, const u_char* text);

#endif
