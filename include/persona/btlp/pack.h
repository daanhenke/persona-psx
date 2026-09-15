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

/* The Persona pack: one start sector per graphics id, an entry running to
   where the next one begins, so a length is always a subtraction.
   summonpersona.c emits it. The entries past the last Persona are the
   summon's own artwork, which is why BtlMemberMotion05 indexes the table
   beyond where the graphics ids stop. */
extern u_short g_btl_persona_sectors[];

/* Where the two graphics packs that table indexes start on the disc, found
   by name as the overlay opens: \B\G.BIN holds each Persona's own artwork
   and \B\M.BIN the moves', the cast's and the summon's. entry.c. */
extern int g_btl_persona_gfx_base;
extern int g_btl_move_gfx_base;

/* The moves' artwork starts this far into the sector table, so a move's
   entry is its id past it. */
#define BTL_MOVE_FILE_FIRST 0x72

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

/* A party member's bank: an entry of the battle's pack picked by the member's
   key, read to the same buffer and opened into the same slot. */
extern u_short g_btl_member_bank_sectors[];

/* Where BtlOpenPackBank and BtlOpenMemberBank move the header and the sequence
   to, how much of each, and the slot both open the bank into. */
#define BTL_PACK_VH_HOME  0x8016FCF0
#define BTL_PACK_SEQ_HOME 0x80171848
#define BTL_PACK_VH_SIZE  0x1B58
#define BTL_PACK_SEQ_SIZE 0xBB8
#define BTL_PACK_SLOT     3

/* Two three-cell fields on the debug HUD: the slot whose bank was last asked
   for, and which kind of bank was opened last. */
extern u_char g_btl_bank_slot_cells[];
extern u_char g_btl_bank_kind_cells[];
#define BTL_BANK_MEMBER 0
#define BTL_BANK_DEMON  1
#define BTL_BANK_CELLS  3

extern void BtlReadSectors(u_long *dest, int sector, int sectors);
extern void BtlReadSectorsAsync(u_long *dest, int sector, int sectors);

extern void BtlLoadPackEntry(int entry);
extern void BtlLoadPackBank(int entry);
extern void BtlSeekPackEntry(int entry);
extern void BtlReadPackBank(int wait, int entry);
extern void BtlOpenPackBank(void);

extern void BtlLoadMemberBank(int key);
extern void BtlReadMemberBank(int wait, int key);
extern void BtlOpenMemberBank(void);

#endif
