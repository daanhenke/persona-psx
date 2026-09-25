/* Persona 1 (JP) - the event script interpreter.  ADV only.
 *   0x800AB7E8 AdvRunScript   0x800AD348 AdvScriptSpecial
 *
 * Every event in the field - a trigger tile, a step, a talk - runs a script
 * out of the scene pack. A command is a byte of padding, an opcode byte and
 * its operands; g_cmd_len says how far each opcode moves on. Commands that
 * branch keep a pointer to their target in the word at +4 (+8 for the few
 * whose operand needs the word at +4), which the pack's loader has already
 * turned into an address.
 *
 * The script runs until END, or until a command asks to leave the room: those
 * set `leave` to say how, and the interpreter stops after that command,
 * saving where it was in g_script_resume. The caller acts on the value it
 * returns.
 *
 * A message is special: MESSAGE opens the window and then waits here, with
 * the four voice sequences open, until the window reports it is done.
 *
 *  op   operands                        does
 *  21   -                               end
 *  22   ->                              jump
 *  23   n ->                            jump unless a random byte exceeds n
 *  24   flag16                          set a story flag
 *  25   flag16                          clear it
 *  26   flag16 ->                       jump if it is set
 *  27   n                               AdvScriptSpecial(n); n >= 0xD0 leaves (5)
 *  28   map                             leave for a map (2)
 *  29   map16 x y room                  leave for a map position (3)
 *  2A   id                              play an event cutscene
 *  2B   map16 room                      leave for a room (1)
 *  2C   map x y z room                  leave for a map position (4)
 *  2D   n                               leave (6), with n
 *  2E   n ->                            jump if g_script_2B34 == n
 *  2F   key lv ->                       jump if the member's level < lv
 *  30   key ->                          jump if key is not in the party
 *  31   key                             a character joins, with their Personas
 *  32   key                             a character leaves, unequipped
 *  33   id                              put a Persona in the stock
 *  34   id ->                           jump if the stock holds it
 *  35   id                              put a Persona in the stock, closing gaps
 *  36   ->                              jump if no Persona record is free
 *  37   key n ->                        jump if the member's unk56 < n
 *  38   key exp32 ->                    jump if the member's unk1C < exp
 *  39   key                             take a Persona from everybody
 *  3A   item ->                         jump if the party has none
 *  3B   item ->                         jump if the item's index is 99
 *  3C   item16 n                        give items
 *  3D   item16 n                        take items
 *  3E   yen32 ->                        jump if the money is short of yen
 *  3F   sub yen32                       add or (sub) take money
 *  40   n ->                            jump if the moon is n
 *  41   - n                             g_script_2B32 = n
 *  42   key                             hp down by a share of it
 *  43   key                             sp down by a share of it
 *  44   key ->                          heal hp, or jump if already full
 *  45   key ->                          heal sp, or jump if already full
 *  46   key ->                          jump if status differs
 *  47   key st ->                       give an ailment, or jump if already so
 *  48   key st ->                       cure it, or jump if not so
 *  49   key - stat ->                   jump if a stat is below
 *  4A   key stat take n                 raise or lower a stat
 *  4B   -                               screen 0x24 (func_80098B8C); may leave (7)
 *  4C   -                               func_800715EC's screen, two images in
 *  4D   frames16                        wait
 *  4E   ->                              jump if the stock is full
 *  4F   key id                          a new Persona for a character
 *  50   key ->                          jump if nobody carries that Persona
 *  51   key ->                          jump if the member's list is full
 *  52   key                             drop a member from the party
 *  53   key                             add one
 *  54   n ->                            jump if the last choice was n
 *  55   - - text32                      show a message and wait for it
 *  56   n                               set a flag in the third bank
 *  57   key n                           block or free a member's Persona list
 *  58   actor script32                  point an actor at a script
 *  59   n ->                            jump if the stock holds n - 1
 *  5A   n ->                            the same after compacting it
 *  60   -                               clear the message box, put the cinema frame up
 *  61   -                               take the cinema frame down
 *  63   n                               start a room effect
 *  64   actor ...                       put an actor in the room
 *  65   actor ...                       put an actor on a tile, with a sprite
 *  66   actor                           take an actor away
 *  67   bust place                      show a bust picture
 *  68   -                               take it down
 *  69   actor se                        a sound, and its mark over an actor
 *  6C   n                               a fade, flash or view shake
 *  6D   -                               stop the view shaking
 *  6E   a b c d e                       ActorStartMove
 *  71   chan image                      start one of the scene's animations
 *  72   chan                            stop it
 *  73   actor                           draw an actor semi-transparent
 *  74   actor                           and solid again
 *  75   actor step                      fade an actor in
 *  76                                   the same
 *  77   actor step                      fade an actor out
 *  78   actor                           the view follows an actor, panning
 *  79   dir n dir n speed order         scroll the view, two legs
 *  7A   n                               clear a sound mark
 *  7B   -                               wait for the view actor, stop the jingle
 *  7C   actor x y dir n                 move an actor
 *  80   bgm                             music; a new track reloads ADVCMD
 *  81   cmd                             AdvSoundCommand
 *  87   h m ->                          jump if the clock is past h:m
 *  88   -                               stop the clock
 *  89   -                               start it
 *  8A   -                               reset it
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

/* This unit calls ImageAnimStart without a prototype. */
#define IMAGE_ANIM_START_KR

#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/adv/actor.h>
#include <persona/common/bg.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/slot.h>
#include <persona/common/imageanim.h>

/* How far each opcode moves on, indexed by the opcode itself. */
extern u_char g_cmd_len[];
/* Where a script that left the room will carry on. */
#define g_script_resume (*(u_char **)0x801F1B84)

#define OP_END   0x21
#define OP_JUMP  0x22

/* What the caller does after the script stops. */
#define LEAVE_ROOM      1
#define LEAVE_MAP       2
#define LEAVE_POS       3
#define LEAVE_POS4      4
#define LEAVE_ADVCMD    5
#define LEAVE_6         6
#define LEAVE_SCREEN    7

#define MONEY_MAX 999999999

/* The original has an empty loop inside the money clamp - most likely a debug
   report that the release build compiles away. It is not nothing to gcc: the
   loop keeps the second test from reusing the value the first one loaded. */
#define SCRIPT_DEBUG(msg) do { } while (0)

#define U16(p) (*(u_short *)(p))
#define U32(p) (*(u_long *)(p))
#define TARGET(s)  (*(u_char **)((s) + 4))
#define TARGET8(s) (*(u_char **)((s) + 8))

#define g_party    ((u_char *)0x801F256C)
#define g_clock    ((u_char *)0x801F29C0)  /* hours, min, sec, frames, on */
#define g_seq_handle ((short *)0x801F537C)
#define g_money2   (*(u_int *)0x801F2678)
#define g_money    (*(int *)0x801F2674)
#define g_adv_room (*(u_char *)0x801F5355)
#define g_script_resuming     (*(u_char *)0x801F1B88)
/* The 16 Persona slots ahead of the saved formations. */
#define g_persona_slots ((u_char *)0x801F2574)
#define g_formation_preset ((u_char *)0x801F2584)

/* The actors' sprite definitions in the scene pack, and the sound marks'. */
#define g_pack_sprites ((void **)0x80100070)
extern void  *g_se_marks[];
extern u_char g_msg_speeds[];
extern int    g_actor_dim;

extern u_short g_script_534C;
extern u_short g_map_id;
extern u_char  g_map_pos_x;
extern u_char  g_map_pos_y;
extern u_char  g_map_unk4;
extern short   g_script_15C2;
extern u_char  g_script_2B34;
extern u_char  g_moon;
extern u_char  g_script_2B32;
extern short   g_cutscene_on;
extern short   g_cutscene_alt;
extern short   g_script_97C;
extern u_char  g_msg_answer;
extern u_long  g_msg_clut[];
extern u_char  g_adv_effect;
extern short   g_cam_x;
extern short   g_cam_y;

/* The scene's animated images (sceneimages.c). */
typedef struct {
    u_long **script;
    short    x, y;
    short    w, h;
    u_short  flags;
    u_char   pad0E[2];
} AdvSceneImage;
#define g_scene_images ((AdvSceneImage *)0x80100C40)

extern CdlFILE      g_adv_scene_file;
extern volatile int g_cd_busy;

extern int    rand(void);
extern void   EventFlagSet(u_short id);
extern void   EventFlagClear(u_short id);
extern int    EventFlagGet(u_short id);
extern short  PartyFindByKey(u_char key);
extern u_char CharFindFree(void);
extern void   PartyAdd(u_char chr);
extern void   PartyCompact(void);
extern void   CharApplyStats(u_char chr);
extern u_char CharFind(u_char key);
extern u_char CharFind2(u_char key);
extern u_char PartyFindSlot(u_char chr);
extern void   CopyShorts(u_short *src, u_short *dst, u_short count);
extern void   CharUnequip(u_char chr, u_char slot);
extern void   CharRecalcStats(u_char chr);
extern void   ItemsCommitPending(void);
extern void   ItemsCompact(void);
extern void   ItemsAdd(short id, short count);
extern void   ItemsRemove(short id, short count);
extern short  ItemsFind(u_short id);
extern void   PersonaStockAdd(u_char id);
extern u_char PersonaStockFind(u_char id);
extern u_char PersonaStockFindFree(void);
extern int    PersonaStockCompact(void);
extern short  PersonaFindFree(void);
extern short  PersonaFind(u_char key);
extern u_char CharEntryFind(u_char chr, u_char v);
extern u_char CharEntryFindFree(u_char chr);
extern u_char CharStat(u_char chr, u_char stat);
extern void   CharStatAdd(u_char chr, u_char stat, u_char amount, u_char take);
extern void   FlagBank3Set(u_char id);
extern void   AdvFadeUpBlocking(short step, short limit);
extern void   AdvEffectSetupSlots(void);
extern void   ActorSetTile(short x, short y, AdvActor *a);
extern void   AdvGrowSlot(u_char place);
extern void   AdvShrinkSlot(void);
extern void   SlotFadeIn(u_char slot, u_char step);
extern void   SlotFadeOut(u_char slot, u_char step);
extern void   CamCenterOnActor(u_char actor);
extern void   AdvScrollCamera(u_char dir, char tiles, char speed);
extern void   SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void   SsSetNck(short seq);
extern void   SsSeqStop(short seq);
extern int    MsgStep(void);
extern void   AdvRunFrame(void);
extern void   AdvSelectFile();  /* called without a prototype here */
extern void   AdvLoadBgm(short id);
extern void   AdvSoundCommand(short cmd);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   ViewShakeStop(void);

/* Not worked out yet. */
extern void   AdvSceneFadeOut(void);
extern int    func_80098B8C(u_char id);
extern void   AdvQueueCmdBar(void);
extern void   AdvTynCutscene(void);
extern void   AdvRoomRebuild(void);
extern void   CharJoin(u_char chr, u_char key, u_char level);
extern void   func_800B0014(int a, u_char level, u_char chr, u_char key);
extern void   PersonaStockReturn(u_char id);
extern void   func_800715EC(void);
extern void   func_80091608(int a);
extern void   CinemaOpen(int a);
extern void   CinemaClose(int a);
extern void   ActorPlace(u_char actor);
extern void   AdvScreenEffect(u_char n);
extern void   ActorStartMove(u_char a, u_char b, u_char c, u_char d, u_char e);
extern u_char ActorsMoveStep(u_char actor);
extern short  PersonaSlotsLast(void);
extern void   PersonaFill(u_char p, u_char n);

void AdvScriptSpecial(u_char n);

#ifdef NON_MATCHING
int AdvRunScript(u_char *s)
{
    MsgState *msg = g_msg;
    RECT     clut = { 0x20, 0x1E2, 11, 1 };
    int      leave = 0;
    u_char  *save = (u_char *)0x801F2AC4;
    u_int    keep;
    int      a;
    int      b;
    int      c;
    int      d;
    short    p;
    u_int    u;
    int      r;
    short    q;
    int      f;
    u_char   e;

    if (s == (u_char *)-1) {
        goto end;
    }
    SlotClear(0x34);
    g_script_97C = 0x10;

loop:
    keep = g_money2;
    switch (s[1]) {
    case 0x23:
        if ((rand() & 0xFF) <= s[2]) {
            goto jump;
        }
        break;
    case 0x24:
        EventFlagSet(U16(s + 2));
        break;
    case 0x25:
        EventFlagClear(U16(s + 2));
        break;
    case 0x26:
        if (EventFlagGet(U16(s + 2))) {
            goto jump;
        }
        break;
    case 0x27:
        AdvScriptSpecial(s[2]);
        if (s[2] >= 0xD0) {
            g_script_resuming = 1;
            leave = LEAVE_ADVCMD;
        }
        break;
    case 0x28:
        g_script_534C = s[2];
        leave = LEAVE_MAP;
        break;
    case 0x29:
        g_map_id = U16(s + 2);
        g_map_pos_x = s[4];
        g_map_pos_y = s[5];
        g_adv_room = s[6];
        leave = LEAVE_POS;
        break;
    case 0x2A:

        g_cutscene_on = 1;
        a = s[2];
        AdvSceneFadeOut();
        func_80098B8C(a);
        AdvQueueCmdBar();
        if (g_cutscene_alt) {
            AdvTynCutscene();
            break;
        }
        AdvRoomRebuild();
        AdvFadeUpBlocking(4, 0x80);
        break;
    case 0x2B:
        g_script_534C = U16(s + 2);
        g_adv_room = s[4];
        leave = LEAVE_ROOM;
        break;
    case 0x2C:
        g_map_id = s[2];
        g_map_unk4 = s[3];
        g_map_pos_x = s[4];
        g_map_pos_y = s[5];
        g_adv_room = s[6];
        leave = LEAVE_POS4;
        break;
    case 0x2D:
        g_script_15C2 = s[2];
        leave = LEAVE_6;
        break;
    case 0x2E:
        if (g_script_2B34 == s[2]) {
            goto jump;
        }
        break;
    case 0x2F:
        a = PartyFindByKey(s[2]);
        b = g_party[a];
        r = s[3];
        u = g_chars[b].level;
    below:
        if (u < r) {
            goto jump;
        }
        break;
    case 0x30:
        p = PartyFindByKey(s[2]);
    none:
        if (p == -1) {
            goto jump;
        }
        break;
    case 0x31:
        /* A character joins, and takes up the Personas that are theirs. */
        b = 0;
        a = CharFindFree();
        PartyAdd(a);
        CharJoin(a, s[2], g_chars[0].level);
        c = 0;
        d = 0;
        PartyCompact();
        for (; b < 31; b++) {
            if (g_personas[b].key != 0 && g_personas[b].owner == s[2]) {
                g_chars[a].list[d] = b;
                d++;
                c = 1;
            }
        }
        if (c) {
            g_chars[a].entry = 0;
            func_800B0014(1, g_chars[0].level, a, s[2]);
        }
        g_chars[a].entry = 0;
        g_chars[a].unk1C = ExpToLevel(g_chars[a].unk56 - 1, 0, 0);
        CharApplyStats(a);
        CharRecalcStats(a);
        g_money2 = keep;
        break;
    case 0x32:

        /* A character leaves: their equipment goes back to the bag. */
        a = PartyFindSlot(CharFind2(s[2]));
        b = g_party[a];
        CopyShorts((u_short *)0x801F267C, (u_short *)0x800EAE4C, 0x17F);
        CharUnequip(b, 0);
        CharUnequip(b, 1);
        CharUnequip(b, 2);
        CharUnequip(b, 3);
        CharUnequip(b, 4);
        CharUnequip(b, 5);
        CharUnequip(b, 6);
        ItemsCommitPending();
        ItemsCompact();
        g_party[a] = 0xFF;
        g_chars[b].key = 0;
        g_chars[b].pad5F[0] = g_chars[b].unk56;
        PartyCompact();
        g_money2 = keep;
        break;
    case 0x33:
        PersonaStockAdd(s[2]);
        break;
    case 0x34:
        if (PersonaStockFind(s[2]) != 0xFF) {
            goto jump;
        }
        break;
    case 0x35:
        PersonaStockReturn(s[2]);
        break;
    case 0x36:
        p = PersonaFindFree();
        goto none;
    case 0x37:
        a = PartyFindByKey(s[2]);
        b = g_party[a];
        r = s[3];
        u = g_chars[b].unk56;
        goto below;
    case 0x38:

        a = PartyFindByKey(s[2]);
        b = g_party[a];
        if (g_chars[b].unk1C < U32(s + 4)) {
            s = TARGET8(s);
            goto loop;
        }
        break;
    case 0x39:
        /* Nobody carries this Persona any more. */
        a = PersonaFind(s[2]);
        if (a == -1) {
            break;
        }
        for (b = 0; b < 5; b++) {
            c = CharEntryFind(b, a);
            if (c != 0xFF) {
                g_chars[b].list[c] = 0xFF;
                if (g_chars[b].entry == c) {
                    g_chars[b].entry = 0xFF;
                }
                CharRecalcStats(PartyFindSlot(b));
            }
        }
        for (b = 0; b < 16; b++) {
            if (g_persona_slots[b] == a) {
                g_persona_slots[b] = 0xFF;
            }
        }
        g_personas[a].key = 0;
        break;
    case 0x3A:
        p = ItemsFind(s[2]);
        goto none;
    case 0x3B:
        if (ItemsFind(s[2]) == 99) {
            goto jump;
        }
        break;
    case 0x3C:
        ItemsAdd(*(short *)(s + 2), s[4]);
        g_money2 = keep;
        break;
    case 0x3D:
        ItemsRemove(*(short *)(s + 2), s[4]);
        g_money2 = keep;
        break;
    case 0x3E:
        if (U32(s + 4) > (u_int)g_money) {
            s = TARGET8(s);
            goto loop;
        }
        break;
    case 0x3F:
        if (s[2]) {
            g_money -= U32(s + 4);
        } else {
            g_money += U32(s + 4);
        }
        if (g_money > MONEY_MAX) {
            SCRIPT_DEBUG("money over limit");
            g_money = MONEY_MAX;
        }
        if (g_money < 0) {
            g_money = 0;
        }
        break;
    case 0x40:
        if (s[2] == g_moon) {
            goto jump;
        }
        break;
    case 0x41:
        g_script_2B32 = s[3];
        break;
    case 0x42:
        /* A share of the hp, s[2] / 255 of it. The share is taken off the
           record the script's own byte at the a names - a slip for
           g_party that the image carries. */
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        c = s[2] * g_chars[b].hp / 255;
        g_chars[s[a]].hp -= c;
        break;
    case 0x43:
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        c = s[2] * g_chars[b].sp / 255;
        g_chars[s[a]].sp -= c;
        break;
    case 0x44:
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        if (g_chars[b].hp == g_chars[b].hp_max) {
            goto jump;
        }
        g_chars[b].hp = g_chars[b].hp_max;
        break;
    case 0x45:
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        if (g_chars[b].sp == g_chars[b].sp_max) {
            goto jump;
        }
        g_chars[b].sp = g_chars[b].sp_max;
        break;
    case 0x46:
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        if (g_chars[b].status != s[2]) {
            goto jump;
        }
        break;
    case 0x47:
        a = PartyFindByKey(s[2]);
        b = g_party[a];
        if (g_chars[b].status == s[3]) {
            goto jump;
        }
        g_chars[b].status = s[3];
        break;
    case 0x48:
        a = PartyFindByKey(s[2]);
        if (a == -1) {
            break;
        }
        b = g_party[a];
        if (g_chars[b].status != s[3]) {
            goto jump;
        }
        g_chars[b].status = 0;
        break;
    case 0x49:

        a = PartyFindByKey(s[2]);
        if (CharStat(g_party[a], s[4]) < s[4]) {
            s = TARGET8(s);
            goto loop;
        }
        break;
    case 0x4A:
        a = PartyFindByKey(s[2]);
        CharStatAdd(g_party[a], s[3], s[5], s[4]);
        break;
    case 0x4B:
        AdvSceneFadeOut();
        a = func_80098B8C(0x24);
        AdvQueueCmdBar();
        AdvRoomRebuild();
        AdvFadeUpBlocking(4, 0x80);
        if (a) {
            leave = LEAVE_SCREEN;
        }
        break;
    case 0x4C:

        AdvSceneFadeOut();
        AdvQueueCmdBar();
        func_800715EC();
        func_80091608(1);
        AdvRoomRebuild();
        TimQueueAt((u_long *)(0x80118000 + ((u_long *)0x80118000)[1]),
                   0x380, 0x1C8, 0x100, 0x1F8);
        TimQueueAt((u_long *)(0x80118000 + ((u_long *)0x80118000)[0]),
                   0x380, 0x100, 0x3C0, 0x1A0);
        AdvFadeUpBlocking(8, 0x80);
        break;
    case 0x4D:
        for (a = 0; a < U16(s + 2); a++) {
            AdvRunFrame();
        }
        break;
    case 0x4E:
        f = PersonaStockFindFree();
    full:
        if (f == 0xFF) {
            goto jump;
        }
        break;
    case 0x4F:

        c = PersonaFindFree();
        PersonaFill(c, s[3]);
        a = CharFind2(s[2]);
        b = CharEntryFindFree(a);
        g_chars[a].list[b] = c;
        g_chars[a].entry = b;
        g_personas[c].owner = g_chars[a].key;
        CharRecalcStats(a);
        g_money2 = keep;
        break;
    case 0x50:
        p = PersonaFind(s[2]);
        goto none;
    case 0x51:
        a = PartyFindByKey(s[2]);
        f = CharEntryFindFree(g_party[a]);
        goto full;
    case 0x52:
        a = PartyFindByKey(s[2]);
        g_party[a] = 0xFF;
        PartyCompact();
        g_money2 = keep;
        break;
    case 0x53:
        PartyAdd(CharFind(s[2]));
        PartyCompact();
        g_money2 = keep;
        break;
    case 0x54:
        if (s[2] == g_msg_answer) {
            goto jump;
        }
        break;
    case 0x55:
        BgMapInit(TARGET(s), g_msg_speeds[save[8]]);
        s += g_cmd_len[s[1]];
        goto msgwait;
    case 0x56:
        FlagBank3Set(s[2]);
        break;
    case 0x57:
        a = PartyFindByKey(s[2]);
        b = g_party[a];
        g_chars[b].blocked = s[3];
        CharApplyStats(b);
        CharRecalcStats(b);
        break;
    case 0x58:
        g_adv_actors[s[2]].script = U32(s + 4);
        break;
    case 0x59:
        q = PersonaSlotsLast();
    count:
        a = q + 1;
        if (s[2] == a) {
            goto jump;
        }
        break;
    case 0x5A:
        q = PersonaStockCompact();
        goto count;
    case 0x60:
        BgMapClearRow(0);
        BgMapClearRow(1);
        BgMapClearRow(2);
        BgMapClearRow(3);
        g_bg_layers[4].scrolly = 0;
        msg->cursor = 0;
        QueueImageUpload((u_short *)&clut, g_msg_clut);
        CinemaOpen(0);
        break;
    case 0x61:
        CinemaClose(0);
        break;
    case 0x63:
        g_adv_effect = s[2];
        AdvEffectSetupSlots();
        break;
    case 0x64:
        a = s[2];
        g_adv_actors[a].kind = s[3];
        g_adv_actors[a].shadow = s[8] >> 4;
        g_adv_actors[a].x = g_adv_actors[a].home_x = s[4];
        g_adv_actors[a].y = g_adv_actors[a].home_y = s[5];
        g_adv_actors[a].dir = g_adv_actors[a].next_dir = s[6];
        g_adv_actors[a].flags = ((s[7] & 0xF) << 8) + ((s[8] & 0xF) << 7) + (s[9] << 9);
        g_adv_actors[a].phase = 0;
        g_adv_actors[a].slope = 0;
        g_adv_actors[a].lift = s[0xB];
        g_adv_actors[a].bright = s[0xA];
        g_adv_actors[a].move = MOVE_NONE;
        g_adv_actors[a].script = -1;
        ActorPlace(a);
        break;
    case 0x65:
        a = s[2];
        g_adv_actors[a].kind = s[3];
        g_adv_actors[a].x = s[4];
        g_adv_actors[a].y = s[5];
        g_adv_actors[a].flags = (s[6] << 7) + (s[7] << 9);
        g_adv_actors[a].unk23 = s[6];
        g_adv_actors[a].bright = s[8];
        g_adv_actors[a].lift = s[9];
        g_adv_actors[a].next_dir = 0;
        g_adv_actors[a].dir = 0;
        g_adv_actors[a].phase = 0;
        g_adv_actors[a].slope = 0;
        g_adv_actors[a].move = MOVE_NONE;
        ActorSetTile(g_adv_actors[a].x, g_adv_actors[a].y, &g_adv_actors[a]);
        b = g_adv_actors[a].kind;
        if ((u_int)b >= 0x80) {
            b -= 0x80;
        }
        if (g_adv_actors[a].flags & 0x80) {
            SlotInit(g_pack_sprites[b], a, g_adv_actors[a].z,
                     g_adv_actors[a].world_x, g_adv_actors[a].world_y);
        } else {
            SlotInitTagged(g_pack_sprites[b], a, g_adv_actors[a].z,
                           g_adv_actors[a].world_x, g_adv_actors[a].world_y);
        }
        SlotSetBrightness(a, g_adv_actors[a].bright -
                                 ((g_adv_actors[a].bright >> 4) << 3) * g_actor_dim);
        g_adv_actors[a].id = 0;
        break;
    case 0x66:
        g_adv_actors[s[2]].id = 0xFFFF;
        g_adv_actors[s[2]].kind = 0xFF;
        break;
    case 0x67:
        AdvRunFrame();
        AdvSelectFile(3, s[2]);
        CdReadFileToAddrAsync(&g_adv_scene_file, 5, (u_long *)0x800F4000);
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        TimQueueAt((u_long *)0x800F4008, 0x140, 0x168, 0, 0x1E6);
        AdvRunFrame();
        AdvGrowSlot(s[3]);
        break;
    case 0x68:
        AdvShrinkSlot();
        break;
    case 0x69:
        AdvSoundCommand(s[3] + 1);
        a = s[2];
        SlotInit(g_se_marks[s[3]], a + 0x20, g_adv_actors[a].z - 1,
                 g_adv_actors[a].world_x, g_adv_actors[a].world_y);
        break;
    case 0x6C:
        AdvScreenEffect(s[2]);
        break;
    case 0x6D:
        ViewShakeStop();
        break;
    case 0x6E:
        ActorStartMove(s[2], s[3], s[4], s[5], s[6]);
        break;
    case 0x71:
        u = s[3];
        ImageAnimStart(s[2], g_scene_images[u].script,
                       g_scene_images[u].x, g_scene_images[u].y,
                       g_scene_images[u].w, g_scene_images[u].h);
        break;
    case 0x72:
        ImageAnimStop(s[2]);
        break;
    case 0x73:
        g_adv_actors[s[2]].flags |= ACTOR_SEMITRANS;
        break;
    case 0x74:
        g_adv_actors[s[2]].flags &= ~ACTOR_SEMITRANS;
        break;
    case 0x75:
    case 0x76:
        SlotFadeIn(s[2], s[3]);
        g_adv_actors[s[2]].bright = 0x80;
        break;
    case 0x77:
        SlotFadeOut(s[2], s[3]);
        g_adv_actors[s[2]].bright = 0;
        break;
    case 0x78:
        /* The view pans a pixel a frame onto the new actor. */
        a = g_cam_y;
        g_cam_actor = s[2];
        b = g_cam_x;
        CamCenterOnActor(g_cam_actor);
        c = g_cam_y;
        d = g_cam_x;
        g_cam_y = a;
        g_cam_x = b;
        while (c != g_cam_y || d != g_cam_x) {
            if (c < g_cam_y) {
                g_cam_y--;
            }
            if (g_cam_y < c) {
                g_cam_y++;
            }
            if (d < g_cam_x) {
                g_cam_x--;
            }
            if (g_cam_x < d) {
                g_cam_x++;
            }
            AdvRunFrame();
        }
        break;
    case 0x79:
        if (s[7]) {
            AdvScrollCamera(s[4] + 2, s[5], s[6]);
            AdvScrollCamera(s[2], s[3], s[6]);
        } else {
            AdvScrollCamera(s[2], s[3], s[6]);
            AdvScrollCamera(s[4] + 2, s[5], s[6]);
        }
        break;
    case 0x7A:
        SlotClear(s[2] + 0x20);
        break;
    case 0x7B:
        while (ActorsMoveStep(g_cam_actor)) {
            AdvRunFrame();
        }
        SsSeqStop(g_seq_handle[10]);
        break;
    case 0x7C:
        a = s[2];
        g_adv_actors[a].x = s[3];
        g_adv_actors[a].y = s[4];
        g_adv_actors[a].dir = g_adv_actors[a].next_dir = s[5];
        g_adv_actors[a].phase = 0;
        g_adv_actors[a].slope = 0;
        g_adv_actors[a].lift = s[6];
        ActorPlace(a);
        break;
    case 0x80:
        AdvLoadBgm(s[2]);
        if (s[2] < 0x40) {
            LoadFileToAddrAsync("\\ADV\\ADVCMD.BIN;1", (void *)0x80118000);
            while (g_cd_busy != -1) {
                AdvRunFrame();
            }
        }
        break;
    case 0x81:
        AdvSoundCommand(s[2]);
        break;
    case 0x87:
        if (s[2] < g_clock[0]) {
            goto jump;
        }
        if (s[2] != g_clock[0] || s[3] >= g_clock[1]) {
            break;
        }
    case OP_JUMP:
    jump:
        s = TARGET(s);
        goto loop;
    case 0x88:
        g_clock[4] = 0;
        break;
    case 0x89:
        g_clock[4] = 1;
        break;
    case 0x8A:
        g_clock[3] = 0;
        g_clock[2] = 0;
        g_clock[1] = 0;
        g_clock[0] = 0;
        break;
    case OP_END:
        goto end;
    }
    s += g_cmd_len[s[1]];
    if (leave == 0) {
        goto loop;
    }
    g_script_resume = s;
    goto end;

msgwait:
    SoundOpenSeq(0x18, 0, 0);
    SoundOpenSeq(0x19, 0, 0);
    SoundOpenSeq(0x1A, 0, 0);
    SoundOpenSeq(0x1B, 0, 0);
    while (!(msg->flags & MSG_DONE)) {
        MsgStep();
        AdvRunFrame();
    }
    for (a = 0; a < 24; a++) {
        AdvRunFrame();
    }
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
    goto loop;

end:
    g_cam_actor = 0;
    return leave;
}
#else
INCLUDE_ASM("adv/nonmatchings/game/script", AdvRunScript);
#endif

/* The ADV command bar - two images out of ADVCMD.BIN, which stays loaded at
   0x80118000 - and the sprites that show it. */
#define ADVCMD_AT     ((u_long *)0x80118000)
#define BAR_SLOT      0x29
#define BAR_SLOT2     0x2A

#define SPECIAL_WALK_FUNC 2
#define SPECIAL_WALK_DIR  3
#define SPECIAL_KAGE      0x80   /* ..0x84: KAGE.BIN entry n - 0x80 */
#define SPECIAL_KAGE_END  0x85
#define SPECIAL_LEAVE     0xD0   /* and up: handed back through g_script_534C */

extern u_short g_adv_walk_dir;
extern u_long  g_bg_shown;
extern u_char  g_kage_sprite[];
extern u_char  g_fade_sprites;
extern u_char  g_adv_loading;

extern void AdvFadeDownBlocking(short step, short floor);
extern void FadeBlackout(void);
extern void FadeStepDown(u_char step, u_char floor);
extern void FadeStepUp(u_char step, u_char limit);
extern int  FadeSpritesStep(short step, short limit);
extern void SlotClearAll(void);
extern void AdvPickEffect(void);
extern void ImageAnimStopAll(void);
extern void func_800AD680(void);

/* The special commands behind script opcode 0x27. 0x80..0x84 put a KAGE.BIN
   picture up over the whole room, with the room faded out and everything in
   it cleared away; 0x85 brings the room back, reloads ADVCMD.BIN over the
   picture and slides the command bar back down from the top. */
#ifdef NON_MATCHING
void AdvScriptSpecial(u_char n)
{
    int y;
    int i;

    switch (n) {
    case SPECIAL_WALK_FUNC:
        func_800AD680();
        break;
    case SPECIAL_WALK_DIR:
        g_adv_walk_dir = 4;
        break;
    case 0x80 ... 0x84:
        AdvSelectFile(6, n - SPECIAL_KAGE);
        CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                              ADVCMD_AT);
        AdvFadeDownBlocking(4, 0);
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        g_bg_shown = 0;
        SlotClearAll();
        FadeBlackout();
        ImageAnimStopAll();
        g_adv_loading = 1;
        TimQueueAt((u_long *)0x80118008, 0x180, 0x100, 0, 0);
        SlotInitTagged(g_kage_sprite, BAR_SLOT, 0x100, 0, 0);
        AdvFadeUpBlocking(4, 0x80);
        break;
    case SPECIAL_KAGE_END:
        AdvFadeDownBlocking(4, 0);
        g_adv_loading = 0;
        LoadFileToAddrAsync("\\ADV\\ADVCMD.BIN;1", (void *)ADVCMD_AT);
        AdvPickEffect();
        AdvEffectSetupSlots();
        AdvRoomRebuild();
        SlotSetPos(BAR_SLOT, 0x35, 0xE0, -0x20);
        SlotSetPos(BAR_SLOT2, 0x34, 0xE3, -0x20);
        FadeStepDown(0, 0);
        g_fade_sprites = 2;
        while (!FadeSpritesStep(2, 0x80)) {
            FadeStepUp(2, 0x80);
            AdvRunFrame();
        }
        FadeStepUp(2, 0x80);
        AdvRunFrame();
        g_fade_sprites = 0;
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        TimQueueAt((u_long *)((u_char *)ADVCMD_AT + ADVCMD_AT[1]), 0x380, 0x1C8,
                   0x100, 0x1F8);
        TimQueueAt((u_long *)((u_char *)ADVCMD_AT + ADVCMD_AT[0]), 0x380, 0x100,
                   0x3C0, 0x1A0);
        AdvRunFrame();
        /* The bar slides down two pixels a frame onto y 14. */
        for (i = 0x10; i != -2; i--) {
            y = 0xC - i * 2;
            SlotSetPos(BAR_SLOT, 0x35, 0xE0, y);
            SlotSetPos(BAR_SLOT2, 0x34, 0xE3, y);
            AdvRunFrame();
        }
        AdvRunFrame();
        break;
    case 0xD0 ... 0xFF:
        g_script_534C = n;
        break;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/script", AdvScriptSpecial);
#endif

