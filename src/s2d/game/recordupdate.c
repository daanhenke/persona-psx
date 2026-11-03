extern short D_800B5E28[];

/* Three 40-byte records: mode at 0, enable at 2, value at 20,
   copied value at 24, and increment at 36. */
void func_80099A00(void)
{
    int i;
    short *record = D_800B5E28;
    short *fields;

    for (i = 0; i < 3; record += 20, i++) {
        fields = record + 1;
        switch (*record) {
        case 0:
            break;
        case 1:
            *(int *)(fields + 9) += *(int *)(fields + 17);
            if (*(int *)(fields + 9) > 4096)
                *(int *)(fields + 9) = 4096;
            *(int *)(fields + 11) = *(int *)(fields + 9);
            if (*(int *)(fields + 9) == 4096)
                *record = 0;
            break;
        case 2:
            *(int *)(fields + 9) -= *(int *)(fields + 17);
            if (*(int *)(fields + 9) < 0)
                *(int *)(fields + 9) = 0;
            *(int *)(fields + 11) = *(int *)(fields + 9);
            if (!*(int *)(fields + 9)) {
                *record = 0;
                *fields = 0;
            }
            break;
        }
    }
}
