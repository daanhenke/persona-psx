/* Build a card object after initializing the depth and visibility of 20 faces. */
#include <persona/casino/casino.h>
typedef struct {
    CasinoFaceDef *defs;
    CasinoTex *textures;
    short **indices;
    short **texture_indices;
    CasinoObj *objects;
} CasinoCardBank;
extern CasinoCardBank card_bank __asm__("D_8009E8D4");
extern void CasinoBuildObj(CasinoModel *);

void func_800824CC(u_char index, int unused, short kind, short first)
{
    CasinoModel model;
    CasinoObj *object = &card_bank.objects[index];
    CasinoFace *face;
    int offset;
    int i;
    face = object->face;
    object->first = first;
    model.def = card_bank.defs;
    model.tex = 0;
    model.def_idx = card_bank.indices[kind];
    model.tex_idx = 0;
    model.obj = object;
    for (i = 0; i < 20; i++) {
        offset = model.def_idx[i];
        face[i].z = card_bank.defs[offset].z;
        face[i].on = card_bank.defs[offset].on;
    }
    CasinoBuildObj(&model);
}
