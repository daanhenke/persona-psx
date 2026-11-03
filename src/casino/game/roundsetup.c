/* Persona 1 (JP) - reset a casino round and queue its opening animations. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
struct CasinoRandomParams;
extern void func_80092430(struct CasinoRandomParams *);
extern void func_800925DC(void);
extern void func_80092910(void);
extern void CasinoSpritesSetOn(int, int, int);
extern void CasinoCursorClear(CasinoCursor *);
extern void CasinoTween();
extern void CasinoStartAnim();
extern s8 g_casino_key_fire[4];
extern s8 g_casino_key_hold[4];
extern char D_800B0720[];
extern u8 init_byte0 __asm__("D_800AAE94");
extern u8 init_byte1 __asm__("D_800AAE94+0x1");
extern u8 init_ready __asm__("D_800AAE9C+0xB");
extern u8 init_pending __asm__("D_800AAE9C+0x2");
extern CasinoLayout *initial_layout __asm__("D_800A2BC8+0x2B30");
extern CasinoObj *initial_object __asm__("D_800A2BC8+0x2C14");
extern CasinoLayout *left_layout __asm__("D_800A2BC8+0x2C68");
extern CasinoLayout *middle_layout __asm__("D_800A2BC8+0x2CC4");
extern CasinoLayout *right_layout __asm__("D_800A2BC8+0x2D20");
extern CasinoObj *last_object __asm__("D_800A2BC8+0x31E0");

void func_80090828(void) {
    SsSeqStop(g_casino_seqs[4]);
    SsSeqPlay(g_casino_seqs[4], 1, 1);
    init_byte0 = 0;
    init_byte1 = 0;
    g_casino_bet_done = 0;
    init_ready = 1;
    func_80092430((struct CasinoRandomParams *)D_800B0720);
    func_800925DC();
    func_80092430((struct CasinoRandomParams *)(D_800B0720 + 0xF0));
    func_80092910();
    CasinoSpritesSetOn(0xED, 0xF, 0);
    CasinoSpritesSetOn(0xFC, 0xF, 0);
    CasinoSpritesSetOn(0x10B, 0xF, 0);
    CasinoSpritesSetOn(0x11A, 0x10, 0);
    CASINO_LAMPS.b0 = 0;
    CASINO_LAMPS.b2 = 0;
    CASINO_LAMPS.b4 = 0;
    CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
    CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
    CasinoTween(initial_layout, 0x140, 0, 0, 0, 0x20);
    CasinoStartAnim(initial_object, 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    CasinoTween(left_layout, -0x140, 0, 0, 0, 1);
    CasinoTween(middle_layout, 0x140, 0, 0, 0, 1);
    CasinoTween(right_layout, -0x140, 0, 0, 0, 1);
    CasinoStartAnim(last_object, 0x18, 0, 0, 0, -0x18, 0, 0, 0, 0, 0);
    g_casino_step = 0x70;
    init_pending = 0;
    g_casino_timer = -1;
}
