#include "tiger.h"
#include <string.h>

extern uint64_t sbox[4][256];

static void round(uint64_t* a, uint64_t* b, uint64_t* c, uint64_t x, int mul)
{
    *c ^= x;

    *a -= sbox[0][*c & 0xff] ^
          sbox[1][(*c >> 16) & 0xff] ^
          sbox[2][(*c >> 32) & 0xff] ^
          sbox[3][(*c >> 48) & 0xff];

    *b = mul * (*b + (
          sbox[3][(*c >> 8) & 0xff] ^
          sbox[2][(*c >> 24) & 0xff] ^
          sbox[1][(*c >> 40) & 0xff] ^
          sbox[0][(*c >> 56) & 0xff]));
}

static void tiger_compress(const uint64_t* str, int passes, uint64_t state[3])
{
    uint64_t a = state[0], b = state[1], c = state[2];
    uint64_t aa = a, bb = b, cc = c;

    uint64_t x[8];
    memcpy(x, str, 8 * sizeof(uint64_t));

    for (int pass = 0; pass < passes; ++pass)
    {
        if (pass > 0)
        {
            x[0] -= x[7] ^ 0xA5A5A5A5A5A5A5A5ull;
            x[1] ^= x[0];
            x[2] += x[1];

            x[3] -= x[2] ^ (~x[1] << 19);
            x[4] ^= x[3];
            x[5] += x[4];

            x[6] -= x[5] ^ (~x[4] >> 23);
            x[7] ^= x[6];
            x[0] += x[7];

            x[1] -= x[0] ^ (~x[7] << 19);
            x[2] ^= x[1];
            x[3] += x[2];

            x[4] -= x[3] ^ (~x[2] >> 23);
            x[5] ^= x[4];
            x[6] += x[5];

            x[7] -= x[6] ^ 0x0123456789ABCDEFull;
        }

        int mul = pass == 0 ? 5 : pass == 1 ? 7 : 9;

        round(&a, &b, &c, x[0], mul);
        round(&b, &c, &a, x[1], mul);
        round(&c, &a, &b, x[2], mul);

        round(&a, &b, &c, x[3], mul);
        round(&b, &c, &a, x[4], mul);
        round(&c, &a, &b, x[5], mul);

        round(&a, &b, &c, x[6], mul);
        round(&b, &c, &a, x[7], mul);

        uint64_t tmpa = a;
        a = c;
        c = b;
        b = tmpa;
    }

    state[0] = a ^ aa;
    state[1] = b - bb;
    state[2] = c + cc;
}

void tiger(const uint64_t* str, size_t length, int passes, uint64_t res[3])
{
    res[0] = 0x0123456789ABCDEFull;
    res[1] = 0xFEDCBA9876543210ull;
    res[2] = 0xF096A5B4C3B2E187ull;

    size_t i, j;

    for (i = length; i >= 64; i -= 64)
    {
        tiger_compress(str, passes, res);
        str += 8;
    }

    uint64_t expanded[8];

    memcpy(expanded, str, i * sizeof(unsigned char));
    j = i;

    ((unsigned char*)expanded)[j++] = 0x01;

    if (j > 56)
    {
        memset((unsigned char*)expanded + j, 0, 64 - j);
        tiger_compress(expanded, passes, res);
        j = 0;
    }

    memset((unsigned char*)expanded + j, 0, 56 - j);

    expanded[7] = length << 3;
    tiger_compress(expanded, passes, res);
}
