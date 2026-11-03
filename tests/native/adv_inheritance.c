#include <persona/common/persona.h>
#include <persona/common/spell.h>
extern int puts(const char *);
extern short FuseInheritSpell(short, short, short *, short *);
PersonaData g_persona_data[2];
const PersonaDef g_persona_defs[33] = {
    [2] = { .pad27 = {12} },
    [3] = { .raw = {1} },
    [4] = { .raw = {2} },
    [5] = { .raw = {3} },
    [6] = { .raw = {4} },
    [7] = { .raw = {5} },
    [8] = { .raw = {6} },
    [9] = { .raw = {7} },
    [10] = { .raw = {8} },
    [11] = { .raw = {9} },
    [12] = { .raw = {10} },
    [13] = { .raw = {11} },
    [14] = { .raw = {12} },
    [15] = { .raw = {13} },
    [16] = { .raw = {14} },
    [17] = { .raw = {15} },
    [18] = { .raw = {16} },
    [19] = { .raw = {17} },
    [20] = { .raw = {18} },
    [21] = { .raw = {19} },
    [22] = { .raw = {20} },
    [23] = { .raw = {21} },
    [24] = { .raw = {22} },
    [25] = { .raw = {23} },
    [26] = { .raw = {24} },
    [27] = { .raw = {25} },
    [28] = { .raw = {26} },
    [29] = { .raw = {27} },
    [30] = { .raw = {28} },
    [31] = { .raw = {29} },
    [32] = { .raw = {30} },
};
SpellData g_spell_data[256];
u_char g_arcana_fuse_group[3], g_fuse_group_classes[30];
u_char g_fuse_spell_runs[20], g_fuse_spell_list[30];

static void reset(void)
{
    int i;
    for (i = 0; i < 10; i++) {
        g_fuse_group_classes[i] = i;
        g_fuse_group_classes[10 + i] = 9 - i;
        g_fuse_group_classes[20 + i] = (i + 3) % 10;
        g_fuse_spell_runs[i * 2] = i * 3;
        g_fuse_spell_runs[i * 2 + 1] = 3;
    }
    for (i = 0; i < 30; i++) g_fuse_spell_list[i] = i + 1;
    for (i = 0; i < 3; i++) g_arcana_fuse_group[i] = i + 1;
    for (i = 0; i < 6; i++) g_persona_data[1].spell[i] = 255;
    for (i = 0; i < 256; i++) g_spell_data[i].kind = 0;
    g_persona_data[1].arcana = 1;
}

int main(void)
{
    int group, target, slot, mode, position, tests = 0;
    short spell, rank, result;
    /* Three distinct class orders, every entry, and each searchable slot. */
    for (group = 0; group < 3; group++) for (target = 0; target < 30; target++)
    for (slot = 0; slot < 5; slot++) for (mode = 0; mode < 4; mode++) {
        reset();
        g_persona_data[1].arcana = group + 1;
        g_persona_data[1].spell[slot] = target + 1;
        if (mode == 3) g_spell_data[target + 1].kind = 8;
        position = group == 0 ? target / 3 : group == 1 ? 9 - target / 3 : (target / 3 + 7) % 10;
        spell = rank = -99;
        result = FuseInheritSpell(mode == 1 ? target + 3 : mode >= 2 ? 2 : 1, 1, &spell, &rank);
        if (mode == 1 || mode == 2) {
            if (result || spell != 255 || rank != 255) return 1;
        } else if (result != 1 || spell != target + 1 || rank != position * 3 + target % 3) return 2;
        tests++;
    }
    /* The sixth data slot is deliberately not searched. */
    reset(); g_persona_data[1].spell[5] = 1;
    if (FuseInheritSpell(1, 1, &spell, &rank) || spell != 255 || rank != 255) return 3;
    tests++;
    /* The first class row controls the sentinel, even for another group. */
    reset(); g_persona_data[1].arcana = 2; g_persona_data[1].spell[0] = 28;
    g_fuse_group_classes[0] = 255;
    if (FuseInheritSpell(1, 1, &spell, &rank) || spell != 255 || rank != 255) return 4;
    tests++;
    /* An empty run adds no ranks; prefer the earliest eligible spell. */
    reset(); g_fuse_spell_runs[1] = 0;
    g_persona_data[1].spell[0] = 6; g_persona_data[1].spell[1] = 4;
    if (FuseInheritSpell(1, 1, &spell, &rank) != 1 || spell != 4 || rank != 0) return 5;
    tests++;
    if (tests != 1803) return 6;
    puts("1803 native inheritance fixtures passed");
    return 0;
}
