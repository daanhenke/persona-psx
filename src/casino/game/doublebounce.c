#include <persona/casino/bounce.h>

void func_8009190C(CasinoBounce *value)
{
    value->valueC0 = -0.2 * value->valueC0;
    value->value78 = 7.7 + value->value78;
    if (value->mode == 0)
    {
        value->value98 = 100.0;
    }
}
