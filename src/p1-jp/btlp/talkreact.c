/* Persona 1 (JP) - what the demon does when it is spoken to.  BTLP only.
 *   0x8006BD40 BtlTalkReact
 *
 * Every reaction starts from the same state: the object stops whatever motion
 * it was in, its colour is put back to an even grey, and it walks toward that
 * colour two steps a frame. Then one of five scripts is armed, chosen by the
 * kind handed in and looked up in the demon's own model record.
 *
 * The five are indices into the object's script table rather than scripts, so
 * the same kind gives every demon its own animation.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>

/* The state every reaction starts from. */
#define BTL_REACT_GREY 0x80
#define BTL_REACT_FADE 2

extern BtlActor g_btl_enemies[];
extern short    g_btl_talk_target;

extern void BtlObjSetMotion(BtlObj *obj, u_char motion);
extern void BtlObjSetRgb(BtlObj *obj, short r, int g, short b);
extern void BtlObjSetFade(BtlObj *obj, u_char rate);
extern void BtlObjSetScript(BtlObj *obj, const u_long *script);

void BtlTalkReact(u_char kind)
{
    BtlActor *a;
    BtlObj   *obj;
    int       script;

    a = &g_btl_enemies[g_btl_talk_target];
    BtlObjSetMotion(a->obj, 0);
    BtlObjSetRgb(a->obj, BTL_REACT_GREY, BTL_REACT_GREY, BTL_REACT_GREY);
    BtlObjSetFade(a->obj, BTL_REACT_FADE);

    switch (kind) {
    case 0:
        script = g_btl_models[a->c.key].react0;
        obj = a->obj;
        break;
    case 1:
        script = g_btl_models[a->c.key].react1;
        obj = a->obj;
        break;
    case 2:
        script = g_btl_models[a->c.key].react2;
        obj = a->obj;
        break;
    case 3:
        script = g_btl_models[a->c.key].react3;
        obj = a->obj;
        break;
    default:
        script = g_btl_models[a->c.key].talk;
        obj = a->obj;
        break;
    }
    BtlObjSetScript(obj, obj->scripts[script]);
}
