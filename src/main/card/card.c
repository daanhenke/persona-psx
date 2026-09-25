/* Persona 1 (JP) - memory card events.  SLPS_005.00 @ 0x80012D7C
 *
 * Built at -O0 (see the Makefile). The four software card events each record
 * how the last operation on the current channel ended.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <kernel.h>
#include <sys/file.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* Which port the card code is working on, and per port how the operation in
   flight stands (CARD_*) and how the last one ended. Slot 2 is not a port:
   CardWaitSwEvent parks the channel there to catch the next event. */
#define CARD_WAIT_CHAN 2
#define CARD_PENDING   0xFF
#define CARD_IOE     0   /* done */
#define CARD_ERROR   1
#define CARD_TIMEOUT 2
#define CARD_NEW     3   /* a card was swapped in */

extern int g_card_chan;
extern volatile int g_card_state[3];
extern volatile int g_card_result[3];

/* The software events first, then the hardware ones, each in the order IOE,
   ERROR, TIMEOUT, NEW. */
extern long g_card_hw_ev[4];
extern long g_card_sw_ev[4];

/* The save file's header as the BIOS reads it: "SC", the icon's frame count,
   the file's size in blocks, then the Shift-JIS title the card manager shows
   - "Persona S-data N  LV N  N:NN:NN" - with its numbers patched in place. */
typedef struct {
    /* 0x00 */ char   magic[2];
    /* 0x02 */ u_char type;
    /* 0x03 */ u_char blocks;
    /* 0x04 */ u_char title[64];
    /* 0x44 */ u_char pad44[0x13C];
} CardHeader;                            /* 0x180 bytes */

extern CardHeader g_card_header;

/* "bu00:BISLPS-00500S0": port digit at [2], save slot digit at [18]. */
#define CARD_NAME_PORT 2
#define CARD_NAME_KIND 17
#define CARD_NAME_SLOT 18
extern char str_card_file_tmpl[];

/* The two ports' device names, "bu00:" and "bu10:". */
typedef struct {
    char *name[2];
} CardDevs;
extern CardDevs g_card_devs;

/* Save slots per kind, and the start of each save's data, which is what the
   menus list. */
#define CARD_SLOTS 7

typedef struct {
    u_char data[0x1A];
} CardSummary;

/* The save data proper starts past the header and its icon frames: the
   header's type is 0x11 + frames - 1, a frame being 0x80 bytes. */
#define CARD_DATA_OFFSET(type) (((type) - 0x11) * 0x80 + 0x100)

/* The save area as it goes to the card: the character records onwards,
   0x3780 bytes, the last of them the XOR of the rest. */
#define SAVE_AREA      ((u_char *)g_chars)
#define SAVE_AREA_SIZE 0x3780
extern u_char g_save_sum;

/* Hours, minutes, seconds and frames played, and the hero's name. Reached by
   address like the rest of the save area. */
#define g_play_time ((u_char *)0x801F29BC)
#define g_hero_name ((u_char *)0x801F29A0)

/* Where the dungeon was left (see preloaddng.c). */
extern u_short g_save_map_id;
extern u_short g_save_unk4;
extern u_char  g_save_pos_x;
extern u_char  g_save_unk5;
extern u_char  g_save_pos_y;
extern u_char  g_save_unk5_idx;
extern u_char  g_save_unk11;
extern u_char  g_save_unk12;

/* The block written after the header: what the load menus list, the rest of
   0x80 padding and the last byte the XOR of the others. CardSummary is its
   head. */
typedef struct {
    /* 0x00 */ u_char  name[8];
    /* 0x08 */ u_char  level;
    /* 0x09 */ u_char  time[4];
    /* 0x0D */ u_char  in_dungeon;    /* kind 2 saves carry the position */
    /* 0x0E */ u_short map_id;
    /* 0x10 */ u_short unk4;
    /* 0x12 */ u_char  pos_x;
    /* 0x13 */ u_char  unk5;
    /* 0x14 */ u_char  pos_y;
    /* 0x15 */ u_char  unk5_idx;
    /* 0x16 */ u_char  unk11;
    /* 0x17 */ u_char  unk12;
    /* 0x18 */ u_char  pad18[0x68];
} CardSaveBlock;                      /* 0x80 bytes */
extern CardSaveBlock g_card_save;

/* Where the title's numbers go. */
#define TITLE_KIND   0x17
#define TITLE_SLOT   0x1F
#define TITLE_LEVEL  0x26
#define TITLE_HOURS  0x2C
#define TITLE_MIN    0x33
#define TITLE_SEC    0x39

/* The retry count for every file operation below. */
#define CARD_TRIES 5

long CardEvIoe(void)
{
    g_card_result[g_card_chan] = g_card_state[g_card_chan];
    g_card_state[g_card_chan] = CARD_IOE;
}

long CardEvError(void)
{
    g_card_result[g_card_chan] = g_card_state[g_card_chan];
    g_card_state[g_card_chan] = CARD_ERROR;
}

long CardEvTimeout(void)
{
    g_card_result[g_card_chan] = g_card_state[g_card_chan];
    g_card_state[g_card_chan] = CARD_TIMEOUT;
}

long CardEvNew(void)
{
    g_card_result[g_card_chan] = g_card_state[g_card_chan];
    g_card_state[g_card_chan] = CARD_NEW;
}

void CardOpenEvents(void)
{
    EnterCriticalSection();
    g_card_sw_ev[0] = OpenEvent(SwCARD, EvSpIOE, EvMdINTR, CardEvIoe);
    g_card_sw_ev[1] = OpenEvent(SwCARD, EvSpERROR, EvMdINTR, CardEvError);
    g_card_sw_ev[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdINTR, CardEvTimeout);
    g_card_sw_ev[3] = OpenEvent(SwCARD, EvSpNEW, EvMdINTR, CardEvNew);
    g_card_hw_ev[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    g_card_hw_ev[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    g_card_hw_ev[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    g_card_hw_ev[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    ExitCriticalSection();
}

void CardCloseEvents(void)
{
    EnterCriticalSection();
    CloseEvent(g_card_sw_ev[0]);
    CloseEvent(g_card_sw_ev[1]);
    CloseEvent(g_card_sw_ev[2]);
    CloseEvent(g_card_sw_ev[3]);
    CloseEvent(g_card_hw_ev[0]);
    CloseEvent(g_card_hw_ev[1]);
    CloseEvent(g_card_hw_ev[2]);
    CloseEvent(g_card_hw_ev[3]);
    ExitCriticalSection();
}

void CardEnableEvents(void)
{
    g_card_state[0] = g_card_state[1] = CARD_TIMEOUT;
    g_card_result[0] = g_card_result[1] = CARD_TIMEOUT;
    g_card_chan = 0;
    EnableEvent(g_card_sw_ev[0]);
    EnableEvent(g_card_sw_ev[1]);
    EnableEvent(g_card_sw_ev[2]);
    EnableEvent(g_card_sw_ev[3]);
    EnableEvent(g_card_hw_ev[0]);
    EnableEvent(g_card_hw_ev[1]);
    EnableEvent(g_card_hw_ev[2]);
    EnableEvent(g_card_hw_ev[3]);
}

/* Polled once a frame. Each call looks at the port whose _card_info has
   finished, then starts one on the other port. `want0`/`want1` say per port
   what the caller cares about: bits 0-6 for a card that answered, bit 7 for
   a card that was swapped (cleared here). Returns 1 << port for the first,
   4 << port for the second. */
int CardPollPorts(int want0, int want1)
{
    int ret;
    int unused;
    int want[2];

    ret = 0;
    want[0] = want0;
    want[1] = want1;
    if (g_card_result[g_card_chan] != CARD_PENDING) {
        if (g_card_result[g_card_chan] != CARD_TIMEOUT && g_card_state[g_card_chan] == CARD_TIMEOUT) {
            ret = (want[g_card_chan] & 0x7F) ? 1 << g_card_chan : 0;
        } else if (want[g_card_chan] & 0x80) {
            if (g_card_state[g_card_chan] == CARD_NEW) {
                _card_clear(g_card_chan * 16);
                ret = 4 << g_card_chan;
            }
        }
        g_card_chan ^= 1;
        g_card_result[g_card_chan] = CARD_PENDING;
        CardClearSwEvents();
        while (_card_info(g_card_chan * 16) == 0)
            ;
    }
    return ret;
}

/* Brings port `chan` up: _card_info, then _card_load to read the card's
   directory, CARD_TRIES times. A swapped card is cleared first. Returns the
   last event (CARD_*), or 4 for a card that is new again after the load -
   one with no directory to read. */
int CardLoad(u_char chan)
{
    int ev;
    int i;

    for (i = 0; i < CARD_TRIES; i++) {
        CardClearSwEvents();
        while (_card_info(chan * 16) == 0)
            ;
        ev = CardWaitSwEvent();
        /* Can never hold; || was surely meant, which would retry here. */
        if (ev == CARD_ERROR && ev == CARD_TIMEOUT) {
            continue;
        } else if (ev == CARD_NEW) {
            CardClearHwEvents();
            _card_clear(chan * 16);
            CardWaitHwEvent();
        }
        CardClearSwEvents();
        _card_load(chan * 16);
        ev = CardWaitSwEvent();
        if (ev == CARD_NEW) {
            return 4;
        }
        if (ev == CARD_IOE) {
            break;
        }
    }
    g_card_result[chan] = g_card_state[chan];
    return g_card_state[chan] = ev;
}

/* Whether port `chan` has room for a save: creates a two-block scratch file
   (slot 9) and deletes it again. 0 if it could, -1 if not. */
int CardCheckFree(u_char chan)
{
    int  fd;
    char name[32];

    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    name[CARD_NAME_SLOT] = '9';
    fd = CardOpen(name, O_CREAT | (2 << 16));
    if (fd != -1) {
        close(fd);
        delete(name);
    }
    return fd == -1 ? -1 : 0;
}

/* 97.71%. The image initialises devs from an anonymous constant (the source
   was char *devs[2] = { "bu00:", "bu10:" }), which puts one more address
   computation in front of the copy. That constant and the two strings are
   this unit's own data at 0x80055BD0, still inside psyq/libs' asm; until the
   data is split out, the copy comes from g_card_devs instead. */
#ifdef NON_MATCHING
/* Formats the card in port `chan`; 1 once it has worked. */
int CardFormat(u_char chan)
{
    CardDevs devs = g_card_devs;
    int      ok;
    int      i;

    for (i = 0; i < CARD_TRIES; i++) {
        if ((ok = format(devs.name[chan]) == 1)) {
            break;
        }
    }
    return ok;
}
#else
INCLUDE_ASM("main/nonmatchings/card/card", CardFormat);
#endif

/* Reads the summary of every save of kind `kind` on port `chan` into `out`,
   one a slot. Returns bit i for each slot that exists, and
   bit 8 + i as well where its checksum was bad; -1 if the card failed. */
int CardScanSaves(u_char chan, u_char kind, CardSummary *out)
{
    long            fd;
    struct DIRENTRY dir;
    char            name[24];
    int             found;
    int             i;
    u_char          buf[0x80];
    int             res;

    fd = 0;
    found = 0;
    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    if (kind == 0) {
        name[CARD_NAME_KIND] = 'S';
    } else if (kind == 1) {
        name[CARD_NAME_KIND] = 'C';
    } else {
        name[CARD_NAME_KIND] = 'D';
    }
    for (i = 0; i < CARD_SLOTS; i++) {
        name[CARD_NAME_SLOT] = i + '0';
        if (firstfile(name, &dir) != 0) {
            if ((fd = CardOpen(name, O_RDONLY)) == -1) {
                return -1;
            }
            if (CardSeek(fd, CARD_DATA_OFFSET(g_card_header.type), SEEK_SET) == -1) {
                return -1;
            }
            res = CardRead(fd, buf, sizeof(buf));
            if (res == -1) {
                return -1;
            }
            if (res == -2) {
                found |= 0x100 << i;
            } else {
                close(fd);
                memcpy(out, buf, sizeof(CardSummary));
            }
            found |= 1 << i;
        }
        out++;
    }
    return found;
}

/* Whether save `slot` of kind `kind` on port `chan` is there and reads back
   with a good checksum: 1 if so, 0 if there is no such file, -1 for a card
   error and -2 for a bad sum. `found` and `i` are left over from
   CardScanSaves. */
int CardCheckSave(u_char chan, u_char kind, u_char slot)
{
    long            fd;
    struct DIRENTRY dir;
    char            name[24];
    int             found;
    int             i;
    u_char          buf[0x80];
    int             res;

    fd = 0;
    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    if (kind == 0) {
        name[CARD_NAME_KIND] = 'S';
    } else if (kind == 1) {
        name[CARD_NAME_KIND] = 'C';
    } else {
        name[CARD_NAME_KIND] = 'D';
    }
    name[CARD_NAME_SLOT] = slot + '0';
    if (firstfile(name, &dir) != 0) {
        if ((fd = CardOpen(name, O_RDONLY)) == -1) {
            return -1;
        }
        if (CardSeek(fd, CARD_DATA_OFFSET(g_card_header.type), SEEK_SET) == -1) {
            return -1;
        }
        res = CardRead(fd, buf, sizeof(buf));
        if (res == -1) {
            return -1;
        } else if (res == -2) {
            return -2;
        } else {
            close(fd);
        }
        return 1;
    }
    return 0;
}

/* Saves to slot `slot` of kind `kind` on port `chan`: the header with its
   title filled in, the summary block, then the save area. 0 or -1. */
int CardSave(u_char chan, u_char slot, u_char kind)
{
    long            fd;
    struct DIRENTRY dir;
    char            name[24];
    u_char         *p;
    int             i;
    int             unused[2];
    u_char          sum;
    int             level;
    int             member;

    fd = 0;
    sum = 0;
    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    if (kind == 0) {
        name[CARD_NAME_KIND] = 'S';
    } else if (kind == 1) {
        name[CARD_NAME_KIND] = 'C';
    } else {
        name[CARD_NAME_KIND] = 'D';
    }
    name[CARD_NAME_SLOT] = slot + '0';

    /* The fullwidth S, C or D and the slot number, trail bytes only. */
    if (kind == 0) {
        g_card_header.title[TITLE_KIND] = 0x72;
    } else if (kind == 1) {
        g_card_header.title[TITLE_KIND] = 0x62;
    } else {
        g_card_header.title[TITLE_KIND] = 0x63;
    }
    g_card_header.title[TITLE_SLOT] = slot + 0x50;

    /* The level shown is the hero's, found in the party - but read a byte
       per member past the first record rather than a record per member. */
    for (i = 0; i < 5; i++) {
        if ((member = g_party[i]) != 0xFF && g_chars[member].key == 1) {
            break;
        }
    }
    CardTitleSetNumber(level = ((Char *)((u_char *)g_chars + member))->level, TITLE_LEVEL);
    CardTitleSetNumber(g_play_time[0], TITLE_HOURS);
    g_card_header.title[TITLE_MIN] = g_play_time[1] / 10 + 0x4F;
    g_card_header.title[TITLE_MIN + 2] = g_play_time[1] % 10 + 0x4F;
    g_card_header.title[TITLE_SEC] = g_play_time[2] / 10 + 0x4F;
    g_card_header.title[TITLE_SEC + 2] = g_play_time[2] % 10 + 0x4F;

    if (firstfile(name, &dir) == 0) {
        if ((fd = CardOpen(name, O_CREAT | (2 << 16))) == -1) {
            close(fd);
            return -1;
        }
        close(fd);
    }
    if ((fd = CardOpen(name, O_WRONLY)) == -1) {
        close(fd);
        return -1;
    }
    if (CardWrite(fd, (u_char *)&g_card_header, CARD_DATA_OFFSET(g_card_header.type)) == -1) {
        close(fd);
        return -1;
    }

    memcpy(g_card_save.name, g_hero_name, 8);
    g_card_save.level = level;
    memcpy(g_card_save.time, g_play_time, 4);
    if (kind == 2) {
        g_card_save.in_dungeon = 1;
        g_card_save.map_id = g_save_map_id;
        g_card_save.unk4 = g_save_unk4;
        g_card_save.pos_x = g_save_pos_x;
        g_card_save.unk5 = g_save_unk5;
        g_card_save.pos_y = g_save_pos_y;
        g_card_save.unk5_idx = g_save_unk5_idx;
        g_card_save.unk11 = g_save_unk11;
        g_card_save.unk12 = g_save_unk12;
    } else {
        g_card_save.in_dungeon = 0;
    }
    p = (u_char *)&g_card_save;
    for (i = 0; i < sizeof(g_card_save) - 1; i++) {
        sum ^= *p++;
    }
    *p = sum;
    if (CardWrite(fd, (u_char *)&g_card_save, sizeof(g_card_save)) == -1) {
        close(fd);
        return -1;
    }

    p = SAVE_AREA;
    g_save_sum = 0;
    for (i = 0; i < (u_int)SAVE_AREA_SIZE - 1; i++) {
        g_save_sum ^= *p++;
    }
    if (CardWrite(fd, SAVE_AREA, SAVE_AREA_SIZE) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

/* Deletes save `slot` of kind `kind` (0 'S', 1 'C', otherwise 'D') from the
   card in port `chan`. 0 once it has gone, -1 if it never did. */
int CardDeleteFile(u_char chan, u_char slot, u_char kind)
{
    char name[24];
    int  i;
    int  done;

    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    if (kind == 0) {
        name[CARD_NAME_KIND] = 'S';
    } else if (kind == 1) {
        name[CARD_NAME_KIND] = 'C';
    } else {
        name[CARD_NAME_KIND] = 'D';
    }
    name[CARD_NAME_SLOT] = slot + '0';
    for (i = 0; i < CARD_TRIES; i++) {
        if ((done = delete(name)) == 1) {
            break;
        }
    }
    return done == 0 ? -1 : 0;
}

/* Loads slot `slot` of kind `kind` from port `chan`: the summary block, whose
   dungeon position is restored if it has one, then the save area. 0, -1 for
   a card error or -2 for a bad checksum. */
int CardLoadSave(u_char chan, u_char slot, u_char kind)
{
    long          fd;
    char          name[32];
    CardSaveBlock save;
    int           res;

    strcpy(name, str_card_file_tmpl);
    name[CARD_NAME_PORT] = chan + '0';
    if (kind == 0) {
        name[CARD_NAME_KIND] = 'S';
    } else if (kind == 1) {
        name[CARD_NAME_KIND] = 'C';
    } else {
        name[CARD_NAME_KIND] = 'D';
    }
    name[CARD_NAME_SLOT] = slot + '0';
    if ((fd = CardOpen(name, O_RDONLY)) == -1) {
        return -1;
    }
    if (CardSeek(fd, CARD_DATA_OFFSET(g_card_header.type), SEEK_SET) == -1) {
        return -1;
    }
    res = CardRead(fd, (u_char *)&save, sizeof(save));
    if (res == -1) {
        return -1;
    } else if (res == -2) {
        return -2;
    }
    if (save.in_dungeon == 1) {
        g_save_map_id = save.map_id;
        g_save_unk4 = save.unk4;
        g_save_pos_x = save.pos_x;
        g_save_unk5 = save.unk5;
        g_save_pos_y = save.pos_y;
        g_save_unk5_idx = save.unk5_idx;
        g_save_unk11 = save.unk11;
        g_save_unk12 = save.unk12;
    }
    bzero(SAVE_AREA, SAVE_AREA_SIZE);
    g_save_sum = 0x55;
    res = CardRead(fd, SAVE_AREA, SAVE_AREA_SIZE);
    if (res == -1) {
        return -1;
    }
    if (res == -2) {
        return -2;
    }
    close(fd);
    return 0;
}

/* Waits for the next software card event, whichever port it is for. */
int CardWaitSwEvent(void)
{
    int chan;

    chan = g_card_chan;
    g_card_chan = CARD_WAIT_CHAN;
    g_card_state[CARD_WAIT_CHAN] = CARD_PENDING;
    while (g_card_state[CARD_WAIT_CHAN] == CARD_PENDING)
        ;
    g_card_chan = chan;
    return g_card_state[CARD_WAIT_CHAN];
}

void CardClearSwEvents(void)
{
    TestEvent(g_card_sw_ev[0]);
    TestEvent(g_card_sw_ev[1]);
    TestEvent(g_card_sw_ev[2]);
    TestEvent(g_card_sw_ev[3]);
}

/* Spins until a hardware card event fires and says which (CARD_*). */
int CardWaitHwEvent(void)
{
    while (1) {
        if (TestEvent(g_card_hw_ev[0]) == 1) {
            return CARD_IOE;
        }
        if (TestEvent(g_card_hw_ev[1]) == 1) {
            return CARD_ERROR;
        }
        if (TestEvent(g_card_hw_ev[2]) == 1) {
            return CARD_TIMEOUT;
        }
        if (TestEvent(g_card_hw_ev[3]) == 1) {
            return CARD_NEW;
        }
    }
}

void CardClearHwEvents(void)
{
    TestEvent(g_card_hw_ev[0]);
    TestEvent(g_card_hw_ev[1]);
    TestEvent(g_card_hw_ev[2]);
    TestEvent(g_card_hw_ev[3]);
}

/* open(), tried CARD_TRIES times. */
int CardOpen(char *name, int mode)
{
    int i;
    int fd;

    for (i = 0; i < CARD_TRIES; i++) {
        if ((fd = open(name, mode)) != -1) {
            break;
        }
    }
    if (fd == -1) {
        return -1;
    }
    return fd;
}

/* write(), tried CARD_TRIES times; the file is closed if it never goes
   through whole. */
int CardWrite(int fd, u_char *buf, int n)
{
    int i;
    int done;

    for (i = 0; i < CARD_TRIES; i++) {
        if ((done = write(fd, buf, n)) == n) {
            break;
        }
    }
    if (done != n) {
        close(fd);
        return -1;
    }
    return fd;
}

/* read(), tried CARD_TRIES times, and then the checksum: the last byte is
   the XOR of all the others. -1 for a failed read, -2 for a bad sum; the
   file is closed either way. */
int CardRead(int fd, u_char *buf, u_int n)
{
    int     i;
    int     done;
    u_char *p;
    u_char  sum;

    sum = 0;
    for (i = 0; i < CARD_TRIES; i++) {
        if ((done = read(fd, buf, n)) == n) {
            break;
        }
    }
    if (done != n) {
        close(fd);
        return -1;
    }
    p = buf;
    for (i = 0; i < n - 1; i++) {
        sum ^= *p++;
    }
    if (*p != sum) {
        close(fd);
        return -2;
    }
    return fd;
}

/* lseek(), tried CARD_TRIES times. 0 or -1. */
int CardSeek(int fd, int offset, int whence)
{
    int i;
    int pos;

    for (i = 0; i < CARD_TRIES; i++) {
        pos = lseek(fd, offset, whence);
        if (pos != -1) {
            break;
        }
    }
    return pos == -1 ? -1 : 0;
}

/* Writes `value` (0-99) into the title at `at` as two fullwidth digits, the
   tens left blank when zero. The ones digit's lead byte is already there. */
void CardTitleSetNumber(int value, int at)
{
    if (value / 10 != 0) {
        g_card_header.title[at] = 0x82;
        g_card_header.title[at + 1] = value / 10 + 0x4F;
    } else {
        g_card_header.title[at] = 0x81;
        g_card_header.title[at + 1] = 0x40;
    }
    g_card_header.title[at + 3] = value % 10 + 0x4F;
}
