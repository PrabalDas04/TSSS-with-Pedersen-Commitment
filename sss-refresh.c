#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "sss.h"

int main(int argc, char *argv[])
{
    // checking condition
    if (argc < 6)
    {
        printf("Usage: %s <prime> <threshold> <shares_n> <g> <h>\n", argv[0]);
        return 1;
    }
    
    mpz_t prime, g, h; // variable define

    // Initialize prime, g and h
    mpz_inits(g, h, prime, NULL); // initialize to 0

    // ascii to integer
    mpz_set_str(prime, argv[1], 10);
    mpz_set_str(g, argv[4], 10);
    mpz_set_str(h, argv[5], 10);
    int threshold = atoi(argv[2]);
    int shares_n = atoi(argv[3]);
    

    // memory for each share holder
    Share* shares = malloc(sizeof(Share) * shares_n);
    share_update(shares, shares_n, threshold, prime, g, h);

    for (int i = 0; i < shares_n; ++i)
    {
        gmp_printf("Share %d: (%Zd, %Zd)\n", i+1, shares[i].x, shares[i].y);
    }

    for (int i = 0; i < threshold; ++i)
    {
        mpz_clear(shares[i].x);
        mpz_clear(shares[i].y);
        mpz_clear(shares[i].r);
    }
    free(shares);
    mpz_clears(prime, g, h, NULL);
    
    return 0;
}