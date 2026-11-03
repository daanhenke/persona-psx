unsigned char func_80082454(unsigned char card)
{
    switch(card) {
    case 0:return 11;
    case 10:case 11:case 12:case 23:case 24:case 25:case 36:case 37:case 38:case 49:case 50:case 51:return 10;
    case 52:return 12;
    default:return (unsigned int)card%13;
    }
}

