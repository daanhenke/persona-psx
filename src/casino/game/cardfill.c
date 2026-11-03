extern int rand(void);
extern int func_80082750(unsigned char *, unsigned char, int, int);

void func_80082658(unsigned char *cards, int count, int range, int limit)
{
    int i, value;

    for (i = 0; i < count; i++)
        cards[i] = 255;
    for (i = 0; i < count; i++) {
        do {
            value = rand() % range;
        } while (!func_80082750(cards, value, count, limit));
        cards[i] = value;
    }
}
