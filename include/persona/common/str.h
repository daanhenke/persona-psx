#ifndef PERSONA_COMMON_STR_H
#define PERSONA_COMMON_STR_H

/* Persona 1 (JP) - CD streaming (STR) declarations shared by the sub-EXEs.
 *
 * ATLUS.EXE and OPEN.EXE compile in the same frame-wait routine; MOVIE.EXE and
 * END.EXE compile in the same MDEC decode routine. Within each pair the
 * declarations were byte-identical across the two sources, which is why they
 * are here rather than in four copies.
 *
 * StGetNext, StFreeRing and DecDCTvlc are Psy-Q streaming entry points that
 * include/psyq does not declare - there is no libpress.h, and libcd.h stops
 * short of the St* group - so they are declared here instead. StGetNext is
 * typed against StrFrameHeader rather than the SDK's second u_long **.
 */
#include <types.h>

/* ---- frame wait: ATLUS.EXE, OPEN.EXE ---------------------------------- */

/* Only the third word matters here: the decoded size of the frame. */
typedef struct {
    u_long unused[2];
    u_long size;
} StrFrameHeader;

extern int  StGetNext(u_long **frame, StrFrameHeader **header);

extern int g_str_overrun;

extern u_long *StrWaitFrame(void);

/* ---- MDEC decode: MOVIE.EXE, END.EXE ---------------------------------- */

/* Double-buffered MDEC target: `buf[index]` is the one currently being
   decoded into, and index flips on every frame. */
typedef struct {
    u_long *buf[2];
    int     index;
} StrDecodeTarget;

extern u_long *StrGetReadyFrame(void);
extern void    DecDCTvlc(u_long *bs, u_long *out);
extern void    StFreeRing(u_long *bs);

extern void StrDecodeNextFrame(StrDecodeTarget *target);

#endif
