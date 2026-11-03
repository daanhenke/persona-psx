#include <decomp/types.h>

extern u32 held __asm__("D_800B0E84+0x20");
extern u32 previous __asm__("D_800B0E84+0x24");
extern u32 pressed __asm__("D_800B0E84+0x28");
extern u32 repeated __asm__("D_800B0E84+0x2C");
extern u32 repeat_count __asm__("D_800B0E84+0x30");
extern s32 PadRead(s32);

void func_8008E1C0(void)
{
    s32 current = PadRead(0);
    u32 old = previous;
    held = current;
    previous = current;
    pressed = ~old & current;
    if (current == 0)
    {
        repeat_count = 0;
    }
    else
    {
        repeat_count++;
        if (repeat_count >= 8)
        {
            repeat_count = 7;
        }
    }
    if (repeat_count == 1 || repeat_count == 7)
    {
        repeated = held;
    }
    else
    {
        repeated = 0;
    }
}
