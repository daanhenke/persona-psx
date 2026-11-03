/* Resolve contact and boundary responses for the two preview objects. */
#include <decomp/types.h>
#include <libsnd.h>
typedef struct
{
    unsigned char previous_transform_and_vectors[112];
    double        value98, valueA0, valueA8, velocity_x, velocity_y;
} PreviewMotionTail;
extern PreviewMotionTail D_800B0748, D_800B0838;
extern int               D_800B0724[], D_800B0814[];
extern unsigned char     D_800B0800[], D_800B08F0[];
extern short             g_casino_seqs[];
extern void              func_80092394(void*);
extern double            first_velocity_x __asm__("D_800B0748+136");
extern double            second_velocity_x __asm__("D_800B0838+136");
extern double            second_velocity_y __asm__("D_800B0838+144");
extern signed char       first_collision_latch __asm__("D_800B0800+7");
extern unsigned char     first_contact __asm__("D_800B0800+1");
extern unsigned char     second_collision_latch __asm__("D_800B08F0+7");
extern double            first_value98 __asm__("D_800B0748+112"), second_value98 __asm__("D_800B0838+112");
void                     func_80091EB8(void)
{
    int    dx, dy;
    double temp[2];
    dx = D_800B0724[1] - D_800B0814[1];
    dy = D_800B0724[2] - D_800B0814[2];
    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;
    if (dx < 25 && dy < 25)
    {
        if (dx < 24 && first_collision_latch == 0)
        {
            SsSeqStop(g_casino_seqs[8]);
            SsSeqPlay(g_casino_seqs[8], 1, 1);
            temp[0]           = first_velocity_x;
            temp[1]           = second_velocity_x;
            second_velocity_x = 1.2 * temp[0];
            first_velocity_x  = 1.2 * temp[1];
            first_collision_latch++;
            second_collision_latch++;
        }
        if (dy < 24 && dx < 21)
        {
            SsSeqStop(g_casino_seqs[8]);
            SsSeqPlay(g_casino_seqs[8], 1, 1);
            temp[0]               = D_800B0748.velocity_y;
            temp[1]               = second_velocity_y;
            second_velocity_y     = temp[0];
            D_800B0748.velocity_y = temp[1];
            first_value98         = -first_value98;
            second_value98        = -second_value98;
            if (first_contact)
                D_800B08F0[1] = 1;
            else if (D_800B08F0[1])
                first_contact = 1;
            if (D_800B0800[8])
                D_800B08F0[8] = 1;
            else if (D_800B08F0[8])
                D_800B0800[8] = 1;
        }
    }
    if (D_800B0724[2] >= 130)
        func_80092394((char*)D_800B0724 - 4);
    if (D_800B0814[2] >= 130)
        func_80092394((char*)D_800B0724 + 236);
    if ((unsigned)(D_800B0724[1] + 63) >= 127 && (signed char)D_800B0800[0] > 0)
    {
        SsSeqStop(g_casino_seqs[10]);
        SsSeqPlay(g_casino_seqs[10], 1, 1);
        first_velocity_x = -1.0 * first_velocity_x;
        first_velocity_x = 0.13 * first_velocity_x;
        first_value98   += first_velocity_x;
    }
    if ((unsigned)(D_800B0814[1] + 63) >= 127 && (signed char)D_800B08F0[0] > 0)
    {
        SsSeqStop(g_casino_seqs[10]);
        SsSeqPlay(g_casino_seqs[10], 1, 1);
        second_velocity_x = -1.0 * second_velocity_x;
        second_velocity_x = 0.13 * second_velocity_x;
        second_value98   += second_velocity_x;
    }
}
