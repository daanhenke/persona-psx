#include <persona/casino/casino.h>
#include <libsnd.h>

#include <persona/casino/rotation.h>

void func_80091D0C(CasinoRotationResponse *value)
{
    u_int angle;

    angle = (int)value->angle % 360;
    if (angle < 90U)
    {
        value->band = 0;
    }
    else if (angle - 180 < 90U)
    {
        value->band = 1;
    }
    if (value->previous_band != value->band && value->mode > 0)
    {
        SsSeqStop(g_casino_seqs[9]);
        SsSeqPlay(g_casino_seqs[9], 1, 1);
        if (value->band != 0)
        {
            value->velocity = 0.777 * value->velocity;
        }
        value->previous_band = value->band;
    }
}
