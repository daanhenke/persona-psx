/* Persona 1 (JP) - ADV's decompressor.  ADV only.
 *   0x800B0ACC AdvUnpack
 *
 * The scene and event files come off the disc packed. The stream is a run of
 * opcodes, each one byte, some with an operand byte after it; `size` is the
 * unpacked length and the loop stops once that much has been written, or at
 * the end marker 7F FF.
 *
 *   0xxxxxyy yyyyyyyy   copy (x + 2) bytes from earlier output: y is a 10-bit
 *                       distance back, 1..0x400, as the low bits of a
 *                       negative offset (0xFC00 | y)
 *   100nnnnn            copy (n + 1) literal bytes from the stream
 *   101nnnnn            (n + 1) pairs: a zero, then one literal byte - a
 *                       run of 16-bit values whose low byte is 0
 *   110nnnnn b          (n + 2) copies of byte b
 *   111nnnnn            (n + 1) zero bytes; 0xFF instead takes the count
 *                       from the next byte, plus 0x20
 *
 * Counting is by output byte, so the two-byte pairs count 2 each.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

#define OP_LITERAL 0x80
#define OP_PAIRS   0xA0
#define OP_FILL    0xC0
#define OP_ZEROES  0xE0
#define END_HI     0x7F
#define END_LO     0xFF
#define ZEROES_LONG 0xFF

/* 80.18%: the same opcodes and loops; the stream pointer, the opcode byte
   and the count sit in other registers than the original's. */
#ifdef NON_MATCHING
void AdvUnpack(u_char *src, u_char *dst, u_int size)
{
    u_int   done;
    u_char  op;
    u_char *from;
    short   k;
    short   n;
    u_char  b;

    done = 0;
next:
    if (done < size) {
        op = *src++;
        if (op == END_HI && *src == END_LO) {
            return;
        }
        k = 0;
        if (op < 0x80) {
            n = ((op >> 2) & 0x1F) + 2;
            from = dst + (short)(((op << 8) + *src++) | 0xFC00);
            for (; k < n; k++) {
                done++;
                *dst++ = from[k];
            }
            goto next;
        }
        switch (op & 0xE0) {
        case OP_LITERAL:
            n = (op & 0x1F) + 1;
            for (; k < n; k++) {
                done++;
                *dst++ = *src++;
            }
            break;
        case OP_PAIRS:
            n = (op & 0x1F) + 1;
            for (; k < n; k++) {
                *dst++ = 0;
                done += 2;
                *dst++ = *src++;
            }
            break;
        case OP_FILL:
            n = (op & 0x1F) + 2;
            b = *src++;
            for (; k < n; k++) {
                *dst++ = b;
                done++;
            }
            break;
        case OP_ZEROES:
            if (op == ZEROES_LONG) {
                n = *src++ + 0x20;
            } else {
                n = (op & 0x1F) + 1;
            }
            for (; k < n; k++) {
                *dst++ = 0;
                done++;
            }
            break;
        }
        goto next;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/unpack", AdvUnpack);
#endif
