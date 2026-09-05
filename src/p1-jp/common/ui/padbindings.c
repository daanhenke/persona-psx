/* Persona 1 (JP) - installing a button layout.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x80090220   ADV 0x8008BFF8   S2D 0x80080730
 *
 * The table holds one row of fifteen masks per layout; the saved pad
 * configuration picks the row. Everywhere this is called PadSetPageButtons is
 * called with the same argument, so the two together are what applying a
 * layout means.
 */
#include <types.h>
#include <persona/common/pad.h>

/* Each overlay carries its own copy of the table in its data. */
extern u_short g_pad_binding_table[][PAD_ACTIONS];

void PadLoadBindings(u_char config)
{
    u_short m0, m1, m2, m3, m4, m5, m6, m7;
    u_short m8, m9, m10, m11, m12, m13, m14;
    /* The original reserves 0x70 bytes of frame it never touches. */
    char    unused[0x70];

    m0 = g_pad_binding_table[config][0];
    m1 = g_pad_binding_table[config][1];
    m2 = g_pad_binding_table[config][2];
    m3 = g_pad_binding_table[config][3];
    m4 = g_pad_binding_table[config][4];
    m5 = g_pad_binding_table[config][5];
    m6 = g_pad_binding_table[config][6];
    m7 = g_pad_binding_table[config][7];
    m8 = g_pad_binding_table[config][8];
    m9 = g_pad_binding_table[config][9];
    m10 = g_pad_binding_table[config][10];
    m11 = g_pad_binding_table[config][11];
    m12 = g_pad_binding_table[config][12];
    m13 = g_pad_binding_table[config][13];
    m14 = g_pad_binding_table[config][14];

    g_pad_bindings[0].mask = m0;
    g_pad_bindings[1].mask = m1;
    g_pad_bindings[2].mask = m2;
    g_pad_bindings[3].mask = m3;
    g_pad_bindings[4].mask = m4;
    g_pad_bindings[5].mask = m5;
    g_pad_bindings[6].mask = m6;
    g_pad_bindings[7].mask = m7;
    g_pad_bindings[8].mask = m8;
    g_pad_bindings[9].mask = m9;
    g_pad_bindings[10].mask = m10;
    g_pad_bindings[11].mask = m11;
    g_pad_bindings[12].mask = m12;
    g_pad_bindings[13].mask = m13;
    g_pad_bindings[14].mask = m14;
}
