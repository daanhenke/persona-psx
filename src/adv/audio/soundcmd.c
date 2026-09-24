/* Persona 1 (JP) - the scene script's sound commands.  ADV only.
 *   0x800882FC AdvSoundCommand
 *
 * One byte from the script says what to do with the sound banks:
 *   0x80..0x8E  make sure the bank a scene's sound set wants is loaded
 *   0xF0..0xFB  even: fade the sequence in slot (cmd & 0xF) / 2 out
 *               odd:  stop it and give the slot's VAB back
 *   otherwise   load the sound set by number, if it is not already, and play
 *               it from the start
 * g_sound_bank says which of the six bank slots each sound set goes in.
 */
#include <decomp/types.h>
#include <libsnd.h>

typedef struct {
    /* 0x00 */ u_short file_id;    /* which SE set is in this slot */
    /* 0x02 */ u_short vab_id;
} AdvBankHead;

extern AdvBankHead *g_adv_banks[];
extern u_char       g_sound_bank[];

/* Indexed by the command itself, so the table proper starts 0x80 in. */
extern u_char g_scene_sound[];

/* Sits just past g_seq_handle; reached by hardcoded address. */
#define g_bank_seq ((short *)0x801F539E)

#define NO_SEQ 0xFFFF

extern void AdvLoadSe(short id, short slot);

void AdvSoundCommand(short cmd)
{
    short       *fade;   /* the fading case keeps its own pointer */
    short       *seq;
    AdvBankHead *bank;
    int          slot;

    switch (cmd) {
    case 0xF0:
    case 0xF2:
    case 0xF4:
    case 0xF6:
    case 0xF8:
    case 0xFA:
        fade = &g_bank_seq[(cmd & 0xF) >> 1];
        if ((u_short)*fade != NO_SEQ) {
            SsSeqStop(*fade);
            SsSeqSetDecrescendo(*fade, 0x7F, 0x80);
        }
        break;

    case 0xF1:
    case 0xF3:
    case 0xF5:
    case 0xF7:
    case 0xF9:
    case 0xFB:
        cmd = (cmd & 0xF) >> 1;
        seq = &g_bank_seq[cmd];
        if ((u_short)*seq != NO_SEQ) {
            bank = g_adv_banks[cmd];
            SsSeqStop(*seq);
            SsSetNck(*seq);
            *(u_short *)seq = NO_SEQ;
            SsVabClose(bank->vab_id);
            bank->vab_id = NO_SEQ;
            bank->file_id = NO_SEQ;
        }
        break;

    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84:
    case 0x85: case 0x86: case 0x87: case 0x88: case 0x89:
    case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E:
        slot = g_sound_bank[g_scene_sound[cmd]];
        bank = g_adv_banks[slot];
        if (bank->file_id != g_scene_sound[cmd]) {
            if ((u_short)g_bank_seq[slot] != NO_SEQ) {
                SsSetNck(g_bank_seq[slot]);
                SsVabClose(bank->vab_id);
            }
            AdvLoadSe(g_scene_sound[cmd], slot);
        }
        break;

    default:
        slot = g_sound_bank[cmd];
        bank = g_adv_banks[slot];
        if (bank->file_id != cmd) {
            if ((u_short)g_bank_seq[slot] != NO_SEQ) {
                SsSetNck(g_bank_seq[slot]);
                SsVabClose(bank->vab_id);
            }
            AdvLoadSe(cmd, slot);
        }
        SsSeqStop(g_bank_seq[slot]);
        SsSeqPlay(g_bank_seq[slot], 1, 1);
        break;
    }
}
