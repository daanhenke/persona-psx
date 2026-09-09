/* Persona 1 (JP) - the battle overlay's graphics decompressor.  BTLP only.
 *   0x800810A4 BtlUnpack
 *
 * One control byte per token, sometimes with a second:
 *
 *   0x7F 0xFF          end of stream
 *   0nnnnndd dddddddd  a back reference: nnnnn + 2 bytes from a distance of
 *                      -1024..-1, copied a byte at a time so an overlapping
 *                      run repeats rather than being a block move
 *   100nnnnn           nnnnn + 1 literal bytes follow
 *   101nnnnn           nnnnn + 1 bytes follow, each written after a zero -
 *                      which is what turns four-bit artwork into eight
 *   110nnnnn bb        nnnnn + 2 copies of bb
 *   111nnnnn           nnnnn + 1 zeros
 *   0xFF nn            nn + 0x20 zeros
 *
 * The first byte is read twice over - once to test for the end marker and
 * again for the token itself - and every run is written through the output as
 * an index rather than a walking pointer, which is what the original does.
 *
 * BtlUploadPackedTim, which unpacks a TIM and hands it straight to the
 * uploader, is in packedtim.c.
 */
#include <decomp/types.h>

/* The end marker, and the code that is a long zero run rather than a short
   one. */
#define PACK_END_LO  0x7F
#define PACK_END_HI  0xFF
#define PACK_LONG    0xFF

/* The top bits that say which code this is, and the five that carry a count. */
#define PACK_KIND    0xE0
#define PACK_COUNT   0x1F
#define PACK_LITERAL 0x80
#define PACK_SPREAD  0xA0
#define PACK_REPEAT  0xC0
#define PACK_ZEROS   0xE0

/* A back-reference is the low bit of the first byte clear: five bits of length
   above the two top bits of the distance. */
#define PACK_BACK_LEN  0x7C
#define PACK_BACK_DIST 0x3

/* Distances are ten bits and always backwards, so the sign is put on by hand
   rather than carried. */
#define PACK_BACK_SIGN 0xFC00

/* What each count is short by. */
#define PACK_MIN_BACK   2
#define PACK_MIN_REPEAT 2
#define PACK_MIN_RUN    1

/* A long zero run is at least this many. */
#define PACK_LONG_MIN 0x20

void BtlUnpack(u_char *dst, const u_char *src)
{
    int     n;
    int     i;
    short   back;

    for (;;) {
        if (src[0] == PACK_END_LO && src[1] == PACK_END_HI) {
            return;
        }
        if ((src[0] & PACK_LITERAL) == 0) {
            n = ((src[0] & PACK_BACK_LEN) >> 2) + PACK_MIN_BACK;
            back = (short)((((src[0] & PACK_BACK_DIST) << 8) | src[1])
                           | PACK_BACK_SIGN);
            for (i = 0; i < n; i++) {
                dst[i] = dst[i + back];
            }
            dst += n;
            src += 2;
        } else if (src[0] == PACK_LONG) {
            n = src[1] + PACK_LONG_MIN;
            for (i = 0; i < n; i++) {
                dst[i] = 0;
            }
            dst += n;
            src += 2;
        } else {
            switch (src[0] & PACK_KIND) {
            case PACK_LITERAL:
                n = (src[0] & PACK_COUNT) + PACK_MIN_RUN;
                for (i = 0; i < n; i++) {
                    dst[i] = src[i + 1];
                }
                dst += n;
                src += n + 1;
                break;
            case PACK_SPREAD:
                n = (src[0] & PACK_COUNT) + PACK_MIN_RUN;
                for (i = 0; i < n; i++) {
                    dst[i * 2] = 0;
                    dst[i * 2 + 1] = src[i + 1];
                }
                dst += n * 2;
                src += n + 1;
                break;
            case PACK_REPEAT:
                n = (src[0] & PACK_COUNT) + PACK_MIN_REPEAT;
                for (i = 0; i < n; i++) {
                    dst[i] = src[1];
                }
                dst += n;
                src += 2;
                break;
            case PACK_ZEROS:
                n = (src[0] & PACK_COUNT) + PACK_MIN_RUN;
                for (i = 0; i < n; i++) {
                    dst[i] = 0;
                }
                dst += n;
                src += 1;
                break;
            }
        }
    }
}
