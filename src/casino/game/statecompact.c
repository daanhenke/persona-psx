void func_8007F5F8(unsigned char *state)
{
    switch(*state) {
    case 3:*state=0;break;

    case 4:*state=1;break;

    case 5:case 6:case 7:*state=2;break;

    case 9:*state=3;break;

    case 10:*state=4;break;

    case 8:*state=5;break;

    case 11:*state=6;break;

    }
}

