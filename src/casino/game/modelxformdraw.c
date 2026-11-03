#include <persona/casino/casino.h>

extern void CasinoTexObj(CasinoModel*);
extern void CasinoSetXform(CasinoXform*);
extern void CasinoDrawObj(CasinoObj*);

void func_80092DEC(CasinoModel* model, SVECTOR rotation, VECTOR translation, VECTOR scale)
{
    model->obj->xform->rot.vx   = rotation.vx;
    model->obj->xform->rot.vy   = rotation.vy;
    model->obj->xform->rot.vz   = rotation.vz;
    model->obj->xform->trans.vx = translation.vx;
    model->obj->xform->trans.vy = translation.vy;
    model->obj->xform->trans.vz = translation.vz;
    model->obj->xform->scale.vx = scale.vx;
    model->obj->xform->scale.vy = scale.vy;
    model->obj->xform->scale.vz = scale.vz;
    CasinoTexObj(model);
    CasinoSetXform(model->obj->xform);
    CasinoDrawObj(model->obj);
}
