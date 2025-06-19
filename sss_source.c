#include "sss.h"
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/rand.h>

#define SEED_SIZE 32 // 32 bytes = 256bits of randomness

// function for generating state for random number generation
void seed_gmp_randstate(gmp_randstate_t state)
{
    unsigned char seed[SEED_SIZE];
    if (RAND_bytes(seed, SEED_SIZE) != 1) {
        fprintf(stderr, "seed_gmp_randstate: RAND_bytes failed\n");
        exit(1);
    }

    // Import seed into GMP
    mpz_t seed_int;
    mpz_init(seed_int);
    mpz_import(seed_int, SEED_SIZE, 1, 1, 0, 0, seed);

    // Seed GMP’s pseudorandom number generator
    gmp_randseed(state, seed_int);

    // Clear
    mpz_clear(seed_int);
}

void generate_prime(mpz_t prime, int bits)
{
    // initialize the state variable
    gmp_randstate_t state;
    gmp_randinit_default(state);

    seed_gmp_randstate(state);

    // Generate a random number with specified number of bits
    mpz_urandomb(prime, state, bits);

    // Find the subsequent prime
    mpz_nextprime(prime, prime);

    // Cleanup
    gmp_randclear(state);
    //mpz_clear(seed_int);
}

// function for generating (t-1)-degree polynomial
void generate_polynomial(mpz_t* coeffs, int threshold, mpz_t secret, mpz_t prime)
{
    // random state generation
    gmp_randstate_t state;
    gmp_randinit_default(state);
    seed_gmp_randstate(state);

    // constant term to be the secret value
    mpz_init_set(coeffs[0], secret);

    for (int i = 1; i < threshold; ++i)
    {
        mpz_init(coeffs[i]);
        mpz_urandomm(coeffs[i], state, prime);
    }
    gmp_randclear(state);
}

void evaluate_polynomial(mpz_t result, mpz_t* coeffs, int threshold, mpz_t x, mpz_t prime)
{
    mpz_t term, x_pow;
    mpz_init_set_ui(result, 0);
    mpz_init_set_ui(x_pow, 1);
    mpz_init(term);

    // polynomial evaluation
    for (int i = 0; i < threshold; ++i)
    {
        mpz_mul(term, coeffs[i], x_pow);
        mpz_add(result, result, term);
        mpz_mod(result, result, prime);
        mpz_mul(x_pow, x_pow, x);
        mpz_mod(x_pow, x_pow, prime);
    }

    mpz_clear(term);
    mpz_clear(x_pow);
}

void deal_shares(Share* shares, int n, int t, mpz_t secret, mpz_t prime)
{
    // memory for coefficients and binding poly
    mpz_t* coeffs = malloc(sizeof(mpz_t) * t);
    mpz_t* r_vals = malloc(sizeof(mpz_t) * t);
    mpz_t g, h;

    // Initialize g and h
    mpz_init(g);
    mpz_init(h);

    // state for random number generation
    gmp_randstate_t state;
    gmp_randinit_default(state);
    seed_gmp_randstate(state);

    // First generate g (a generator or group element)
    do {
        mpz_urandomb(g, state, 256);
    } while (mpz_cmp_ui(g, 0) == 0);
    mpz_mod(g, g, prime);

    // Now generate a random secret a
    mpz_t a;
    mpz_init(a);
    mpz_urandomb(a, state, 256);
    mpz_mod(a, a, prime);

    // Now compute h = g^a % p
    mpz_powm(h, g, a, prime);

    gmp_printf("g, h: (%Zd,\n %Zd)\n", g, h);
    generate_polynomial(coeffs, t, secret, prime);

    // Generate Pedersen commitments
    mpz_t* commitments = malloc(t * sizeof(mpz_t));
    generate_pedersen_commitments(commitments, coeffs, r_vals, t, g, h, prime);

    // share generation of n parties
    for (int i = 0; i < n; ++i)
    {
        mpz_init_set_ui(shares[i].x, i + 1);
        mpz_init(shares[i].y);
        mpz_init(shares[i].r);
        // evaluate f(i) and store in shares[i].y
        evaluate_polynomial(shares[i].y, coeffs, t, shares[i].x, prime);
        // evaluate r(i) and store in shares[i].r
        evaluate_polynomial(shares[i].r, r_vals, t, shares[i].x, prime);
    }

    // verify share from commitments
    //verify_share(shares, n, t, commitments, g, h, prime);

    // Cleanup
    for (int i = 0; i < t; ++i)
    {
        mpz_clear(coeffs[i]);
        mpz_clear(r_vals[i]);
        mpz_clear(commitments[i]);
    }
    free(r_vals);
    free(commitments);
    free(coeffs);
    gmp_randclear(state);
    mpz_clear(g);
    mpz_clear(h);
}

// function for reconstruction of secret
void reconstruct_secret(mpz_t secret, Share* shares, int t, mpz_t prime)
{
    mpz_t num, den, tmp, inv, term;
    mpz_init_set_ui(secret, 0);
    mpz_inits(num, den, tmp, inv, term, NULL);

    for (int i = 0; i < t; ++i)
    {
        mpz_set_ui(num, 1);
        mpz_set_ui(den, 1);
        for (int j = 0; j < t; ++j)
        {
            if (i != j)
            {
                mpz_sub(tmp, shares[j].x, shares[i].x);
                mpz_mod(tmp, tmp, prime);
                mpz_mul(den, den, tmp);
                mpz_mod(den, den, prime);

                mpz_mul(num, num, shares[j].x);
                mpz_mod(num, num, prime);
            }
        }
        mpz_invert(inv, den, prime);
        mpz_mul(term, shares[i].y, num);
        mpz_mul(term, term, inv);
        mpz_add(secret, secret, term);
        mpz_mod(secret, secret, prime);
    }

    mpz_clears(num, den, tmp, inv, term, NULL);
}

// pedersen commitment generation for each share
void generate_pedersen_commitments(mpz_t *commitments, mpz_t *coeffs, mpz_t *r_vals, int threshold, mpz_t g, mpz_t h, mpz_t p)
{
    // random state generation
    gmp_randstate_t state;
    gmp_randinit_default(state);
    seed_gmp_randstate(state);

    for (int i = 0; i < threshold; ++i)
    {
        mpz_init(r_vals[i]);

        // Generate random binding r_i
        mpz_urandomb(r_vals[i], state, 256);
        mpz_mod(r_vals[i], r_vals[i], p);

        // Commitment C_i = g^a_i * h^r_i (mod p)
        mpz_init(commitments[i]);

        mpz_t g_pow, h_pow, temp;
        mpz_init(g_pow);
        mpz_init(h_pow);
        mpz_init(temp);

        mpz_powm(g_pow, g, coeffs[i], p); // g_pow <- g^coeff[i] mod p
        mpz_powm(h_pow, h, r_vals[i], p); // h_pow <- h^r_vals[i] mod p
        mpz_mul(temp, g_pow, h_pow); // temp <- g_pow * h_pow
        mpz_mod(temp, temp, p); // temp <- temp mod p
        mpz_set(commitments[i], temp); // C_i <- temp
        //gmp_printf("C[%d]: %Zd", i, commitments[i]);

        mpz_clear(g_pow);
        mpz_clear(h_pow);
        mpz_clear(temp);
    }

    gmp_randclear(state);
}

void verify_share(Share *shares, int n, int threshold, mpz_t *commitments, mpz_t g, mpz_t h, mpz_t p)
{
    for(int j = 0; j < n; j++)
    {
        // Left side: g^f(j+1) * h^r(j+1)
        mpz_t lhs, g_pow, h_pow;
        mpz_init(lhs);
        mpz_init(g_pow);
        mpz_init(h_pow);

        mpz_powm(g_pow, g, shares[j].y, p); // g_pow <- g^shares[j].y mod p
        mpz_powm(h_pow, h, shares[j].r, p); // h_pow <- h^shares[j].r mod p
        mpz_mul(lhs, g_pow, h_pow); // lhs <- g_pow * h_pow
        mpz_mod(lhs, lhs, p); // lhs <- lhs mod p

        // Right side: Product C_i^(j^i)
        mpz_t rhs, temp, exp;
        mpz_init(rhs);
        mpz_set_ui(rhs, 1);
        mpz_init(temp);
        mpz_init(exp);

        for (int i = 0; i < threshold; i++)
        {
            // exp = x^i
            mpz_powm_ui(exp, shares[j].x, i, p);
            // temp = C_i^(exp)
            mpz_powm(temp, commitments[i], exp, p);
            // Multiply into rhs
            mpz_mul(rhs, rhs, temp);
            mpz_mod(rhs, rhs, p);
        }
        //gmp_printf("lhs: %Zd\n rhs: %Zd\n", lhs, rhs);
        //gmp_printf("gg Share %d: (%Zd, %Zd, %Zd)\n", 1, shares[j].x, shares[j].y, shares[j].r);
        if (mpz_cmp(lhs, rhs) == 0) {
            gmp_printf("Share is VALID for %Zd\n", shares[j].x);

        } else {
            gmp_printf("Share is INVALID for %Zd\n", shares[j].x);
        }

        mpz_clear(lhs);
        mpz_clear(g_pow);
        mpz_clear(h_pow);
        mpz_clear(rhs);
        mpz_clear(temp);
        mpz_clear(exp);
    }
}

// function for refreshing shares
void share_update(Share* shares, int n, int t, mpz_t prime, mpz_t g, mpz_t h)
{
    mpz_t secret;
    mpz_inits(secret, 0);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    seed_gmp_randstate(state);

    mpz_t* coeffs = malloc(sizeof(mpz_t) * t);
    mpz_t* r_vals = malloc(sizeof(mpz_t) * t);

    generate_polynomial(coeffs, t, secret, prime);

    // Generate Pedersen commitments
    mpz_t* commitments = malloc(t * sizeof(mpz_t));
    generate_pedersen_commitments(commitments, coeffs, r_vals, t, g, h, prime);

    // share generation of n parties
    for (int i = 0; i < n; ++i)
    {
        mpz_init_set_ui(shares[i].x, i + 1);
        mpz_init(shares[i].y);
        mpz_init(shares[i].r);
        // evaluate f(i) and store in shares[i].y
        evaluate_polynomial(shares[i].y, coeffs, t, shares[i].x, prime);
        // evaluate r(i) and store in shares[i].r
        evaluate_polynomial(shares[i].r, r_vals, t, shares[i].x, prime);
    }

    // verify share from commitments
    //verify_share(shares, n, t, commitments, g, h, prime);

    // Cleanup
    for (int i = 0; i < t; ++i)
    {
        mpz_clear(coeffs[i]);
        mpz_clear(r_vals[i]);
        mpz_clear(commitments[i]);
    }
    free(r_vals);
    free(commitments);
    free(coeffs);
    gmp_randclear(state);
    mpz_clear(secret);
}
