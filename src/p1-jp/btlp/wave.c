/* Persona 1 (JP) - the ripple across the battle mesh.  BTLP only.
 *   0x8008A430 BtlWaveMesh
 *
 * The floor the fight stands on is a 21 x 10 grid of vertices, each keeping
 * both the position it is drawn at and the rest position it hangs from. Every
 * frame this walks the grid and displaces the drawn pair from the rest pair by
 * two lookups into the wave tables, one per coordinate and at different
 * amplitudes, with the phase advancing along both axes as well as with time -
 * so the crests travel diagonally rather than as straight bands.
 *
 * The border stays put: the first and last column keep their first coordinate
 * and the top and bottom row their second, which is what keeps the sheet from
 * tearing away from whatever is drawn around it.
 */
#include <types.h>

#define BTL_MESH_COLS 21
#define BTL_MESH_ROWS 10

/* Entries in each wave table, and the step between neighbouring vertices. */
#define BTL_WAVE_MASK  0x1FF
#define BTL_WAVE_STEP  0x20

/* How far the phase moves in a frame. */
#define BTL_WAVE_SPEED 2

typedef struct {
    /* 0x00 */ int  x;        /* where the vertex is drawn      */
    /* 0x04 */ int  y;
    /* 0x08 */ int  rest_x;   /* what it is displaced from      */
    /* 0x0C */ int  rest_y;
    /* 0x10 */ int  pad10[2];
} BtlMeshVertex;              /* 0x18 bytes */

extern BtlMeshVertex g_btl_mesh[];
extern const int     g_btl_wave_sin[];
extern const int     g_btl_wave_cos[];
extern u_short       g_btl_wave_phase;
extern u_short       g_btl_mesh_phase;

void BtlWaveMesh(void)
{
    BtlMeshVertex *v;
    const int     *sine;
    const int     *cosine;
    const int     *phase;
    u_int          along;
    u_int          down;
    u_int          start;
    int            col;
    int            row;
    int            last;

    v = g_btl_mesh;
    row = 0;
    last = BTL_MESH_COLS - 1;
    sine = g_btl_wave_sin;
    start = g_btl_wave_phase;
    cosine = g_btl_wave_cos;
    down = start;
    do {
        col = 0;
        phase = &sine[down & BTL_WAVE_MASK];
        along = start;
        do {
            if (col != 0 && col != last) {
                v->x = *phase * 4 + v->rest_x;
            }
            col++;
            /* The row test also excludes 15, which a ten-row grid never
               reaches; it is in the original and reproducing it costs a
               comparison that would otherwise not be there. */
            if (row != 0 && row != BTL_MESH_ROWS - 1 && row != 15) {
                v->y = cosine[along & BTL_WAVE_MASK] * 8 + v->rest_y;
            }
            along += BTL_WAVE_STEP;
            v++;
        } while (col < BTL_MESH_COLS);
        row++;
        down += BTL_WAVE_STEP;
    } while (row < BTL_MESH_ROWS);

    g_btl_wave_phase += BTL_WAVE_SPEED;
    g_btl_mesh_phase += BTL_WAVE_SPEED;
}
