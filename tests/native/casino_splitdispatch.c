#include <persona/casino/blackjack.h>
extern int puts(const char *);
u_char g_bj_play_step;
static int called, count, bad_pointer;
#define HANDLER(name, state) void name(void) { called = state; count++; g_bj_play_step=255; }
HANDLER(func_8007AFC0,18)
HANDLER(func_8007B12C,45)
HANDLER(func_8007B208,46)
HANDLER(func_8007B744,33)
HANDLER(func_8007B8B0,44)
HANDLER(func_8007BA14,27)
HANDLER(func_8007BA60,112)
HANDLER(func_8007C080,113)
HANDLER(func_8007C0E4,42)
HANDLER(func_8007C158,37)
HANDLER(func_8007C2B8,120)
HANDLER(func_8007C358,121)
HANDLER(func_8007CBE4,48)
void func_8007D3CC(u_char *step) {
    called=116; count++; bad_pointer=(step != &g_bj_play_step);
    *step=255;
}
int main(void) {
    static const unsigned char states[] = {18,27,33,37,42,44,45,46,48,112,113,116,120,121};
    int state, i, expected;
    for(state=0;state<256;state++) {
        expected=-1;
        for(i=0;i<(int)sizeof(states);i++) if(states[i]==state) expected=state;
        called=-1; count=bad_pointer=0; g_bj_play_step=state;
        func_8007ADD8();
        if(called!=expected || count!=(expected>=0) || bad_pointer) return 1;
        if(g_bj_play_step!=(expected>=0 ? 255 : state)) return 2;
    }
    puts("256 native split-dispatch state fixtures passed");
    return 0;
}
