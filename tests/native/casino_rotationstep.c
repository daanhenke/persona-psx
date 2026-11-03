#include <persona/casino/rotation.h>
extern int puts(const char *);

static int calls, change_snap;
static double expected_angle;
static int bad_callback;

void func_80091D0C(CasinoRotationResponse *value)
{
    calls++;
    if (value->angle != expected_angle) bad_callback = 1;
    if (change_snap >= 0) value->snap = change_snap;
    value->step = 7.0;
    value->velocity = 13.0;
}

static int check(double start, double step, unsigned char snap,
                 int override, double integrated, short expected,
                 unsigned char final_snap)
{
    CasinoRotationResponse value = {0};
    short result;
    value.angle = start; value.step = step; value.velocity = 42.0;
    value.snap = snap; value.mode = 3; value.band = 4; value.previous_band = 5;
    calls = bad_callback = 0; change_snap = override; expected_angle = integrated;
    result = func_80091B78(&value);
    if (calls != 1 || bad_callback || result != expected ||
        value.snap != final_snap || value.angle != integrated ||
        value.mode != 3 || value.band != 4 || value.previous_band != 5) return 0;
    if (final_snap == 2) return value.step == 0.0 && value.velocity == 0.0;
    return value.step == 7.0 && value.velocity == 13.0;
}

int main(void)
{
    static const struct { int start; short result; } cases[] = {
        {0,1024}, {93,1024}, {94,2048}, {186,2048}, {187,3072},
        {279,3072}, {280,4096}, {372,4096}, {373,1024},
        {-1,-1024}, {-93,-1024}, {-94,-2048}, {-186,-2048},
        {-187,-3072}, {-279,-3072}, {-280,-4096}, {-372,-4096},
        {-373,-1024}, {1024,4096}, {-1024,-4096}, {2048,3072},
        {-2048,-3072}, {3072,2048}, {-3072,-2048}, {4096,1024}
    };
    unsigned int i;
    if (__builtin_offsetof(CasinoRotationResponse, angle) != 0x50 ||
        __builtin_offsetof(CasinoRotationResponse, step) != 0x98 ||
        __builtin_offsetof(CasinoRotationResponse, velocity) != 0xB8 ||
        __builtin_offsetof(CasinoRotationResponse, snap) != 0xE2) return 1;
    for (i=0; i<sizeof(cases)/sizeof(cases[0]); i++)
        if (!check(cases[i].start,0.0,1,-1,cases[i].start,cases[i].result,2)) return 2;
    if (!check(10.0,10.0,0,-1,11.0,121,0)) return 3;
    if (!check(-10.0,-10.0,0,-1,-11.0,-121,0)) return 4;
    if (!check(10.0,10.0,0,1,11.0,1024,2)) return 5;
    if (!check(10.0,10.0,1,0,11.0,121,0)) return 6;
    puts("29 native rotation-step fixtures passed");
    return 0;
}
