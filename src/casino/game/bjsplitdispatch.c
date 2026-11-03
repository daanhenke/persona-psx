/* Blackjack split-hand state dispatch.
 * The explicit decision tree preserves the original sparse branch layout;
 * gcc 2.6 turns a flat switch over these states into a jump table instead.
 */
#include <persona/casino/blackjack.h>
extern void func_8007AFC0(void);
extern void func_8007B12C(void);
extern void func_8007B208(void);
extern void func_8007B744(void);
extern void func_8007B8B0(void);
extern void func_8007BA14(void);
extern void func_8007BA60(void);
extern void func_8007C080(void);
extern void func_8007C0E4(void);
extern void func_8007C158(void);
extern void func_8007C2B8(void);
extern void func_8007C358(void);
extern void func_8007CBE4(void);
void func_8007ADD8(void)
{
    int step = g_bj_play_step;

    if (step == 0x2E) goto deal;
    if (step < 0x2F) {
        if (step == 0x25) goto state25;
        if (step < 0x26) {
            if (step == 0x1B) goto choose;
            if (step < 0x1C) {
                if (step == 0x12) goto enter;
                goto done;
            } else {
                if (step == 0x21) goto state21;
                goto done;
            }
        } else {
            if (step == 0x2C) goto state2c;
            if (step >= 0x2D) goto state2d;
            if (step == 0x2A) goto state2a;
            goto done;
        }
    } else {
        if (step == 0x74) goto dealer;
        if (step < 0x75) {
            if (step == 0x70) goto state70;
            if (step < 0x71) {
                if (step == 0x30) goto finish;
                goto done;
            } else {
                if (step == 0x71) goto state71;
                goto done;
            }
        } else {
            if (step == 0x79) goto state79;
            if (step >= 0x7A) goto done;
            if (step == 0x78) goto state78;
            goto done;
        }
    }

enter:   func_8007AFC0(); goto done;
state2d: func_8007B12C(); goto done;
deal:    func_8007B208(); goto done;
state21: func_8007B744(); goto done;
state2c: func_8007B8B0(); goto done;
choose:  func_8007BA14(); goto done;
state70: func_8007BA60(); goto done;
state71: func_8007C080(); goto done;
state2a: func_8007C0E4(); goto done;
dealer:  func_8007D3CC(&g_bj_play_step); goto done;
state25: func_8007C158(); goto done;
state78: func_8007C2B8(); goto done;
state79: func_8007C358(); goto done;
finish:  func_8007CBE4(); goto done;
done:;
}
