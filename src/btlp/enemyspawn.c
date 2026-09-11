/* Persona 1 (JP) - putting the encounter's enemies on the field.  BTLP only.
 *   0x800870C0 BtlSpawnEnemies
 *
 * The overlay's entry point calls this once. Every combatant record is emptied
 * and the occupancy grid cleared, then the encounter's graphics are read to the
 * staging buffer in one go and its nine slots walked. A slot naming a species
 * gets a record, a display object, a marker and a square on the grid.
 *
 * The staging buffer opens with two pointers per species - where its TIM starts
 * and where its image does - so a species' image ends where the next one's TIM
 * begins. The halfword at +2 of a TIM is the palette length; it doubles as the
 * species number in its high byte, and has to be zeroed across the load and put
 * back afterwards because the upload reads the same halfword as header.
 *
 * Two species are special: 0xC2 moves a party member out of the way, and 0xB9
 * is spawned hidden and left off the grid entirely.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* Nine enemy slots to an encounter, two bytes a slot. */
#define BTL_ENEMY_SLOTS 9
#define BTL_ENEMY_ROW   2

/* The grid is 0x2D squares and 0xFF is an empty one. */
#define BTL_GRID_SQUARES 0x2C
#define BTL_GRID_EMPTY   0xFF
#define BTL_GRID_WIDTH   9

/* Where an enemy object's marker number starts, past the five members. */
#define BTL_ENEMY_MARK0 5

/* The two species that are not placed like the rest. */
#define BTL_SPECIES_SHOVE 0xC2
#define BTL_SPECIES_UNSEEN 0xB9

/* Where the shoved member goes. */
#define BTL_SHOVE_MEMBER 6
#define BTL_SHOVE_COL    2

/* The marker's own piece sits this much to the side of the marker. */
#define BTL_MARK_OFFSET 0xC

/* The two encounters that bring fixed enemies along with them. */
#define BTL_FIXED_FIRST 0x1D
#define BTL_FIXED_COUNT 2

/* The staging buffer the encounter is read into, by address. */
#define BTL_STAGE ((u_long **)0x80140000)

/* g_cd_busy once the read is over. */
#define CD_IDLE (-1)

extern BtlModel  g_btl_models[];
extern u_char    g_btl_grid[];
extern u_char    g_btl_encounters[];
extern u_short   g_btl_enemy_gfx_offsets[];
extern int       g_btl_enemy_gfx_base;
extern u_char   *g_btl_gfx_next;
extern u_char   *g_btl_enemy_gfx_start;
extern volatile int g_cd_busy;

extern void    CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern short   BtlLoadEnemyGfx(int species, int slot, u_long *tim,
                               u_char *image, int bytes);
extern BtlObj *BtlSpawnEnemy(int species, int col, int row, short gfx,
                             int depth);
extern void    BtlLoadEnemyStats(int slot, int species);
extern BtlObj *BtlSpawnActorObj(int model, const long *pos);
extern void    BtlPlaceMember(int member, int col, int row);
extern void    BtlSpawnFixedEnemies(void);
extern int     BtlResetTalk(int open);

#ifdef NON_MATCHING
void BtlSpawnEnemies(int set)
{
    CdlLOC    loc;
    BtlActor *a;
    BtlObj   *obj;
    u_char   *p;
    u_long   *tim;
    u_char   *image;
    u_short   head;
    int       raw;
    int       species;
    int       col;
    int       row;
    int       slot;
    int       live;
    int       i;
    int       rec;
    int       which;
    int       fill;

    rec = 0;
    i = 0;
    g_btl_gfx_next = g_btl_enemy_gfx_start;
    do {
        g_btl_combatants[i].c.key = 0;
        rec++;
        g_btl_combatants[i].obj = 0;
        i++;
    } while (rec < BTL_ENEMY_SLOTS);

    fill = BTL_GRID_EMPTY;
    i = BTL_GRID_SQUARES;
    p = &g_btl_grid[BTL_GRID_SQUARES];
    do {
        *p = fill;
        i--;
        p--;
    } while (i >= 0);

    CdIntToPos(g_btl_enemy_gfx_offsets[set] + g_btl_enemy_gfx_base, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc,
                          g_btl_enemy_gfx_offsets[set + 1]
                              - g_btl_enemy_gfx_offsets[set],
                          (u_long *)BTL_STAGE);
    while (g_cd_busy != CD_IDLE) {
        BtlDrawFrame();
    }

    i = 0;
    live = 0;
    rec = 0;
    g_cd_busy = CD_IDLE;
    do {
        which = (set * BTL_ENEMY_SLOTS + i) * BTL_ENEMY_ROW;
        raw = g_btl_encounters[which + 1];
        if (raw != 0) {
            tim = BTL_STAGE[raw * 2 - 2];
            image = (u_char *)BTL_STAGE[raw * 2 - 1];
            head = *(u_short *)((char *)tim + 2);
            *(u_short *)((char *)tim + 2) = 0;
            species = head >> 8;
            col = g_btl_encounters[which] >> 4;
            row = g_btl_encounters[which] & 0xF;
            slot = BtlLoadEnemyGfx(species, live, tim, image,
                                   (int)BTL_STAGE[raw * 2] - (int)image);
            obj = BtlSpawnEnemy(species, col, row, slot, live);
            g_btl_combatants[rec].obj = obj;
            obj->mark_num = live + BTL_ENEMY_MARK0;
            a = &g_btl_combatants[rec];
            a->obj->actor = a;
            a->clut_len = head + 1;
            BtlLoadEnemyStats(live, species);
            obj = BtlSpawnActorObj(*(signed char *)&a->c.status,
                                   &g_btl_combatants[rec].obj->x);
            g_btl_combatants[rec].obj->mark = obj;
            g_btl_combatants[rec].obj->mark->shift_x =
                (int)*(signed char *)&g_btl_models[species].pad0A[4] << 16;
            g_btl_combatants[rec].obj->mark->shift =
                (int)*(signed char *)&g_btl_models[species].pad0A[5] << 16;
            g_btl_combatants[rec].obj->mark->attached->shift_x =
                (*(signed char *)&g_btl_models[species].pad0A[4]
                 + BTL_MARK_OFFSET) << 16;
            g_btl_combatants[rec].obj->mark->attached->shift =
                (int)*(signed char *)&g_btl_models[species].pad0A[5] << 16;

            if (species == BTL_SPECIES_SHOVE) {
                BtlPlaceMember(BTL_SHOVE_MEMBER, BTL_SHOVE_COL, 0);
            }
            if (species == BTL_SPECIES_UNSEEN) {
                g_btl_combatants[rec].c.key = 0;
                g_btl_combatants[rec].obj->attr |= BTL_OBJ_HIDDEN;
                g_btl_combatants[rec].obj->shadow->attr |= BTL_OBJ_HIDDEN;
                g_btl_combatants[rec].obj->mark->attr |= BTL_OBJ_HIDDEN;
                g_btl_combatants[rec].obj->mark->attached->attr
                    |= BTL_OBJ_HIDDEN;
            } else {
                g_btl_grid[row * BTL_GRID_WIDTH + col] = head >> 8;
            }
            *(u_short *)((char *)tim + 2) = head;
            rec++;
            live++;
        }
        i++;
    } while (i < BTL_ENEMY_SLOTS);

    DrawSync(0);
    if (g_btl_encounter - BTL_FIXED_FIRST < BTL_FIXED_COUNT) {
        BtlSpawnFixedEnemies();
    }
    BtlResetTalk(0);
}
#else
INCLUDE_ASM("btlp/nonmatchings/enemyspawn", BtlSpawnEnemies);
#endif

