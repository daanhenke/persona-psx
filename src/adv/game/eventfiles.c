/* Persona 1 (JP) - loading the files a scene names.  ADV only.
 *   0x800850B8 AdvLoadImageFile   0x8008514C AdvPickSceneByFlags
 *   0x80085440 AdvLoadEventBg
 *
 * The scene pack's head (AdvPackHead, scene.h) names the other files a scene
 * needs. AdvPickSceneByFlags loads the scene's MES entry - picked out of four
 * by two story flags - and puts its eight images into VRAM. AdvLoadEventBg
 * loads the EBG entry: an archive of images, each going to its own VRAM
 * column, and for the larger room kinds a block of room data copied into the
 * work area.
 *
 * Both archives are packed (unpack.c) and start with a table of byte offsets
 * to their members, one word each, counted from the table itself.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/adv/scene.h>
#include <persona/common/eventflag.h>
#include <persona/common/imageanim.h>

#define NONE 0xFFFF

#define FILE_MES 1
#define FILE_EBG 2

/* Where the packed files land and where they are unpacked to. The first
   eight bytes of a read are the archive entry's header. */
#define MES_READ    ((u_long *)0x80118000)
#define MES_PACKED  ((u_char *)0x80118008)
#define MES_AT      ((u_long *)0x80178000)
#define MES_SIZE    0x3F900
#define EBG_READ    ((u_long *)0x80160000)
#define EBG_PACKED  ((u_char *)0x80160008)
#define EBG_AT      ((u_long *)0x80118000)

/* Member n of an unpacked archive. */
#define MEMBER(at, n) ((u_long *)((u_char *)(at) + (at)[n]))

/* The MES entry's images: eight across the VRAM row at y 0x100, 64 apart,
   with their palettes one row each from 0x1F0. */
#define MES_IMAGES  8
#define MES_X0      0x180
#define MES_PITCH   64
#define MES_Y       0x100
#define MES_CLUT_Y0 0x1F0

/* The room data the larger room kinds carry, by kind. */
#define g_room_data ((u_char *)0x800EB780)

extern CdlFILE g_adv_scene_file;

extern void LoadFileToAddr(const char *name, void *dest);
extern void AdvSelectFile(short kind, short id);
extern void AdvUnpack(u_char *src, u_char *dst, u_int size);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void bcopy(void *src, void *dst, int len);

/* Story flag `id`, tested through a pointer to the flag bytes. */
static inline int FlagSet(short id)
{
    u_char *flags;

    flags = g_event_flags;
    return flags[id / 8] & (1 << (id & 7));
}

/* Loads an image file by name and puts it into VRAM straight away. */
void AdvLoadImageFile(const char *name, u_long *buf, short x, short y,
                      short cx, short cy)
{
    LoadFileToAddr(name, buf);
    TimQueueAt(buf, x, y, cx, cy);
    FlushImageUploads();
    DrawSync(0);
}

/* 98.89%: the second test against 0xFFFF loads the constant afresh in the
   original, where gcc here keeps it in a register from the first. */
#ifdef NON_MATCHING
void AdvPickSceneByFlags(void)
{
    u_short id;
    u_short second;

    id = g_adv_pack->mes_flag[0];
    second = g_adv_pack->mes_flag[1];
    if ((id == NONE && second == NONE) ||
        (!FlagSet(id) && (!FlagSet(second) || second == NONE))) {
        id = g_adv_pack->mes[0];
    } else if (FlagSet(id) && (!FlagSet(second) || second == NONE)) {
        id = g_adv_pack->mes[1];
    } else if (!FlagSet(id) && FlagSet(second)) {
        id = g_adv_pack->mes[2];
    } else if (FlagSet(id) && FlagSet(second)) {
        id = g_adv_pack->mes[3];
    }

    AdvSelectFile(FILE_MES, id);
    CdReadFileToAddr(&g_adv_scene_file, g_adv_scene_file.size, MES_READ);
    AdvUnpack(MES_PACKED, (u_char *)MES_AT, MES_SIZE);
    /* id now counts the images. */
    for (id = 0; id < MES_IMAGES; id++) {
        TimQueueAt(MEMBER(MES_AT, id), id * MES_PITCH + MES_X0, MES_Y, 0,
                   id + MES_CLUT_Y0);
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/eventfiles", AdvPickSceneByFlags);
#endif

/* The id a variant resolves to: `alt` once its flag is set. */
#define VARIANT(v)                                                             \
    ((v).flag != NONE && FlagSet((v).flag) ? (v).alt : (v).id)

/* 97.92%: the original tests the EBG id against "none" a second time after
   the read and keeps both tests; gcc here proves the second one redundant. */
#ifdef NON_MATCHING
void AdvLoadEventBg(void)
{
    u_short n;
    u_short id;

    if (g_adv_pack->bg.flag != NONE && FlagSet(g_adv_pack->bg.flag)) {
        n = g_adv_pack->bg.alt;
    } else {
        n = g_adv_pack->bg.id;
    }
    if (n != NONE) {
        AdvSelectFile(FILE_EBG, n);
        CdReadFileToAddr(&g_adv_scene_file, g_adv_scene_file.size, EBG_READ);
    }
    if (n == NONE) {
        return;
    }
    AdvUnpack(EBG_PACKED, (u_char *)EBG_AT, -1);

    /* n now counts the archive's members as they are used. */
    n = 0;
    id = VARIANT(g_adv_pack->tim_180);
    if (id != NONE) {
        TimQueueAt(MEMBER(EBG_AT, n), 0x180, 0, 0, 0x1E0);
        TimQueueAt(MEMBER(EBG_AT, n), 0x180, 0, 0, 0x1E0);
        n++;
    }
    id = VARIANT(g_adv_pack->tim_300);
    if (id != NONE) {
        TimQueueAt(MEMBER(EBG_AT, n), 0x300, 0, 0, 0x1E8);
        n++;
    }
    id = VARIANT(g_adv_pack->tim_280);
    if (id != NONE) {
        TimQueueAt(MEMBER(EBG_AT, n), 0x280, 0, 0, 0x1E1);
        n++;
    }
    if (g_adv_pack->tim_160 != NONE) {
        TimQueueAt(MEMBER(EBG_AT, n), 0x160, 0, 0x110, 0x1EF);
        n++;
    }
    if (g_adv_pack->tim_380 != NONE) {
        TimQueueAt(MEMBER(EBG_AT, n), 0x380, 0xA0, 0, 0x1E7);
        n++;
    }
    FlushImageUploads();

    switch (g_adv_scene->kind) {
    case 2:
        bcopy(MEMBER(EBG_AT, n), g_room_data, 0xBE8);
        break;
    case 3:
        bcopy(MEMBER(EBG_AT, n), g_room_data, 0x13F4);
        break;
    case 4:
        bcopy(MEMBER(EBG_AT, n), g_room_data, 0x1458);
        break;
    }
    DrawSync(0);
}
#else
INCLUDE_ASM("adv/nonmatchings/game/eventfiles", AdvLoadEventBg);
#endif
