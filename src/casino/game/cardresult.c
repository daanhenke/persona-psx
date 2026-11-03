/* Persona 1 (JP) - select the outcome from two flags and card totals. */
#include <decomp/types.h>

int func_80082400(u_char first, u_char second, u_char first_total,
                  u_char second_total)
{
    if (first == 2) {
        return 12;
    } else if (second == 2 || first_total > second_total) {
        return 11;
    } else if (first_total < second_total) {
        return 12;
    } else if (first_total == second_total) {
        return 13;
    }
}
