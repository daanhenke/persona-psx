/* Pulse the scene lighting and restart the alert sequence on active frames. */
#include <persona/s2d/s2d.h>
#include <libsnd.h>
extern int D_800B56EC;
extern int scene_tick __asm__("D_800A5314+0x350");
#ifndef PERSONA_ALERT_SEQUENCE
#define PERSONA_ALERT_SEQUENCE ((short *)0x801F5394)
#endif

void func_800967DC(void)
{
    u_char *const rgb = g_fade.rgb;
    GsSetAmbient(0x555, 0x555, 0x555);
    rgb[0] = 96;
    rgb[1] = 96;
    rgb[2] = 96;
    if (D_800B56EC == 0) return;
    switch (D_800B56EC) {
    case 1:
        if (scene_tick == 3) {
            GsSetAmbient(2048, 585, 585);
            rgb[0] = 96;
            rgb[1] = 19;
            rgb[2] = 19;
            SsSeqStop(*PERSONA_ALERT_SEQUENCE);
            SsSeqSetVol(*PERSONA_ALERT_SEQUENCE, 126, 126);
            SsSeqPlay(*PERSONA_ALERT_SEQUENCE, 1, 1);
        }
        if (--scene_tick < 0) scene_tick = 0;
        break;
    case 2:
        if (scene_tick & 1) {
            GsSetAmbient(2048, 585, 585);
            rgb[0] = 96;
            rgb[1] = 19;
            rgb[2] = 19;
            SsSeqStop(*PERSONA_ALERT_SEQUENCE);
            SsSeqSetVol(*PERSONA_ALERT_SEQUENCE, 126, 126);
            SsSeqPlay(*PERSONA_ALERT_SEQUENCE, 1, 1);
        }
        if (--scene_tick < 0) scene_tick = 0;
        break;
    }
}
