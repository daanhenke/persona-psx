#ifndef PERSONA_MOVIE_MOVIE_H
#define PERSONA_MOVIE_MOVIE_H

/* Persona 1 (JP) - MOVIE.EXE, the FMV player.
 *
 * The resident executable leaves the movie to play in g_movie_id and runs
 * this one; it plays that movie from \STR<bank>\MV<nn>.STR, sets up where the
 * game resumes, and returns to the resident.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/common/str.h>

/* Where the game picks up after a movie from 0xC on. */
typedef struct {
    int     state;  /* the scene main runs next */
    u_short map;
    u_char  x;
    u_char  y;
    u_char  map_unk4;
    u_char  room;
} MovieAfter;

/* How many frames each movie runs, and where each one from 0xC on leaves
   the game. */
extern u_short    g_movie_frames[];
extern MovieAfter g_movie_after[];

/* gp small data; each unit keeps its own tentative definition. */
extern u_char g_movie_no;
extern u_char g_movie_first;
extern u_char g_movie_again;
extern int    g_movie_end;
extern int    g_movie_frame;

/* The resident's work area. */
extern u_char g_movie_id;
extern int    g_movie_next_state;
extern u_char g_options[];

extern void StrPutHex(short value, char *p, short n);

#endif
