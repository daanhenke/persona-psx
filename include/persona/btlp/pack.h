#ifndef PERSONA_BTLP_PACK_H
#define PERSONA_BTLP_PACK_H

/* Persona 1 (JP) - the two packed files the battle reads from disc.
 *
 * Both are laid out the same way: a table of start offsets in sectors, where
 * an entry's end is simply where the next one begins, so a length is the
 * difference between neighbours. g_btl_pack_offsets indexes the battle's own
 * pack and g_btl_pack_bank_sectors the demon voice banks, and each has a base
 * sector to add on. Only one entry of each is resident at a time, which is why
 * callers reload whenever the offer changes.
 *
 * A read lands in one fixed buffer. BtlOpenPackBank then moves two of the
 * three pieces it holds into the resident work area and hands the bank to the
 * SPU; the VAB body stays where it landed, since nothing reads it on the way
 * through.
 */
#include <decomp/types.h>

/* The battle's pack. */
extern u_short g_btl_pack_offsets[];
extern int     g_btl_pack_base;

/* The demon voice banks. */
extern u_short g_btl_pack_bank_sectors[];
extern int     g_btl_pack_bank_base;

/* Where a bank arrives, and the three pieces BtlOpenPackBank finds in it. */
#define BTL_PACK_BUFFER 0x80152400

extern u_char *g_btl_pack_vh;
extern u_char *g_btl_pack_vb;
extern u_long *g_btl_pack_seq;

/* Set once a bank is resident, and cleared when one is asked for. */
extern u_char g_btl_pack_ready;

extern void BtlReadSectors(u_long *dest, int sector, int sectors);
extern void BtlReadSectorsAsync(u_long *dest, int sector, int sectors);

extern void BtlLoadPackEntry(int entry);
extern void BtlLoadPackBank(int entry);
extern void BtlReadPackBank(int wait, int entry);
extern void BtlOpenPackBank(void);

#endif
