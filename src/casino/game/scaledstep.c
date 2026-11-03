short D_80091E30(double *values)
{
    values[11] = 0.1 * values[20] + values[11];
    return (int)values[11] * 11;
}
