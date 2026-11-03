/* Map quarter-turn dice orientation to its visible face. */
void func_800911E4(short *angle, unsigned char *face)
{
    short sector;
    if (angle[1] == 0)
    {
        sector = (angle[0] / 1024) % 4;
        switch (sector)
        {
        case 0:
            *face = 1;
            break;
        case 1:
        case -3:
            *face = 3;
            break;
        case 2:
        case -2:
            *face = 6;
            break;
        case 3:
        case -1:
            *face = 4;
            break;
        }
    }
    else if (angle[1] == 1024)
    {
        sector = (angle[0] / 1024) % 4;
        switch (sector)
        {
        case 0:
            *face = 2;
            break;
        case 1:
        case -3:
            *face = 3;
            break;
        case 2:
        case -2:
            *face = 5;
            break;
        case 3:
        case -1:
            *face = 4;
            break;
        }
    }
    else if (angle[1] == 2048)
    {
        sector = (angle[0] / 1024) % 4;
        switch (sector)
        {
        case 0:
            *face = 6;
            break;
        case 1:
        case -3:
            *face = 3;
            break;
        case 2:
        case -2:
            *face = 1;
            break;
        case 3:
        case -1:
            *face = 4;
            break;
        }
    }
    else if (angle[1] == 3072)
    {
        sector = (angle[0] / 1024) % 4;
        switch (sector)
        {
        case 0:
            *face = 5;
            break;
        case 1:
        case -3:
            *face = 3;
            break;
        case 2:
        case -2:
            *face = 2;
            break;
        case 3:
        case -1:
            *face = 4;
            break;
        }
    }
}
