/* Expand the 21 source entries into twelve five-entry reel lines.
 * Unused line entries use the signed -1 sentinel. */
extern void func_800876FC(signed char (*)[5], unsigned char), func_80087964(void);
void func_8008749C(unsigned char *input, unsigned char mode)
{
    signed char rows[12][5];
    rows[0][0] = input[9];
    rows[1][0] = input[12];
    rows[0][1] = input[10];
    rows[1][1] = input[11];
    rows[0][2] = rows[1][2] = input[2];
    rows[0][3] = input[13];
    rows[1][3] = input[16];
    rows[0][4] = input[14];
    rows[1][4] = input[15];
    rows[2][0] = input[5];
    rows[3][0] = input[6];
    rows[2][1] = input[8];
    rows[3][1] = input[7];
    rows[3][2] = rows[2][2] = input[2];
    rows[2][3] = input[17];
    rows[3][3] = input[18];
    rows[2][4] = input[20];
    rows[3][4] = input[19];
    rows[4][0] = rows[5][0] = input[0];
    rows[4][1] = input[5];
    rows[5][1] = input[8];
    rows[4][2] = input[6];
    rows[5][2] = input[7];
    rows[4][3] = rows[5][3] = input[1];
    rows[4][4] = rows[5][4] = -1;
    rows[6][0] = rows[7][0] = input[3];
    rows[6][1] = input[17];
    rows[7][1] = input[20];
    rows[6][2] = input[18];
    rows[7][2] = input[19];
    rows[6][3] = rows[7][3] = input[4];
    rows[6][4] = rows[7][4] = -1;
    rows[8][0] = rows[9][0] = input[0];
    rows[8][1] = input[9];
    rows[9][1] = input[10];
    rows[8][2] = input[12];
    rows[9][2] = input[11];
    rows[8][3] = rows[9][3] = input[3];
    rows[8][4] = rows[9][4] = -1;
    rows[10][0] = rows[11][0] = input[1];
    rows[10][1] = input[13];
    rows[11][1] = input[14];
    rows[10][2] = input[16];
    rows[11][2] = input[15];
    rows[10][3] = rows[11][3] = input[4];
    rows[10][4] = rows[11][4] = -1;
    func_800876FC(rows, mode);
    func_80087964();
}
