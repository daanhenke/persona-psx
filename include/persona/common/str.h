#ifndef PERSONA_COMMON_STR_H
#define PERSONA_COMMON_STR_H

/* Persona 1 (JP) - CD streaming (STR) declarations shared by the sub-EXEs.
 *
 * ATLUS.EXE and OPEN.EXE compile in the same frame-wait routine; MOVIE.EXE and
 * END.EXE compile in the same MDEC player, which is Sony's streaming sample
 * (DECENV and the strNext/strSync/strCallback routines) with the file name
 * patched per movie. Within each pair the declarations were byte-identical
 * across the two sources, which is why they are here rather than in four
 * copies.
 *
 * StGetNext takes the sector header as a u_long **; the callers pass their
 * StrFrameHeader * through a cast.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpress.h>

/* The sector header of a streamed frame; only the third word matters here,
   the number of the frame this sector belongs to. */
typedef struct {
    u_long unused[2];
    u_long frame_count;
} StrFrameHeader;

/* ---- frame wait: ATLUS.EXE, OPEN.EXE ---------------------------------- */

extern int g_str_overrun;

extern u_long *StrWaitFrame(void);

/* ---- MDEC decode: MOVIE.EXE, END.EXE ---------------------------------- */

/* The decode state: two run-level buffers the frames are unpacked into in
   turn, the slice buffer the MDEC writes, the two display areas the slices
   are loaded into in turn, and the slice itself, which walks across the
   current area one DecDCTout at a time. `done` is raised by the callback
   once the last slice of a frame is in. */
typedef struct {
    u_long *vlc_buf[2];
    int     vlc_id;
    u_long *img_buf;
    RECT    rect[2];
    int     rect_id;
    RECT    slice;
    int     done;
} StrDecodeTarget;

/* Where the movie starts, the MDEC buffers and the decode state. The two
   run-level buffers sit right after the location in the bss. */
extern CdlLOC          g_str_loc;
extern u_long          g_vlc_buf0[];
extern u_long          g_vlc_buf1[];
extern u_long          g_img_buf[];
extern StrDecodeTarget g_dec;
extern u_long          g_ring_buf[];

extern u_long *StrGetReadyFrame(void);

extern void StrDecodeNextFrame(StrDecodeTarget *dec);
extern void StrSync(StrDecodeTarget *dec);
extern void StrSetDecEnv(StrDecodeTarget *dec, int x0, int y0, int x1, int y1);
extern void StrKickCD(CdlLOC *loc);

#endif
