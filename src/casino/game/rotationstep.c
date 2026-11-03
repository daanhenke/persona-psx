/* Advance the rotation accumulator, then optionally snap to a quarter turn. */
#include <persona/casino/rotation.h>

short func_80091B78(CasinoRotationResponse *value)
{
    int angle;

    value->angle = 0.1 * value->step + value->angle;
    angle = (int)value->angle * 11;
    angle %= 4096;
    if (angle <= -4096 || angle >= 4096)
        angle -= 4096;

    func_80091D0C(value);
    if (value->snap)
    {
        if (angle >= 0 && angle <= 1023)
        {
            angle = 1024;
            goto snapped;
        }
        else if (angle >= 1024 && angle <= 2047)
        {
            angle = 2048;
            goto snapped;
        }
        else if (angle >= 2048 && angle <= 3071)
        {
            angle = 3072;
            goto snapped;
        }
        else if (angle >= 3072 && angle <= 4096)
        {
            angle = 4096;
            goto snapped;
        }
        else if (angle >= -1023 && angle <= -1)
        {
            angle = -1024;
            goto snapped;
        }
        else if (angle >= -2047 && angle <= -1024)
        {
            angle = -2048;
            goto snapped;
        }
        else if (angle >= -3071 && angle <= -2048)
        {
            angle = -3072;
            goto snapped;
        }
        else if (angle >= -4095 && angle <= -3072)
        {
            angle = -4096;
            goto snapped;
        }
    }
    return (short)angle;

snapped:
    value->step = 0.0;
    value->velocity = 0.0;
    value->snap = 2;
    return (short)angle;
}
