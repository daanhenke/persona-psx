/* Persona 1 (JP) - resolve per-map conditional nodes to their output bytes. */
#include <decomp/types.h>
typedef struct SceneNode {
    u_char value;
    char padding[11];
    int condition;
    struct SceneNode *next;
} SceneNode;
extern u_long *map_nodes[] __asm__("D_800AA73C+0x5C00");
extern u_char D_800B8A74[];
extern u_short g_map_id;
extern int EventFlagTest(u_short *);

void func_8009BF9C(void)
{
    int i = 0;
    u_long **maps = map_nodes;
    u_char *output = D_800B8A74;
    u_long *slot;
    u_long value;
    SceneNode *node;
next_node:
        slot = &maps[g_map_id][i];
        value = *slot;
        if (value >= 256) {
            node = (SceneNode *)value;
            if (node->condition != -1 && EventFlagTest((u_short *)&node->condition) == 1) {
                node = node->next;
            }
            *output = node->value;
        } else {
            *output = 0;
        }
        if (*slot != 255) {
            i++;
            output++;
            if (i < 256) goto next_node;
        }
}
