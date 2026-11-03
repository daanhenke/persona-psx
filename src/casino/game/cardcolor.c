/* Preserve byte-wrapped range tests and the original final comparison. */
int func_80082794(unsigned char card)
{
    int result;

    if (card < 13)
        return 1;
    if ((unsigned char)(card - 26) < 13)
        return 1;
    if ((unsigned char)(card - 13) < 13)
        return 0;
    result = (unsigned char)(card - 39) < 13;
    if (result)
        result = 0;
    return result;
}
