extern short g_s2d_selected_event_flag;
extern unsigned short func_8008E158(int);

void func_8008BBD8(void)
{
    int step, up, down;

    if (func_8008E158(0) & 0x20)
        step = 5;
    else
        step = 1;
    if (func_8008E158(2) & 0x2000) {
        up = g_s2d_selected_event_flag + step;
        if (up > 512)
            up = 512;
        g_s2d_selected_event_flag = up;
    }
    if (func_8008E158(2) & 0x8000) {
        down = g_s2d_selected_event_flag - step;
        if (down < 0)
            down = 0;
        g_s2d_selected_event_flag = down;
    }
    if (func_8008E158(1) & 0x80) {
        short n = g_s2d_selected_event_flag;
        unsigned char *flag = (unsigned char *)0x801F29C8 + (n >> 3);
        *flag ^= 1 << (n & 7);
    }
}
