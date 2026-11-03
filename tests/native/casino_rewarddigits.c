/* Only drop the PS1 symbol alias: the same C uses a host-sized model array. */
#define __asm__(symbol)
#include "../../src/casino/game/rewarddigits.c"
#undef __asm__
extern int puts(const char *);
CasinoModel reward_models[3];
static CasinoObj objects[5];
static short digits[3];
static int value, change, events, bad;

void CasinoStartAnim(CasinoObj *object, short frames, short rx, short ry, short rz,
                     long tx, long ty, long tz, long sx, long sy, long sz)
{
    CasinoObj *expected;
    if (value < 10) {
        expected = &objects[0];
        if (events != 0 || digits[0] != value) bad = 1;
    } else {
        expected = &objects[(events == 2 ? 1 : 2) + (change ? 2 : 0)];
        if (events != 2 && events != 3) bad = 1;
    }
    if (object != expected || frames != 16 || rx != 1024 || ry || rz ||
        tx || ty || tz || sx || sy || sz) bad = 1;
    events++;
}

void CasinoTexObj(CasinoModel *model)
{
    if (value < 10) {
        if (events != 1 || model != &reward_models[0]) bad = 1;
    } else {
        if (events > 1 || model != &reward_models[2 - events] ||
            digits[1] != value / 10 || digits[2] != value % 10) bad = 1;
        if (change && events == 0) {
            reward_models[1].obj = &objects[3];
            reward_models[2].obj = &objects[4];
        }
    }
    events++;
}

int main(void)
{
    int i;
    for (value = 0; value < 256; value++) for (change = 0; change < 2; change++) {
        for (i = 0; i < 3; i++) {
            digits[i] = -123;
            reward_models[i].tex_idx = &digits[i];
            reward_models[i].obj = &objects[i];
        }
        events = bad = 0;
        func_800913FC(value);
        if (bad || events != (value < 10 ? 2 : 4)) return 1;
        if (value < 10 ? digits[1] != -123 || digits[2] != -123 : digits[0] != -123) return 2;
        for (i = 0; i < 3; i++) if (reward_models[i].tex_idx != &digits[i]) return 3;
    }
    puts("512 native reward-digit fixtures passed");
    return 0;
}
