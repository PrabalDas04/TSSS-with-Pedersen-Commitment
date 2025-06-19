#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "sss.h"

int main(int argc, char *argv[])
{
    mpz_t prime, reconstructed;
    mpz_inits(prime, reconstructed, NULL);
    mpz_set_str(prime, argv[1], 10);

    // Number of shares = (argc - 2)/2
    int threshold = (argc - 2) / 2;

    Share* shares = calloc(threshold, sizeof(Share));

    for (int i = 0; i < threshold; i++)
    {
        int idx = 2 + 2 * i; // for argument index

        // Initialize
        mpz_init(shares[i].x);
        mpz_init(shares[i].y);
        mpz_init(shares[i].r); // this component will not be needed, but for sake of initialization

        // Set x
        mpz_set_str(shares[i].x, argv[idx], 10);
        // Set y
        mpz_set_str(shares[i].y, argv[idx + 1], 10);
        // r is not used for reconstruction
        mpz_set_ui(shares[i].r, 0);
    }

    reconstruct_secret(reconstructed, shares, threshold, prime);
    gmp_printf("Reconstructed Secret: %Zd\n", reconstructed);

    for (int i = 0; i < threshold; ++i) {
        mpz_clear(shares[i].x);
        mpz_clear(shares[i].y);
        mpz_clear(shares[i].r);
    }
    free(shares);
    mpz_clears(prime, reconstructed, NULL);
    return 0;
}