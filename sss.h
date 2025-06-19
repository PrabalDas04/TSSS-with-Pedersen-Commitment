#include <gmp.h>

typedef struct {
    mpz_t x;
    mpz_t y;
    mpz_t r;
} Share;

void seed_gmp_randstate(gmp_randstate_t state);
void generate_prime(mpz_t prime, int bits);
void generate_polynomial(mpz_t* coeffs, int threshold, mpz_t secret, mpz_t prime);
void evaluate_polynomial(mpz_t result, mpz_t* coeffs, int threshold, mpz_t x, mpz_t prime);
void deal_shares(Share* shares, int n, int t, mpz_t secret, mpz_t prime);
void reconstruct_secret(mpz_t secret, Share* shares, int t, mpz_t prime);
void generate_pedersen_commitments(mpz_t *commitments, mpz_t *coeffs, mpz_t *r_vals, int threshold, mpz_t g, mpz_t h, mpz_t p);
void verify_share(Share *shares, int n, int threshold, mpz_t *commitments, mpz_t g, mpz_t h, mpz_t p);
void share_update(Share* shares, int n, int t, mpz_t prime, mpz_t g, mpz_t h);