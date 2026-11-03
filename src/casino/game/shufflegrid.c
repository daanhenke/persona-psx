/* Persona 1 (JP) - fill the 3x3 grid with distinct random indices. */
#include <decomp/types.h>
extern int rand(void);
extern u8 D_800A87E0[3][3];
void func_8008C8D0(void)
{
    u8 used[9] = {0};
    u8 count = 0;
    u8 choice;
    u8 row;
    u8 column;
    do {
        choice = rand() % 9;
        if (used[choice] == 0) {
            row = count / 3;
            column = count % 3;
            D_800A87E0[row][column] = choice;
            used[choice] = 1;
            count++;
        }
    } while (count < 9);
}
