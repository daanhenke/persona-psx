/* Persona 1 (JP) - the resident executable's entry point.  SLPS_005.00 @ 0x8001128C
 *
 * main brings the hardware and the CD up, reads the three file index tables
 * and the font, then runs the scene loop: each pass takes the scene an overlay
 * left in g_state_next and loads the overlay for it, which runs until it
 * returns with the next one set. Scene -1 is the boot: the logo and opening
 * executables, then the title menu's choice.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <kernel.h>
#include <memory.h>
#include <persona/main/cd.h>
#include <persona/main/state.h>

/* Defined here: main reaches them gp-relative. */
volatile int g_cd_busy;
int          g_state_cur;
int          g_state_prev;
int          g_state_next;

extern const char g_cd_filenames[];
extern int        g_cd_file_lba[];
extern int        g_cd_file_size[];

extern void LoadAndExecPsExe(const char *name);
extern void LoadOverlay(Overlay *ovl);
extern void PreloadName(void);

/* Each overlay's entry point, at the start of its image. */
extern void ovl_dng_entry(void);
extern void ovl_btlp_entry(void);
extern void ovl_s2d_entry(void);
extern void ovl_adv_entry(void);
extern void ovl_casino_entry(void);
extern void ovl_name_entry(void);

extern u_char  g_map_pos_x;
extern u_char  g_map_pos_y;
extern u_short g_save_map_id;
extern u_short g_save_unk4;
extern u_char  g_save_pos_x;
extern u_char  g_save_pos_y;
extern u_char  g_save_unk5_idx;
extern u_char  g_save_unk11;
extern u_short g_save_unk14;
extern u_short g_save_unk16;
extern u_char  g_title_choice;
/* Which movie MOVIE.EXE plays; 0x30 and up (below 0x8000) is the ending.
   The top bit, read as the high byte on its own, sends the game back to the
   dungeon afterwards. */
extern u_short g_movie_id;
extern u_char  g_movie_id_hi;
extern int     g_movie_next_state;
extern u_char  g_save_unk29A9;
extern u_short g_adv_start_scene;
extern u_char  g_map_state_lut[];

/* The names typed in at the start, copied over the text tables' defaults. */
extern u_char g_text_names[0x14];
extern u_char g_text_hero_name[8];
#define g_entered_names ((u_char *)0x801F298C)
#define g_hero_name     ((u_char *)0x801F29A0)

#define g_map_unk4 (*(u_char *)0x801F5354)
#define g_map_room (*(u_char *)0x801F5355)

#define SAVE_WORK      ((u_char *)0x801F0000)
#define SAVE_WORK_SIZE 0x8000
#define FONT_DEST      ((void *)0x801E0000)
#define OPEN_DEST      ((void *)0x80180000)

/* The title menu. */
#define TITLE_NEW       0
#define TITLE_ADV       1
#define TITLE_NEW2      2
#define TITLE_DNG       3
#define TITLE_CONTINUE  4

int main(void)
{
    Overlay ovl[7] = {
        { "\\DNG.BIN;1",    ovl_dng_entry },
        { "\\DNG.BIN;1",    ovl_dng_entry },
        { "\\BTLP.BIN;1",   ovl_btlp_entry },
        { "\\S2D.BIN;1",    ovl_s2d_entry },
        { "\\ADV.BIN;1",    ovl_adv_entry },
        { "\\CASINO.BIN;1", ovl_casino_entry },
        { "\\NAME.BIN;1",   ovl_name_entry },
    };
    CdlFILE file;
    u_char  unused[0x40];
    int     first;

    ResetCallback();
    while (!CdInit())
        ;
    ResetGraph(0);
    PadInit(0);
    SsInit();
    first = 1;
    InitCARD(1);
    StartCARD();
    _bu_init();
    bzero(SAVE_WORK, SAVE_WORK_SIZE);
    g_cd_busy = -1;
    g_state_cur = -1;
    g_state_next = -1;

    while (!CdSearchFile(&file, "\\FNAME.DAT;1"))
        ;
    CdReadFileToAddr(&file, (file.size + 0x7FF) >> 11, (u_long *)g_cd_filenames);
    while (!CdSearchFile(&file, "\\FSECT.DAT;1"))
        ;
    CdReadFileToAddr(&file, (file.size + 0x7FF) >> 11, (u_long *)g_cd_file_lba);
    while (!CdSearchFile(&file, "\\FSIZE.DAT;1"))
        ;
    CdReadFileToAddr(&file, (file.size + 0x7FF) >> 11, (u_long *)g_cd_file_size);
    LoadFileToAddr("\\FONT.BIN;1", FONT_DEST);

    while (1) {
        g_state_prev = g_state_cur;
        g_state_cur = g_state_next;
        switch (g_state_cur) {
        case -1:
            if (!first) {
                ResetCallback();
                while (!CdInit())
                    ;
                ResetGraph(0);
                PadInit(0);
                SsInit();
                bzero(SAVE_WORK, SAVE_WORK_SIZE);
                g_cd_busy = -1;
            }
            first = 0;
            bzero(SAVE_WORK, SAVE_WORK_SIZE);
            g_cd_busy = -1;
            SsEnd();
            SsQuit();
            LoadAndExecPsExe("\\EXE\\ATLUS.EXE;1");
            LoadFileToAddr("\\OPEN.BIN;1", OPEN_DEST);
            LoadAndExecPsExe("\\EXE\\OPEN.EXE;1");
            ResetGraph(0);
            PadInit(0);
            SsEnd();
            SsQuit();
            SsInit();

            g_map_pos_x = 0x68;
            g_map_pos_y = 0x69;
            g_save_unk14 = 0xFFFE;
            g_map_id = 0;
            g_map_unk4 = 0;
            g_map_room = 0;
            g_save_unk16 = 0xFFFF;
            g_save_unk11 = 0;
            switch (g_title_choice) {
            case TITLE_ADV:
                g_map_id = 0;
                g_map_unk4 = 0;
                g_map_room = 0;
                PreloadAdv();
                g_state_next = GAME_STATE_ADV;
                break;
            case TITLE_NEW:
            case TITLE_NEW2:
                bcopy(g_entered_names, g_text_names, sizeof(g_text_names));
                bcopy(g_hero_name, g_text_hero_name, sizeof(g_text_hero_name));
                g_map_room = 8;
                g_map_id = g_adv_start_scene;
                PreloadAdv();
                g_state_next = GAME_STATE_ADV;
                break;
            case TITLE_DNG:
                g_map_pos_x = 2;
                g_map_pos_y = 0;
                PreloadDng();
                g_state_next = GAME_STATE_DNG;
                break;
            case TITLE_CONTINUE:
                bcopy(g_entered_names, g_text_names, sizeof(g_text_names));
                bcopy(g_hero_name, g_text_hero_name, sizeof(g_text_hero_name));
                g_map_id = g_save_map_id;
                g_map_unk4 = g_save_unk4;
                g_map_pos_x = g_save_pos_x;
                g_map_pos_y = g_save_pos_y;
                g_map_room = g_map_state_lut[g_save_unk5_idx];
                PreloadDng();
                g_state_next = GAME_STATE_DNG;
                break;
            }
            break;
        case GAME_STATE_DNG:
            LoadOverlay(&ovl[1]);
            break;
        case GAME_STATE_BTL:
            LoadOverlay(&ovl[2]);
            break;
        case GAME_STATE_S2D:
            g_save_unk14 = 0xFFFF;
            g_save_unk29A9 = 0;
            LoadOverlay(&ovl[3]);
            break;
        case GAME_STATE_ADV:
            LoadOverlay(&ovl[4]);
            break;
        case GAME_STATE_CASINO:
            LoadOverlay(&ovl[5]);
            break;
        case GAME_STATE_NAME:
            LoadOverlay(&ovl[6]);
            g_state_next = GAME_STATE_MOVIE;
            g_movie_id = 0x12;
            break;
        case GAME_STATE_MOVIE:
            if (g_movie_id < 0x30 || g_movie_id >= 0x8000) {
                LoadAndExecPsExe("\\EXE\\MOVIE.EXE;1");
                if (g_movie_id_hi >> 7) {
                    PreloadDng();
                    g_state_next = GAME_STATE_DNG;
                    break;
                }
                switch (g_state_next = g_movie_next_state) {
                case GAME_STATE_DNG:
                    PreloadDng();
                    break;
                case GAME_STATE_ADV:
                    PreloadAdv();
                    break;
                case GAME_STATE_S2D:
                    PreloadS2d();
                    break;
                case GAME_STATE_NAME:
                    PreloadName();
                    break;
                }
            } else {
                LoadAndExecPsExe("\\EXE\\END.EXE;1");
                if (g_movie_next_state == GAME_STATE_ADV) {
                    PreloadAdv();
                }
                g_state_next = g_movie_next_state;
            }
            break;
        }
    }
}
