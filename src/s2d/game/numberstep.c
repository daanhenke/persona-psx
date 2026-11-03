/* Update a bounded number from directional input, wrapping at either end. */
#include <decomp/types.h>

extern u_int func_8008E158(int);

int func_8008BCC8(int value, int minimum, int maximum)
{
    int step;

    step = (func_8008E158(0) & 0x20) ? 5 : 1;
    if (func_8008E158(2) & 0x2000) {
        value += step;
        if (maximum < value) {
            value = minimum;
        }
    }
    if (func_8008E158(2) & 0x8000) {
        value -= step;
        if (value < minimum) {
            value = maximum;
        }
    }
    return value;
}
