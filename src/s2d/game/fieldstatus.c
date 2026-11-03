/* Field ailment ticks and the walking SP recovery timer. */
#include <persona/common/char.h>
#include <persona/common/status.h>

extern int D_800B56EC;
extern int scene_wait __asm__("D_800A5314+0x34C");
extern int scene_tick __asm__("D_800A5314+0x350");
extern int sp_recovery_wait __asm__("D_800A5314+0x354");

void func_80096514(void)
{
    int i;
    Char *p;

    if (D_800B56EC != 0) goto active;
    p = g_chars;
    for (i = 0; i < CHAR_COUNT; i++, p++) {
        if (p->key) {
            if (p->status == STATUS_POISON) {
                D_800B56EC = 1;
                scene_wait = 2;
            }
            if (p->status == STATUS_SICK) goto sickness;
        }
    }
state_ready:
    if (D_800B56EC == 0) goto recovery;
active:
    if (--scene_wait == 0) {
        scene_tick = 3;
        scene_wait = D_800B56EC * 2;
    }
    p = g_chars;
    if (D_800B56EC != 1) goto mixed;
    for (i = 0; i < CHAR_COUNT; i++, p++) {
        if (p->key && p->status == STATUS_POISON && scene_tick == 3) {
            p->hp--;
            if (p->hp <= 0) p->hp = 1;
        }
    }
    goto recovery;

sickness:
    D_800B56EC = 2;
    scene_wait = 4;
    goto state_ready;

mixed:
    for (i = 0; i < CHAR_COUNT; p++, i++) {
        if (p->key) {
            if (p->status == STATUS_POISON && (scene_wait == 2 || scene_wait == 4)) {
                p->hp--;
                if (p->hp <= 0) p->hp = 1;
            }
            {
                u_char status = p->status;
                if (status == STATUS_SICK && scene_tick == 3) {
                    p->hp -= p->hp_max / 8;
                    if (p->hp <= 0) p->hp = 1;
                }
            }
        }
    }
recovery:
    if (--sp_recovery_wait == 0) {
        p = g_chars;
        for (i = 0; i < CHAR_COUNT; i++, p++) {
            if (p->key) {
                int next = p->sp + 1;
                if (p->sp_max < next) next = p->sp_max;
                p->sp = next;
            }
        }
        sp_recovery_wait = 2;
    }
}
