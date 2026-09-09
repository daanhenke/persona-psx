/* Persona 1 (JP) - the thresholds BtlTalkSceneStare rolls against.
 *
 * Its own translation unit because the image places it between two scenes'
 * data, and the scene that reads it reaches it by name rather than through
 * either of them.
 *
 * The scene measures rand() % 0x100 against these in turn.
 */
#include <decomp/types.h>

const int g_btl_talk_stare_odds[3] = { 0x80, 0x100, 0x0 };
