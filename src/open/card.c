/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE, the memory card @ 0x80086A98
 *
 * The card's events, and the save file: checking, loading, writing and
 * formatting. Built without optimisation like the rest of this executable.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <kernel.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/open/open.h>

INCLUDE_ASM("open/nonmatchings/card", func_80086A98);

INCLUDE_ASM("open/nonmatchings/card", func_80086D14);

INCLUDE_ASM("open/nonmatchings/card", func_80086E7C);

INCLUDE_ASM("open/nonmatchings/card", func_80086F34);

INCLUDE_ASM("open/nonmatchings/card", func_8008701C);

INCLUDE_ASM("open/nonmatchings/card", func_8008729C);

INCLUDE_ASM("open/nonmatchings/card", func_800874B4);

INCLUDE_ASM("open/nonmatchings/card", func_8008758C);

INCLUDE_ASM("open/nonmatchings/card", func_80087678);

INCLUDE_ASM("open/nonmatchings/card", func_80087900);

INCLUDE_ASM("open/nonmatchings/card", func_80087B04);

INCLUDE_ASM("open/nonmatchings/card", func_800881A8);

INCLUDE_ASM("open/nonmatchings/card", func_80088314);

INCLUDE_ASM("open/nonmatchings/card", func_800885C4);

INCLUDE_ASM("open/nonmatchings/card", func_8008864C);

INCLUDE_ASM("open/nonmatchings/card", func_800886B4);

INCLUDE_ASM("open/nonmatchings/card", func_80088798);

INCLUDE_ASM("open/nonmatchings/card", func_80088800);

INCLUDE_ASM("open/nonmatchings/card", func_800888C4);

INCLUDE_ASM("open/nonmatchings/card", func_800889A4);

INCLUDE_ASM("open/nonmatchings/card", func_80088B34);

INCLUDE_ASM("open/nonmatchings/card", func_80088BFC);
