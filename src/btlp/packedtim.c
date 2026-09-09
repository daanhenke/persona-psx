/* Persona 1 (JP) - unpacking a TIM straight into VRAM.  BTLP only.
 *   0x8008DBDC BtlUploadPackedTim
 *
 * One control byte per token, sometimes with a second:
 *
 *   0x7F 0xFF          end of stream
 *   0nnnnndd dddddddd  a back reference: nnnnn + 2 bytes from a distance of
 *                      -1024..-1, copied a byte at a time so an overlapping
 *                      run repeats rather than being a block move
 *   100nnnnn           nnnnn + 1 literal bytes follow
 *   101nnnnn           nnnnn + 1 bytes follow, each written after a zero
 *   110nnnnn bb        nnnnn + 2 copies of bb
 *   111nnnnn           nnnnn + 1 zeros
 *   0xFF nn            nn + 0x20 zeros
 *
 * The 101 form is what makes the whole thing worth having: a 4bpp image
 * stored as bytes has a zero high nibble everywhere, so half the data is a
 * known constant and only the other half is written out.
 *
 * 0xFF is tested before the 111 group it would otherwise belong to, which is
 * how a run longer than 32 is spelt.
 */
#include <decomp/types.h>

#define UNPACK_END_HI  0x7F
#define UNPACK_END_LO  0xFF

#define UNPACK_LONG_ZEROS 0xFF
#define UNPACK_ZERO_BASE  0x20

#define UNPACK_KIND     0xE0
#define UNPACK_LITERAL  0x80
#define UNPACK_NIBBLES  0xA0
#define UNPACK_RUN      0xC0
#define UNPACK_ZEROS    0xE0

#define UNPACK_COUNT    0x1F

/* A back reference's length is five bits of the control byte and its distance
   the low two bits joined to the whole of the next, sign extended. */
#define UNPACK_REF_LEN  0x7C
#define UNPACK_REF_HI   0x03
#define UNPACK_REF_SIGN 0xFC00

extern u_long *g_btl_tim_buf;

/* BtlUploadTim narrows `y` itself; declaring it short here would make this
   sign-extend the argument before passing it on, which the original does not. */
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int nclut);

/* The decompressor is a unit of its own; see unpack.c. */
extern void BtlUnpack(u_char *dst, const u_char *src);


/* `y` is an int here even though BtlUploadTim narrows it: this only forwards
   the argument, and declaring it short adds the sign extension. */
void BtlUploadPackedTim(const u_char *src, int page, int slot, int abr,
                        int y, int nclut)
{
    BtlUnpack((u_char *)g_btl_tim_buf, src);
    BtlUploadTim(g_btl_tim_buf, page, slot, abr, y, nclut);
    DrawSync(0);
}
