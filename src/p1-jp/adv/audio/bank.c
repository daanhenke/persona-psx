/* Persona 1 (JP) - loading a sound bank off the disc.  ADV @ 0x80088554.
 *
 * A bank slot holds one SE.BIN/SVB.BIN pair: the sequences and the VAB they
 * play through. The file is read into the slot's buffer at +8, so the first
 * eight bytes stay free for what is loaded there now.
 *
 * The VAB arrives in two halves, as libsnd wants it. The header comes down
 * with the rest of the file and goes to SsVabOpenHead, which allocates a vab
 * id; the body is read separately into 0x80150000 and pushed to sound RAM with
 * SsVabTransBody, which is asynchronous, so the load waits on
 * SsVabTransCompleted before opening the sequence against the new vab id.
 *
 * Every wait spins on AdvRunFrame rather than blocking, which is what keeps
 * the screen drawing while a load runs.
 */
#include <types.h>
#include <libcd.h>

typedef struct {
    /* 0x00 */ u_short file_id;    /* which SE set is in this slot           */
    /* 0x02 */ u_short vab_id;     /* from SsVabOpenHead                     */
    /* 0x04 */ short  seq;        /* from SsSeqOpen                         */
    /* 0x06 */ short  pad06;
    /* 0x08 */ int    unk08;      /* the file is read in from here on       */
    /* 0x0C */ int    unk0C;
    /* 0x10 */ int    unk10;
    /* 0x14 */ int    vab_offset; /* header sits at bank + this + 0x10      */
    /* 0x18 */ u_long seq_data[1];
} AdvBank;

/* The body is staged here before it goes to sound RAM; SsVabTransBody is
   handed the payload eight bytes in. */
#define VAB_BODY_STAGE 0x80150000

extern AdvBank *g_adv_banks[];
/* Sits just past g_seq_handle; reached by hardcoded address. */
#define g_bank_seq ((short *)0x801F539E)

extern void  AdvSelectFile(short kind, short id);
extern void  AdvRunFrame(void);
extern void  CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern short SsVabOpenHead(u_char *addr, short vabid);
extern short SsVabTransBody(u_char *addr, short vabid);
extern short SsVabTransCompleted(short immediateFlag);
extern short SsSeqOpen(u_long *addr, short vabid);
extern void  SsSeqStop(short seq);
extern void  SsSetNck(short seq);
extern void  SsVabClose(short vabid);
extern void  SsSeqSetVol(short seq, short voll, short volr);

/* AdvSelectFile leaves the resolved path here and puts the read length in
   the size field, so the loaders never name a file themselves. */
extern CdlFILE      g_adv_scene_file;
extern volatile int g_cd_busy;

void AdvLoadSe(short id, short slot)
{
    AdvBank *bank;

    bank = g_adv_banks[slot];

    AdvSelectFile(9, id);
    CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                          (u_long *)&bank->unk08);
    while (g_cd_busy != -1) {
        AdvRunFrame();
    }
    bank->vab_id = SsVabOpenHead((u_char *)bank + bank->vab_offset + 0x10, -1);

    AdvSelectFile(10, id);
    CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                          (u_long *)VAB_BODY_STAGE);
    while (g_cd_busy != -1) {
        AdvRunFrame();
    }
    SsVabTransBody((u_char *)(VAB_BODY_STAGE + 8), bank->vab_id);
    while (SsVabTransCompleted(0) == 0) {
        AdvRunFrame();
    }

    g_bank_seq[slot] = SsSeqOpen(bank->seq_data, bank->vab_id);
    bank->seq = g_bank_seq[slot];
    SsSeqSetVol(g_bank_seq[slot], 0x7F, 0x7F);
    bank->file_id = id;
}

/* Marks every slot empty. Six is the slot count. */
void AdvResetBanks(void)
{
    AdvBank *bank;
    int      i;

    for (i = 0; i < 6; i++) {
        bank = g_adv_banks[i];
        bank->file_id = 0xFFFF;
        bank->vab_id = 0xFFFF;
        g_bank_seq[i] = -1;
    }
}

/* Stops whatever each slot is playing and gives its VAB back, without
   clearing the slot. */
void AdvCloseBanks(void)
{
    AdvBank *bank;
    int      i;

    for (i = 0; i < 6; i++) {
        bank = g_adv_banks[i];
        if (bank->file_id != 0xFFFF) {
            SsSeqStop(g_bank_seq[i]);
            SsSetNck(g_bank_seq[i]);
        }
        if (bank->vab_id != 0xFFFF) {
            SsVabClose(bank->vab_id);
        }
    }
}
