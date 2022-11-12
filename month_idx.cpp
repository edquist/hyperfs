
/*  month_idx - simple hash, unique for short month names
 *
 *  {19,  0, "jan"}
 *  { 7,  1, "feb"}
 *  { 5,  2, "mar"}
 *  { 8,  3, "apr"}
 *  {12,  4, "may"}
 *  {18,  5, "jun"}
 *  {16,  6, "jul"}
 *  { 2,  7, "aug"}
 *  {13,  8, "sep"}
 *  {11,  9, "oct"}
 *  { 3, 10, "nov"}
 *  { 6, 11, "dec"}
 */

int month_idx(const char *short_month_name)
{
    static const int idx[21] = {-1, -1,  7, 10, -1, 2, 11,
                                 1,  3, -1, -1,  9, 4,  8,
                                -1, -1,  6, -1,  5, 0, -1};

    int key = (short_month_name[0] | ' ')
            + (short_month_name[1] | ' ')
            + (short_month_name[2] | ' ');

    return idx[key % 21];
}

