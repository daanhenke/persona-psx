typedef struct Node
{
    short a;
    union {
        short flags;
        struct
        {
            unsigned char kind, bits;
        } b;
    } f;
    union {
        int word;
        unsigned short half;
    } data;
    unsigned char v[4];
    int condition;
    struct Node *next;
} Node;
extern Node **nodes[] __asm__("D_800AA73C+0x5C00");
extern int previous __asm__("D_800AA73C+0x5C24");
extern unsigned short g_map_id;
extern unsigned char g_map_pos_x, g_map_pos_y;
extern short g_s2d_exit, g_script_15C2;
extern int D_800B5F3C, D_800A4CEC, D_800A4CE4, D_800B9188, g_state_next;
extern int EventFlagTest(void *);
extern short func_80094594(void);
extern void func_8009BC4C(void), SoundCheckMarkMap(int), S2dMsgOpen(int), SoundPlayBgm(int),
    func_80093DF8(void), S2dEventFlagSet(int);
int func_8009BCBC(int index, int restrict)
{
    Node *n = nodes[g_map_id][index];
    if (n->condition != -1 && EventFlagTest(&n->condition) == 1)
        n = n->next;
    if (restrict && n->f.b.kind != 3 && n->f.b.kind != 6)
        return 1;
    if ((n->f.flags >> (func_80094594() + 8)) & 1)
        return 1;
    switch (n->f.flags & 255)
    {
    case 0:
        g_map_id = n->data.half;
        *(unsigned char *)0x801f5355 = n->v[1];
        g_map_pos_x = n->v[2];
        g_map_pos_y = n->v[3];
        g_s2d_exit = 0;
        D_800B5F3C = 0;
        return 1;
    case 1:
        func_8009BC4C();
        g_map_id = n->data.half;
        *(unsigned char *)0x801f5354 = n->v[0];
        *(unsigned char *)0x801f5355 = n->v[1];
        g_map_pos_x = n->v[2];
        g_map_pos_y = n->v[3];
        g_s2d_exit = 1;
        D_800B5F3C = 0;
        return 1;
    case 2:
        func_8009BC4C();
        SoundCheckMarkMap(n->data.word);
        g_map_id = n->data.half;
        *(unsigned char *)0x801f5355 = n->v[1];
        g_s2d_exit = 3;
        D_800B5F3C = 0;
        return 1;
    case 3:
        if (n->data.word != D_800A4CEC)
            S2dMsgOpen(n->data.word);
        if (previous != index)
        {
            if (!g_map_id)
            {
                int track = (signed char)n->v[0];
                if (track == -1)
                    track = 0;
                SoundPlayBgm(track);
            }
            if ((signed char)n->v[1] != -1)
                D_800A4CE4 = (signed char)n->v[1] + 128;
            previous = index;
        }
        return 0;
    case 4:
        func_8009BC4C();
        D_800B9188 = n->data.word;
        func_80093DF8();
        return 1;
    case 5:
        S2dEventFlagSet(372);
        func_8009BC4C();
        g_script_15C2 = 26;
        g_s2d_exit = 4;
        D_800B5F3C = 0;
        g_state_next = 6;
        return 1;
    case 6:
        return 0;
    }
    return 0;
}
