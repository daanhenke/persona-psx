/* Persona 1 (JP) - putting a summoned persona in the arena.  BTLP only.
 *   0x80084614 BtlSpawnPersona
 *
 * BtlSummonPersona has already read the persona's file to the staging buffer
 * at 0x80140000. Its first two words are the TIM and the model image: the
 * image is copied to the effect image's own buffer and bound there, and the
 * TIM goes to page 0x1A with its CLUT in slot 0x10. The page beside it is
 * given the same texture with abr 3, so the artwork can be drawn additively
 * as well as normally.
 *
 * The persona itself is six records chained through `attached`, each four
 * frames behind the one before - the trail it appears out of. Only the first
 * is returned; the caller scales that one and puts it on its motion, and every
 * setter walks the chain from there.
 *
 * Graphics ids above 100 stand on the far row rather than the near one.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/object.h>

/* The file was read to the shared staging buffer. Its first word is the TIM,
   its second the model image, built to no fixed address. */
extern u_long *g_load_stage[];

/* Where the model image is copied so BtlBindGfx can fix it up in place. */
#define PERSONA_IMAGE ((u_char *)0x801D8400)
#define PERSONA_IMAGE_BYTES 0x1000

/* BtlBindGfx's slot for an effect image, which is what a persona is bound as. */
#define GFX_EFFECT 2

/* The page the artwork is uploaded to, and the CLUT slot beside it. */
#define PERSONA_PAGE 0x1A
#define PERSONA_SLOT 0x10
#define PERSONA_ABR  1

/* Page 0x1B is the same texture drawn with abr 3 - bits 5 and 6 of a tpage
   word are the semi-transparency mode. */
#define TPAGE_ABR_ADD 0x60

/* Which list the assembly is built in, and what each record is. */
#define PERSONA_GROUP 3
#define PERSONA_KIND  5

/* Six layers, four frames apart. */
#define PERSONA_LAYERS 6
#define PERSONA_STAGGER 4

/* Every layer but the first is one of the trailing copies. */
#define BTL_OBJ_TRAIL 0x400
#define PERSONA_TRAIL_CD 0x1B

/* How far a layer's colour walks in one frame. */
#define PERSONA_FADE 2

/* Where the assembly stands: the near row for a persona, the far one for a
   graphics id past the personas proper. */
#define PERSONA_FAR_ID  100
#define PERSONA_COL_W   15
#define PERSONA_COL_X   0x3C
#define PERSONA_ROW_H   20
#define PERSONA_ROW_NEAR 0x3C
#define PERSONA_ROW_FAR  0x8C
#define PERSONA_Z        0xFFE00000

/* Which of the model's scripts a negative motion asks for. */
#define PERSONA_SCRIPT_DEFAULT 7

/* One record per persona graphics id. */
typedef struct {
    /* 0x0 */ u_long attr;       /* OR'd into every layer of the assembly */
    /* 0x4 */ u_char script[12]; /* a script index per motion, into the model's
                                    own table; only the first nine are ever
                                    anything but zero */
} BtlPersonaGfx;                 /* 0x10 bytes */

extern BtlPersonaGfx g_btl_persona_gfx[];
extern BtlObjDef     g_btl_persona_def;
extern u_char       *g_btl_persona_image;
extern u_long       *g_btl_persona_tim;
extern const u_long **g_btl_effect_gfx;
extern u_short       g_btl_tpage[];

extern int     BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int nclut);

#ifdef NON_MATCHING
BtlObj *BtlSpawnPersona(int gfx, int col, int row, int motion)
{
    BtlObj *obj;
    BtlObj *prev;
    BtlObj *first;
    long    pos[3];
    int     layer;
    int     y;
    int     which;

    /* A second name for the id, used only by the loop below. It looks
       redundant and is not: it is what keeps the table's index in the register
       the original uses. */
    which = gfx;
    g_btl_persona_image = PERSONA_IMAGE;
    memcpy(PERSONA_IMAGE, g_load_stage[1], PERSONA_IMAGE_BYTES);
    BtlBindGfx(GFX_EFFECT, gfx, &g_btl_persona_image);
    g_btl_persona_tim = BtlUploadTim(g_load_stage[0], PERSONA_PAGE,
                                     PERSONA_SLOT, PERSONA_ABR, 0, 1);

    /* Spelt out in both arms so gcc cross-jumps the shared tail; hoisting the
       lookup into a variable of its own puts the store in the wrong place. */
    if (motion < 0) {
        g_btl_persona_def.scripts = (const u_long **)
            g_btl_effect_gfx[g_btl_persona_gfx[gfx].script[PERSONA_SCRIPT_DEFAULT]];
    } else {
        g_btl_persona_def.scripts = (const u_long **)
            g_btl_effect_gfx[g_btl_persona_gfx[gfx].script[motion]];
    }

    /* The row's depth is reached twice and computed once - the original keeps
       it in a register across the test below. */
    y = row * PERSONA_ROW_H;
    pos[0] = (col * PERSONA_COL_W - PERSONA_COL_X) << 16;
    pos[1] = (y + PERSONA_ROW_NEAR) << 16;
    pos[2] = PERSONA_Z;
    g_btl_tpage[PERSONA_PAGE + 1] = g_btl_tpage[PERSONA_PAGE] | TPAGE_ABR_ADD;
    if (gfx > PERSONA_FAR_ID) {
        pos[1] = (y - PERSONA_ROW_FAR) << 16;
    }

    layer = 0;
    prev = 0;
    do {
        obj = BtlObjAlloc(&g_btl_persona_def, PERSONA_GROUP, prev, PERSONA_KIND,
                          0, pos, PERSONA_PAGE, PERSONA_SLOT);
        obj->scripts = g_btl_effect_gfx;
        obj->timer = layer * PERSONA_STAGGER;
        obj->kind = gfx;
        obj->unkD2 = layer;
        obj->fade = PERSONA_FADE;
        obj->col2 = col;
        obj->row = row;
        obj->attr |= g_btl_persona_gfx[which].attr;
        if (layer != 0) {
            obj->attr |= BTL_OBJ_TRAIL;
            obj->unkCD = PERSONA_TRAIL_CD;
            prev->attached = obj;
        } else {
            first = obj;
        }
        layer++;
        prev = obj;
    } while (layer < PERSONA_LAYERS);

    return first;
}
#else
INCLUDE_ASM("btlp/nonmatchings/personaspawn", BtlSpawnPersona);
#endif

