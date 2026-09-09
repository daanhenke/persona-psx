#ifndef PERSONA_BTLP_NUMBER_H
#define PERSONA_BTLP_NUMBER_H

/* Persona 1 (JP) - numbers on the battle screen.
 *
 * Laying a number out and drawing it are two steps. The formatters write digit
 * *values* into a scratch field - g_btl_number_buf, with 0xFF for a blank and
 * 0xFE for a minus - and the drawing pass turns that field into font cells by
 * adding a base. Keeping them apart is what lets the same field be drawn in
 * either size of digit.
 *
 * The hex pair is the exception: BtlHexDigits writes ASCII rather than values,
 * because it has letters to spell, and BtlFormatHex converts from there.
 */
#include <decomp/types.h>

/* The scratch field every formatter writes into. */
extern signed char g_btl_number_buf[];

/* What a place in the field can hold besides a digit. */
#define NUMBER_BLANK (-1)
#define NUMBER_MINUS (-2)

/* Font cells. The long vowel mark stands in for a minus sign, the font having
   nothing better, and cell 0 is blank. */
#define GLYPH_MINUS 0xCC
#define GLYPH_SPACE 0
#define GLYPH_END   0xFF

/* Added to a digit value to reach the font. d - 0x40 lands on 0xC0 + d, which
   is where 0 to 9 live, and d - 0x1B on 0xE5 + d, the other size the overlay
   carries. */
#define DIGIT_BASE     (-0x40)
#define DIGIT_BASE_ALT (-0x1B)

/* Added to an ASCII hex character instead, since BtlHexDigits spells rather
   than counts: '0' (0x30) lands on the same 0xC0 and 'A' (0x41) on 0xA6. */
#define HEX_DIGIT_BASE  (-0x70)
#define HEX_LETTER_BASE 0x65
#define HEX_LETTERS     6

/* strcpy, strcat and strlen over a GLYPH_END terminator. */
extern void BtlTileCopy(u_char *dst, const u_char *src);
extern void BtlTileAppend(u_char *dst, const u_char *src);
extern int  BtlTileLength(const u_char *s);

extern u_char *BtlFormatDecimal(int value, u_char *dst, int pad);
extern void    BtlFormatRight(signed char *dst, int value, int width);
extern void    BtlFormatHex(u_char *dst, int value, int digits);
extern void    BtlHexDigits(signed char *dst, int value, int digits);
extern int     BtlDrawNumber(u_char *dst, int value, int width);
extern int     BtlDrawNumberAlt(u_char *dst, int value, int width);

#endif
