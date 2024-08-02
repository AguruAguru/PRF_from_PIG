#ifndef API_HPP
#define API_HPP

#define NO_GFLUT
#include "schifra_galois_field.hpp"
#undef NO_GFLUT
#include "schifra_galois_field_polynomial.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_reed_solomon_block.hpp"
#include "schifra_error_processes.hpp"

#include <random>
#include <cassert>
#include <cmath>

#define assertm(exp, msg) assert(((void)msg, exp))


#define BIT_PREFIX_MASK(x) ( ( 1 << (x)) - 1)

typedef enum running_mode : int {
    RAW_UNIVERSAL = 0, // Output the truth table of the universal function (simply running the TM on consecutive inputs); Explicitly computable
    NW_UNIVERSAL = 1, // Run the NW generator using the universal function, this is effectively the original construction from [LP24]; Explicitly computable
    NW_RS = 2, // Run the NW generator using the truth table obtained by applying Reed-Solomon and then Hadamard on the truth table of the universal function; Not explicitly computable
    NW_LOCAL_ENC = 3, // Run the NW generator using the truth table obtained by applying local encoding (see documentation for 'locally_encode_explicit_calc'); Explicitly computable
    ENCRYPT_STREAM = 4, // Uses NW with RS as a twice expanding PRG, as a stream cipher
    CYCLICALLY_APPLY = 5 // Uses NW with RS as a twice expanding PRG, cyclically on itself
} running_mode;

typedef struct Bit {
    char val : 1;

    Bit(int _val) : val(_val & 1) {};
    Bit() : val(0) {};

    operator int() const { return val & 1; }
} bit;

/* -------------------- configuration -------------------- */

constexpr int NUM_TAPES = 2; // number of tapes used by TM
constexpr int log_N = 5; // log number of states
constexpr int N = (1 << log_N); // number of states
const int T = 150; // for how many steps to emulate the TM
constexpr long long l = 12; // size of the design sets, bit length of each NW input
const int PAD_LENGTH = 0; // How much random padding is used per tape
const int INP_LENGTH = 15; // Input length taken for each tape
extern int TM[N][1 << NUM_TAPES]; // the TM itself
extern int random_pad[NUM_TAPES][PAD_LENGTH]; // the random pading

const int log_RS_q = 7; // q: RS field size
constexpr int RS_q = 1 << log_RS_q;
constexpr int RS_d = 32; // RS distance
constexpr int LOCAL_ENC_k = 2; // input size for every TM emulation when using local encoding, k+1 should divide l

constexpr int seedLen = ( N*(1 << NUM_TAPES)*(log_N + 2*NUM_TAPES) + PAD_LENGTH*NUM_TAPES ) + ( l * ( 1 << ((int)log2(l-1) + 1)) ); // used for the stream cipher + cyclic length test; Formula in notes

/*

Notes about the parameters :
- Random bits used for TM: N*(2^NUM_TAPES)*(log(N) + 2NUM_TAPES) + PAD_LENGTH*NUM_TAPES
- Random bits used for NW: ~l^2, l times smallest power of two ge to l.

- INP_LENGTH should be set to the total inputh length divided by 'NUM_TAPES', e.g. if local encoding is used then it should be LOCAL_ENC_k/NUM_TAPES

- If RS is used, l should be rather small, as the entire TM tt is generated.

- T should allow viewing the inputs in their entirety
- RS_q is also the parameter for the local computation of the folloiwng hadamard code, and thus the final TT will be of size (RS_q - 1) * (RS_q / log_RS_q), and so output size is to be considered respectively.

- The stream cipher seed length is configured internally and should be modified alongside 
*/

/* -------------------- TM Emulation -------------------- */

/* generate a random TM with tapes */
void random_TM(int TM[][1 << NUM_TAPES]);

/*
    Pad each of the 'NUM_TAPES' arrays with 'PAD_LENGTH' random bits.
*/
void gen_random_pad(int random_pad[][PAD_LENGTH]);

/* 
Emulates the provided TM on the given input and return its output bit (last bit of final state)

* distribute 'inp' accross the tapes, taking 'INP_LENGTH' bits for each tape.
* append the random padding to the inputs on the tapes
* run the TM for 'T' steps
* output the last bit of the final state

*/
bit emulate_TM(const int TM[][1 << NUM_TAPES], int64_t inp);


/* -------------------- NW & Designs -------------------- */

typedef bit (*evaluation_function)(const std::vector<bit>&);

class DesignsPolynomials {
    /* class to compute and maintain combinatorial designs, computed using polynomials over a finite field */

    public:
    unsigned l; // the size of each set in the design
    unsigned q; // smallest power of two ge to l
    unsigned d; // universe size, equals l*q
    unsigned log_q;
    std::unordered_map<int, std::vector<int>> I; // the designs

    /* init */
    DesignsPolynomials(unsigned design_l);

    /* Explicitly calculate the restriction of 'y' to the indices in the i'th set in the design

    * If the i'th set is not yet calculated:
        * Init GF(q)=GF(2^log_q)
        * Calculate the i'th polynomial p over GF(q), canonically sorted lexicographically by the coefficients, taking the digits of i in base q as the coefficients
        * Construct the design set as all of the pairs (k, p(k)) for k < l; specifically takes all k*q + p(k), using interchangeably elements in GF(q) as also integers in Z/qZ.
        * sort the set
    * return the restriction of y to the indices in the aforementioned set.

    */
    std::vector<bit> explicit_calculation(unsigned i, const std::vector<bit>& y);
};

class NW {
    /* apply the NW generator, given a hard function and designs*/

    public:
    bit (*hard_function)(const std::vector<bit>&); // the hard function to be used by the generator
    unsigned log_security_param; // the log of the security param
    DesignsPolynomials *designs; // the designs used. One can use any combinatorial designs

    /* init */
    NW(evaluation_function hard_function, unsigned design_l, DesignsPolynomials *designs=nullptr);

    ~NW();

    /*
        Explicitly compute the hard function on the restriction of y according to the i'th design set.

        * Call designs->explicit_calculation(i, y) to get the restriction of y
        * Return the result of the hard function on the restriction
    */
    bit explicit_calculation(unsigned i, const std::vector<bit>& y);
    
    /*
        Simply return the restriction of y without applying the hard function
    */
    std::vector<bit> restrict_y(unsigned i, const std::vector<bit>& y);
};

/* -------------------- ECCs -------------------- */

/*
    Given the input and after having initialized the TM, calculate the local encoding of the universal function on this input

    * Deconstuct the first 'l' bits of the input to be 'w' (internal param) vectors of length 'k', and another vector of length 'w' (so l=w*k + w)
    * Compute the output bit of the TM on each of the resulting k-bit strings 
    * return the inner product of the vector of the output bits, with the final reserved w-bit vector.
    
    * Formally computes: <(TM(inp[0:k-1]), ..., TM(inp[w*(k-1):w*k])), (inp[w*k], ..., inp[w*k + w - 1])>
*/
bit locally_encode_explicit_calc(const std::vector<bit>& inp);

/*
    Return the first 'table_length' bits of the truth table of the TM
*/
std::vector<bit> get_TM_tt(int table_length = 65536, bool printProgress = false);

/*
    Apply the Hadamard code on the provided tt, on every loq_RS_q bits.

    * For every log_RS_q bit string in the tt, compute the inner product with all log_RS_q bit vectors
    * Concatenate the inner products to the results from the previous log_RS_q strings
    * Return the computed table
*/
std::vector<bit> apply_hadamard(std::vector<bit> tt);

/*
    Apply the Reed Solomon code on the given message, with parameters
    Field size: RS_q
    n: RS_1 - 1
    d: 32
    k: n - d + 1
*/
std::vector<bit> apply_RS(std::vector<bit> msg);

/* ------------------ util ------------------ */

/* returns the field GF(2^field_descriptor) */
schifra::galois::field* getGFOverF2(unsigned field_descriptor);

void setSeed(const std::vector<bit>& stream);

bit randBit();

int randIntMod(int mod);

void printRandomUsageStats();

int64_t bitsToInt(const std::vector<bit>& inp);

#endif