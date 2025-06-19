# Shamir's Secret Sharing Scheme with Pedersen Commitment

This project is a C implementation of **Shamir's Secret Sharing Scheme (SSS)** with **Pedersen commitments** for share verification.

## Features

- Shamir’s (t, n) Secret Sharing
- Pedersen Commitment for Share Validation
- Large integer operations with GMP
- Command–line tools for convenient usage
- Separate programs for **sharing** and **reconstructing** the secret.
- Anyone can refresh the share without knowing the secret or previous shares


## File Structure
  |-- sss-deal.c
  |-- sss-recon.c
  |-- sss-refresh.c
  |-- sss_source.c
  |-- sss.h
  |-- test.txt
  |-- README.md

## Build Instructions

Ensure you have GMP installed:

---bash
$ sudo apt-get install libgmp-dev

Then, compile
---bash
$ gcc -o sss-deal sss-deal.c sss_source.c sss.h -lgmp -lcrypto
$ ./sss-deal <secret> <threshold> <shares>

Example:
---bash
$ ./sss-deal 12345678 3 5
#output:
Prime used: 21943360814533509326657313729417832173678270435879030953656745540963122520173
Share 1: (1, 7439331108623755954382487379713266945203487631859107254850491975062667055920)
Share 2: (2, 8849510331777628903125495176154783064336986968459657493892254785066342121011)
Share 3: (3, 4230537669461618846229023389324548357400498009801650717125288430011037540951)
Share 4: (4, 15525773936209235110350385748640394998072291191764117878206338450859875835913)
Share 5: (5, 20791858317486968368832268524684490812674096078468028023478659306649734485724)

---bash
$ gcc -o sss-recon sss-recon.c sss_source.c sss.h -lgmp -lcrypto
$ ./sss-recon <prime> <share no> <share>

Example:
$ ./sss-recon 21943360814533509326657313729417832173678270435879030953656745540963122520173 1 7439331108623755954382487379713266945203487631859107254850491975062667055920 3 4230537669461618846229023389324548357400498009801650717125288430011037540951 5 20791858317486968368832268524684490812674096078468028023478659306649734485724

#output:
$ Reconstructed Secret: 12345678

# Pedersen Commitment
We are using Pedersen commitment scheme for polynomial commitment. There are two functions, one for generating pedersen commitment of the coefficients *generate_pedersen_commitments()* and another is for verifying the commitments *verify_share()*, both are being called from *deal_shares()* function after generating the shares.

# Refresh share
Dealer or anyone can refresh the shares without knowing the previous secrets using *share_update()* function. This generates share of secret 0 along with it's commitments. Parties need to verify the share and add it to the previous share. If *f()* was the previous polynomial and *g()* is the 0-constant polynomial, then the new polynomial is *f+g* and the i-th share is *f(i)+g(i)* i.e. the previous share add to share of secret 0.

--bash
$ gcc sss-refresh.c sss.h sss_source.c -o sss-refresh -lgmp -lcrypto
$ ./sss-refresh <prime> <threshold> <shares> <g> <h>