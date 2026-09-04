/* Persona 1 (JP) - CASINO overlay @ 0x8007F2B0 */
#include <types.h>

extern void CasinoAnimStep0(void);
extern void CasinoAnimStep1(void);
extern void CasinoAnimStep2(void);

/* Runs the three step animations forwards for dir 0 and backwards for dir 1;
   any other dir does nothing. */
void CasinoPlayStepAnim(char dir, int step)
{
    switch (dir) {
    case 0:
        if (step == 0) {
            CasinoAnimStep0();
        }
        if (step == 2) {
            CasinoAnimStep1();
        }
        if (step == 4) {
            CasinoAnimStep2();
        }
        break;
    case 1:
        if (step == 0) {
            CasinoAnimStep2();
        }
        if (step == 2) {
            CasinoAnimStep1();
        }
        if (step == 4) {
            CasinoAnimStep0();
        }
        break;
    }
}
