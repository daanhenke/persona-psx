/* Production interpreter with synthetic command streams and observed map calls. */
static union {
    unsigned int align;
    unsigned char bytes[0x14000];
} nativeMap;
#define PERSONA_TILE_MAP (nativeMap.bytes)
#include "../../src/s2d/game/actorscript.c"
extern int printf(const char *, ...);
extern void abort(void);
LookupRecord D_800B0FB0[96];
u8 D_800B1D30[2], *D_800A514C[7];
s8 D_80064FF0[2][4] = {{1, -1, 127, -128}, {-128, 127, -1, 1}};
static MapActor actor;
static u8 scripts[7][8], command[32], restartScript[8], loopScript[8];
static struct {
    int direction, x, y;
} queue[16];
static int queued, used, randomValue, randomWant, randomGot, findMode, findWant, findGot,
    packedValue, packedWant, packedGot, cases;
static unsigned long checks;
#define CHECK(c)                                                                                   \
    do {                                                                                           \
        checks++;                                                                                  \
        if (!(c)) {                                                                                \
            printf("actor script failed: line %d case %d map call %d/%d\n", __LINE__, cases, used, \
                   queued);                                                                        \
            abort();                                                                               \
        }                                                                                          \
    } while (0)
static void tileAt(int x, int y, u16 flags) {
    u8 *p = nativeMap.bytes + 0x40 + y * 400 + x * 2;
    p[0] = flags;
    p[1] = flags >> 8;
}
static void add(int direction, int x, int y, u16 flags) {
    CHECK(queued < 16);
    queue[queued].direction = direction;
    queue[queued].x = x;
    queue[queued++].y = y;
    tileAt(x, y, flags);
}
void func_80094E68(MapActor *a, s16 direction, s16 *xy) {
    CHECK(a == &actor && used < queued);
    CHECK(direction == queue[used].direction);
    xy[0] = queue[used].x;
    xy[1] = queue[used].y;
    used++;
}
int rand(void) {
    randomGot++;
    CHECK(randomGot <= randomWant);
    return randomValue;
}
int func_80094F9C(u16 x, u16 y) {
    int d = x - 40;
    findGot++;
    CHECK(findGot <= findWant && d >= 1 && d <= 3 && y == 50);
    return findMode == 0 ? -1 : d;
}
int func_80094AA4(s16 x, s16 y) {
    packedGot++;
    CHECK(packedGot <= packedWant && x == 70 && y == 71);
    return packedValue;
}
static void reset(void) {
    int i, j;
    u8 *p = (u8 *)&actor;
    for (i = 0; i < (int)sizeof(actor); i++)
        p[i] = 0;
    for (i = 0; i < 32; i++)
        command[i] = 1;
    for (i = 0; i < 8; i++)
        restartScript[i] = loopScript[i] = 1;
    for (i = 0; i < 7; i++) {
        D_800A514C[i] = scripts[i];
        for (j = 0; j < 8; j++)
            scripts[i][j] = 1;
    }
    queued = used = randomWant = randomGot = findWant = findGot = packedWant = packedGot = 0;
    randomValue = 17;
    D_800B1D30[0] = D_800B1D30[1] = 0;
    actor.script = command;
    actor.restart = restartScript;
    actor.loop = loopScript;
    actor.pos.xy[0] = 9;
    actor.pos.xy[1] = 13;
    actor.previous = 0xABCD;
    actor.colour = 83;
    actor.enabled = 1;
    *(u32 *)nativeMap.bytes = 0x40;
}
static void run(int result) {
    CHECK(func_80095700(&actor) == result);
    CHECK(used == queued && randomGot == randomWant && findGot == findWant &&
          packedGot == packedWant);
    CHECK(actor.colour == 83 && actor.enabled == 1);
    cases++;
}
static void commands(void) {
    int op, dir, j, old, n, iterations, axis, flag;
    static const int operands[] = {0, 1, 127, 255};
    static const int speeds[] = {0, 1, 3, 4, 127, 128, 254, 255};
    for (op = 0; op <= 21; op++)
        for (dir = 0; dir < 4; dir++) {
            reset();
            actor.direction = dir;
            command[0] = op;
            switch (op) {
            case 0:
                run(0);
                CHECK(actor.script == scripts[0] + 1 && actor.restart == scripts[0]);
                break;
            case 1:
                run(0);
                CHECK(actor.script == command + 1);
                break;
            case 2:
                run(0);
                CHECK(actor.script == restartScript + 1);
                break;
            case 3:
                run(0);
                CHECK(actor.script == command + 2);
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                run(0);
                CHECK(actor.speed == op - 4 && actor.script == command + 2);
                CHECK((dir & 1 ? actor.fracX : actor.fracY) == op - 4);
                break;
            case 9:
            case 10: /* Boundary matrices below cover these. */
                break;
            case 11:
            case 12:
                actor.turn = op == 11 ? 0 : 255;
                run(0);
                CHECK(actor.turn == (op == 11 ? 255 : 0) && actor.script == command + 2);
                break;
            case 13:
            case 14:
                actor.fracX = 9;
                actor.fracY = 10;
                actor.turn = 17;
                run(0);
                CHECK(actor.direction == (dir + (op == 13 ? 3 : 1)) % 4 && actor.turn == 0);
                CHECK((dir & 1 ? actor.fracX : actor.fracY) == 0);
                break;
            case 15:
                run(0);
                CHECK(actor.script == command);
                break;
            case 16:
            case 17:
                for (j = 0; j < 4; j++) {
                    reset();
                    actor.direction = dir;
                    command[0] = op;
                    command[1] = operands[j];
                    actor.fracX = (dir & 1) ? 0 : 7;
                    actor.fracY = (dir & 1) ? 7 : 0;
                    run(0);
                    CHECK((dir & 1 ? actor.fracY : actor.fracX) == (u8)(7 + operands[j]));
                    CHECK((dir & 1 ? actor.dy : actor.dx) == D_80064FF0[op - 16][dir]);
                    CHECK(actor.script == command + 3);
                }
                break;
            case 18:
                command[1] = 7;
                run(0);
                CHECK(actor.timer == 7 && actor.script == command);
                break;
            case 19:
                for (j = 0; j < 7; j++) {
                    reset();
                    actor.direction = dir;
                    command[0] = 19;
                    command[1] = j;
                    run(0);
                    CHECK(actor.restart == scripts[j] && actor.script == scripts[j] + 1);
                }
                break;
            case 20:
                for (j = 0; j < 4; j++) {
                    reset();
                    actor.direction = dir;
                    n = operands[j];
                    command[0] = 20;
                    command[1] = n;
                    command[2] = 11;
                    command[3] = 21;
                    command[4] = 1;
                    actor.turn = 79;
                    run(0);
                    iterations = n ? n : 256;
                    CHECK(actor.turn == (u8)(79 - iterations) && actor.repeat == 0 &&
                          actor.loop == command + 2 && actor.script == command + 5);
                }
                break;
            case 21:
                for (j = 0; j < 4; j++) {
                    reset();
                    actor.direction = dir;
                    command[0] = 21;
                    actor.repeat = operands[j];
                    run(0);
                    CHECK(actor.repeat == (u8)(operands[j] - 1));
                    CHECK(actor.script == (operands[j] == 1 ? command + 2 : loopScript + 1));
                }
                break;
            }
        }
    for (dir = 0; dir < 4; dir++)
        for (j = 0; j < 8; j++) {
            reset();
            actor.direction = dir;
            actor.speed = speeds[j];
            command[0] = 9;
            run(0);
            n = (u8)(speeds[j] + 1);
            if ((s8)n >= 4) {
                CHECK(actor.speed == 4 && actor.script == scripts[1] &&
                      actor.restart == scripts[1]);
            } else
                CHECK(actor.speed == n && actor.script == command + 2);
        }
    for (dir = 0; dir < 4; dir++)
        for (j = 0; j < 6; j++) {
            static const int dec[] = {0, 1, 3, 4, 129, 255};
            reset();
            actor.direction = dir;
            old = dec[j];
            actor.speed = old;
            command[0] = 10;
            run(0);
            CHECK(actor.speed == (u8)(old - 1));
            CHECK(actor.script == ((s8)(old - 1) < 0 ? scripts[0] + 1 : command + 2));
        }
    for (axis = 0; axis < 2; axis++)
        for (flag = 0; flag < 3; flag++)
            for (j = 0; j < 4; j++) {
                reset();
                actor.axis = axis;
                D_800B1D30[axis] = flag;
                command[0] = 15;
                randomValue = j;
                randomWant = flag == 1;
                run(0);
                if (flag == 1)
                    CHECK(actor.timer == j % 2 + 1 && actor.script == scripts[5] + 1 &&
                          actor.restart == scripts[5]);
                else
                    CHECK(actor.script == command);
            }
    for (j = 0; j < 5; j++) {
        static const int timers[] = {-1, 0, 1, 2, 32767};
        reset();
        command[0] = 18;
        command[1] = 5;
        actor.timer = timers[j];
        run(0);
        CHECK(actor.timer == (timers[j] ? timers[j] - 1 : 5));
        CHECK(actor.script == (timers[j] == 1 ? command + 2 : command));
    }
    reset();
    command[0] = 18;
    command[1] = 3;
    run(0);
    CHECK(actor.timer == 3);
    run(0);
    CHECK(actor.timer == 2);
    run(0);
    CHECK(actor.timer == 1);
    run(0);
    CHECK(actor.timer == 0 && actor.script == command + 2);
    run(0);
    CHECK(actor.script == command + 3);
}
static void blocked(void) {
    int state, hit, d, r;
    for (state = 1; state <= 2; state++)
        for (hit = 0; hit <= 3; hit++)
            for (r = 0; r < 5; r++) {
                reset();
                actor.blocked = state;
                actor.timer = 19;
                randomValue = r;
                for (d = 3; d >= 1; d--) {
                    add(d, 20 + d, 20, d == hit ? 0x800 : 0);
                    if (d == hit)
                        break;
                }
                if (!hit)
                    randomWant = 1;
                run(0);
                CHECK(actor.blocked == (hit ? state : 0));
                CHECK(actor.timer == (hit ? 19 : r % 5 + 2));
                CHECK(actor.script == (hit ? command + 1 : scripts[5] + 1));
            }
}
static void crossings(void) {
    static const int flags[] = {0, 0x2000, 0x4000, 0x6000, 0x3000, 0x5000, 0x7000};
    static const int packed[] = {0, 1, 15, 257};
    int mask, mode, axis, control, a, b, k, d, hit, chosen;
    for (mask = 0; mask < 8; mask++)
        for (mode = 0; mode < 3; mode++)
            for (axis = 0; axis < 2; axis++)
                for (control = 0; control < 4; control++)
                    for (a = 0; a < 7; a++)
                        for (b = 0; b < 7; b++)
                            for (k = 0; k < 4; k++) {
                                reset();
                                actor.direction = axis ? 3 : 2;
                                actor.axis = axis;
                                actor.speed = 4;
                                if (axis)
                                    actor.fracX = 12;
                                else
                                    actor.fracY = 12;
                                findMode = mode;
                                D_800B1D30[0] = control & 1;
                                D_800B1D30[1] = control >> 1;
                                packedValue = packed[k];
                                hit = 0;
                                chosen = -1;
                                add(1, 31, 41, 0);
                                for (d = 3; d >= 1; d--) {
                                    add(d, 40 + d, 50, (mask & (1 << (d - 1))) ? 0x800 : 0);
                                    D_800B0FB0[d].direction = mode == 2 ? actor.direction : 0;
                                    if (mask & (1 << (d - 1))) {
                                        findWant++;
                                        if (mode == 2) {
                                            hit = d;
                                            chosen = 2;
                                            break;
                                        }
                                    }
                                }
                                if (!hit) {
                                    add(2, 60, 61, flags[a]);
                                    if (axis == 0 && (flags[a] & 0x2000) && !(flags[a] & 0x1000) &&
                                        !(control & 1))
                                        chosen = 3;
                                    else {
                                        add(2, 62, 63, flags[b]);
                                        if (axis == 1 && (flags[b] & 0x4000) &&
                                            !(flags[b] & 0x1000) && !(control >> 1))
                                            chosen = 3;
                                        else {
                                            add(1, 70, 71, 0);
                                            packedWant = 1;
                                            if ((u8)packed[k] == 1)
                                                chosen = 6;
                                        }
                                    }
                                }
                                run(1);
                                CHECK(actor.pos.xy[0] == 31 && actor.pos.xy[1] == 41 &&
                                      actor.previous == (9 | (13 << 8)));
                                CHECK((axis ? actor.fracX : actor.fracY) == 0 &&
                                      actor.blocked == (hit != 0));
                                if (chosen < 0)
                                    CHECK(actor.script == command + 1 &&
                                          actor.restart == restartScript);
                                else
                                    CHECK(actor.script == scripts[chosen] &&
                                          actor.restart == scripts[chosen]);
                            }
    /* A still-blocked actor skips the second collision scan, but still checks signals. */
    reset();
    actor.blocked = 2;
    actor.speed = 4;
    actor.fracY = 12;
    add(3, 23, 20, 0x800);
    add(1, 31, 41, 0);
    add(2, 60, 61, 0);
    add(2, 62, 63, 0);
    add(1, 70, 71, 0);
    packedValue = 0;
    packedWant = 1;
    run(1);
    CHECK(actor.blocked == 2 && actor.script == command + 1);
}
int main(void) {
    commands();
    blocked();
    crossings();
    printf("actor script: %d calls/cases, %lu checks passed\n", cases, checks);
    return 0;
}
