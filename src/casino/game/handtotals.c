/* Two-card blackjack totals, with low and high ace values. */
extern unsigned char func_80082340(unsigned char *);
extern int func_800823A0(unsigned char);

void func_8008205C(unsigned char *cards, unsigned char *totals)
{
    int sum;
    switch (func_80082340(cards)) {
    case 0:
        sum = func_800823A0(cards[0]);
        sum += func_800823A0(cards[1]);
        totals[0] = totals[1] = sum;
        break;
    case 1:
        if ((unsigned char)(cards[0] % 13) == 0) {
            totals[0] = func_800823A0(cards[1]) + 1;
            totals[1] = func_800823A0(cards[1]) + 11;
        } else if ((unsigned char)(cards[1] % 13) == 0) {
            totals[0] = func_800823A0(cards[0]) + 1;
            totals[1] = func_800823A0(cards[0]) + 11;
        }
        break;
    case 2:
        totals[0] = 2;
        totals[1] = 12;
        break;
    }
}
