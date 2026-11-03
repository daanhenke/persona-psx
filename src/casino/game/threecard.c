unsigned char func_80081F80(unsigned char *cards)
{
    unsigned char first, second, third;

    first = (unsigned int)cards[0] % 13;
    if (first == 6) {
        second = (unsigned int)cards[1] % 13;
        if (second == first) {
            third = (unsigned int)cards[2] % 13;
            if (third == second)
                return 3;
        }
    }
    return 255;
}
