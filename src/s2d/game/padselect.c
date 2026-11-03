/* Persona 1 (JP) - held, pressed, or repeated field input. */
extern int D_800B0E84[];

int func_8008E158(int which)
{
    int result = 1;

    if (which == 1) goto pressed;
    result = which < 2;
    if (result) {
        if (which == 0) goto held;
        goto done;
    }
    result = 2;
    if (which == 2) goto repeated;
    goto done;

held:
    result = D_800B0E84[8];
    goto done;
pressed:
    result = D_800B0E84[10];
    goto done;
repeated:
    result = D_800B0E84[11];
    goto done;
done:
    return result;
}
