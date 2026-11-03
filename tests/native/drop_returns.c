#include <persona/common/item.h>
extern int puts(const char *);
static const int *rolls;
static int roll_count, roll_index, failed;
static unsigned int slot_result;
static unsigned short first_allowed;
int rand(void) {
    if (roll_index == roll_count) { failed = 1; return 0; }
    return rolls[roll_index++];
}
void srand(unsigned int seed) { if (seed != 123) failed = 1; }
int VSync(int mode) { if (mode != -1) failed = 1; return 123; }
unsigned int BtlItemSlot(unsigned short item) {
    return item >= first_allowed ? slot_result : 0;
}
static int check(int (*fn)(void), const int *values, int count,
                 unsigned short minimum, unsigned int slot, int expected) {
    int actual;
    rolls=values; roll_count=count; roll_index=0; failed=0;
    first_allowed=minimum; slot_result=slot;
    actual=fn();
    return actual == expected && !failed && roll_index == count;
}
int main(void) {
    const int common[]={3};
    const int uncommon1[]={0,0,2};
    const int uncommon2[]={0,1,1};
    const int uncommon_fallback[]={1,4};
    const int rare[]={0,0,1};
    const int nested[]={0,1,0,0,1};
    const int drop_common[]={1,7};
    if (!check(BtlRollCommon, common, 1, 0, 0x70002, 0x70061)) return 1;
    if (!check(BtlRollUncommon, uncommon1, 3, 0, 0x20002, 0x2005A)) return 2;
    if (!check(BtlRollUncommon, uncommon2, 3, 0, 0x20002, 0x2005C)) return 3;
    if (!check(BtlRollUncommon, uncommon_fallback, 2, 0, 1, 0x62)) return 4;
    if (!check(BtlRollDrop, rare, 3, 0, 1, 0x57)) return 5;
    if (!check(BtlRollDrop, nested, 5, 0, 0x40002, 0x40059)) return 6;
    if (!check(BtlRollDrop, drop_common, 2, 0, 1, 0x65)) return 7;
    if (!check(BtlRollCommon, common, 0, 0, 0, 0)) return 8;
    if (!check(BtlRollUncommon, uncommon1, 3, 0x5B, 1, 0x5D)) return 9;
    puts("9 native item-roll return fixtures passed");
    return 0;
}
