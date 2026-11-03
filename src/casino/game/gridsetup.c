/* Persona 1 (JP): grid-game layouts and model groups. */
typedef struct
{
    int    pad[3];
    short* xform;
} Obj;
extern char grid_data_16d8[] __asm__("D_800A2BC8+5848");
extern char grid_data_16c8[] __asm__("D_800A2BC8+5832");
extern char grid_data_145c[] __asm__("D_800A2BC8+5212");
extern char grid_data_144c[] __asm__("D_800A2BC8+5196");
extern char grid_data_11dc[] __asm__("D_800A2BC8+4572");
extern char grid_data_11d0[] __asm__("D_800A2BC8+4560");
extern Obj* grid_data_3404 __asm__("D_800A2BC8+3404");
extern char grid_data_3388[] __asm__("D_800A2BC8+3388");
extern Obj* grid_data_3268 __asm__("D_800A2BC8+3268");
extern char grid_data_3252[] __asm__("D_800A2BC8+3252");
extern Obj* grid_data_3132 __asm__("D_800A2BC8+3132");
extern char grid_data_3116[] __asm__("D_800A2BC8+3116");
extern char grid_data_2916[] __asm__("D_800A2BC8+2916");
extern char grid_data_1208[] __asm__("D_800A2BC8+1208");
extern char grid_data_1128[] __asm__("D_800A2BC8+1128");
extern char grid_data_944[] __asm__("D_800A2BC8+944");
extern char grid_data_864[] __asm__("D_800A2BC8+864");
extern char grid_data_668[] __asm__("D_800A2BC8+668");
extern char grid_data_596[] __asm__("D_800A2BC8+596");
extern char grid_data_400[] __asm__("D_800A2BC8+400");
extern char grid_data_292[] __asm__("D_800A2BC8+292");
extern char grid_data_220[] __asm__("D_800A2BC8+220");
extern char grid_data_140[] __asm__("D_800A2BC8+140");

extern void CasinoBuildLayouts(void*, int), CasinoBuildObj(void*), CasinoTexObj(void*), CasinoSetXform(void*), CasinoDrawObj(void*);
void        func_8008BD08(void)
{
    int   count, first_offset, model_address, pair_offset;
    char *model, *object_slot, *first_slot, *second_slot;
    CasinoBuildLayouts(grid_data_140, 0);
    CasinoBuildLayouts(grid_data_220, 0);
    CasinoBuildLayouts(grid_data_292, 0);
    CasinoBuildLayouts(grid_data_400, 0);
    CasinoBuildLayouts(grid_data_596, 0);
    CasinoBuildLayouts(grid_data_668, 0);
    CasinoBuildLayouts(grid_data_864, 0);
    CasinoBuildLayouts(grid_data_944, 0);
    CasinoBuildLayouts(grid_data_1208, 0);
    CasinoBuildLayouts(grid_data_2916, 0);
    CasinoBuildLayouts(grid_data_1128, 0);
    CasinoBuildObj(grid_data_3252);
    CasinoTexObj(grid_data_3252);
    CasinoSetXform(grid_data_3268->xform);
    CasinoDrawObj(grid_data_3268);
    CasinoBuildObj(grid_data_3116);
    CasinoTexObj(grid_data_3116);
    CasinoSetXform(grid_data_3132->xform);
    CasinoDrawObj(grid_data_3132);
    CasinoBuildObj(grid_data_3388);
    CasinoTexObj(grid_data_3388);
    CasinoSetXform(grid_data_3404->xform);
    CasinoDrawObj(grid_data_3404);
    count         = 0;
    first_offset  = 0;
    model_address = (int)grid_data_11d0;
    object_slot   = (char*)(model_address + 16);
    model         = (char*)model_address;
    do
    {
        *(*(Obj**)object_slot)->xform              = 0;
        **(short**)(grid_data_11dc + first_offset) = 0;
        CasinoBuildObj(model);
        CasinoTexObj(model);
        CasinoSetXform((*(Obj**)object_slot)->xform);
        CasinoDrawObj(*(Obj**)object_slot);
        count++;
        first_offset += 20;
        model        += 20;
        object_slot  += 20;
    } while (count < 9);
    count       = 0;
    second_slot = grid_data_16d8;
    first_slot  = grid_data_145c;
    pair_offset = 0;
    do
    {
        CasinoBuildObj(grid_data_144c + pair_offset);
        CasinoTexObj(grid_data_144c + pair_offset);
        CasinoSetXform((*(Obj**)first_slot)->xform);
        CasinoDrawObj(*(Obj**)first_slot);
        CasinoBuildObj(grid_data_16c8 + pair_offset);
        CasinoTexObj(grid_data_16c8 + pair_offset);
        CasinoSetXform((*(Obj**)second_slot)->xform);
        CasinoDrawObj(*(Obj**)second_slot);
        count++;
        pair_offset += 20;
        first_slot  += 20;
        second_slot += 20;
    } while (count < 6);
}
