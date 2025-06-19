#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "sss.h"

int main(int argc, char *argv[])
{
    // checking condition
    if (argc < 4)
    {
        printf("Usage: %s <secret> <threshold> <shares>\n", argv[0]);
        return 1;
    }

    // ascii to integer
    int threshold = atoi(argv[2]);
    int shares_n = atoi(argv[3]);

    // memory for each share holder
    Share* shares = malloc(sizeof(Share) * shares_n);

    mpz_t secret, prime; // variable define
    mpz_inits(secret, prime, NULL); // initialize to 0
    mpz_set_str(secret, argv[1], 10); // secret <- argv[1] base 10

    // prime number generation of 256 bits
    generate_prime(prime, 256);
    gmp_printf("Prime used: %Zd\n", prime);

    // share the secret to parties
    deal_shares(shares, shares_n, threshold, secret, prime);
    for (int i = 0; i < shares_n; ++i)
    {
        gmp_printf("Share %d: (%Zd, %Zd)\n", i+1, shares[i].x, shares[i].y);
    }
    for (int i = 0; i < shares_n; ++i)
    {
        mpz_clear(shares[i].x);
        mpz_clear(shares[i].y);
        mpz_clear(shares[i].r);
    }
    free(shares);
    mpz_clears(secret, prime, NULL);
    return 0;
}