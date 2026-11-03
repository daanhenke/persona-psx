/* Only the two fields read by this scan are identified. */
typedef struct {
    char pad[10];
    unsigned char active;
    char pad2[10];
    unsigned char type;
    char tail[74];
} Record;

extern int D_800B56EC;
extern int scene_tick __asm__("D_800A5314+0x350");
extern int scene_wait __asm__("D_800A5314+0x34C");

void func_80096454(void)
{
    int i;
    Record *records = (Record *)0x801F1C00;
    Record *p;

    D_800B56EC = 0;
    scene_tick = 0;
    for (i = 0, p = records; i < 5; i++, p++)
    {
        if (p->active)
        {
            if (p->type == 13)
                D_800B56EC = 1;
            if (p->type == 16)
            {
                D_800B56EC = 2;
                break;
            }
        }
    }
    if (D_800B56EC == 1 && scene_wait >= 3)
        scene_wait -= 2;
}
