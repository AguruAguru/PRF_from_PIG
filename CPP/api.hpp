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

#define BIT_PREFIX_MASK(x) ( ( 1 << (x)) - 1)

typedef enum running_mode : int {
    RAW_UNIVERSAL = 0, // Output the truth table of the universal function (simply running the TM on consecutive inputs); Explicitly computable
    NW_UNIVERSAL = 1, // Run the NW generator using the universal function, this is effectively the original construction from [LP24]; Explicitly computable
    NW_RS = 2, // Run the NW generator using the truth table obtained by applying Reed-Solomon and then Hadamard on the truth table of the universal function; Not explicitly computable
    NW_LOCAL_ENC = 3, //Run the NW generator using the truth table obtained by applying local encoding (see documentation for 'locally_encode_explicit_calc'); Explicitly computable
} running_mode;

typedef struct Bit {
    char val : 1;

    Bit(int _val) : val(_val & 1) {};
    Bit() : val(0) {};
} bit;

/* -------------------- configuration -------------------- */

const int NUM_TAPES = 3; // number of tapes used by TM
const int N = (1 << 5); // number of states
const int T = 100; // for how many steps to emulate the TM
constexpr long long l = 23*6; // size of the design sets, bit length of each NW input
const int PAD_LENGTH = 0; // How much random padding is used per tape
const int INP_LENGTH = 15; // Input length taken for each tape
extern int TM[N][1 << NUM_TAPES]; // the TM itself
extern int random_pad[NUM_TAPES][PAD_LENGTH]; // the random pading

const int log_RS_q = 8; // q: RS field size - TODO: calculate optimal
constexpr int RS_q = 1 << log_RS_q;
constexpr int RS_d = 32; // RS distance
constexpr int w = 6; // local encoding vector length

/*

Notes about the parameters :
- Random bits used for TM: N*NUM_TAPES*log(N) + PAD_LENGTH*NUM_TAPES
- Random bits used for NW: ~l^2, l times smallest power of two ge to l.

- If RS is used, l should be rather small, as the entire TM tt is generated.
- If local encoding is used, and one wants 1M output bits for example, l should be the vector size of the local encoding times roughly log(1M). One should also consider INP_LENGTH accordingly. TODO: need more bits but why

- T should allow viewing the inputs in their entirety
- RS_q is also the parameter for the local computation of the folloiwng hadamard code, and thus the final TT will be of size (RS_q - 1) * (RS_q / log_RS_q), and so output size is to be considered respectively.

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
    unsigned m; // how many design sets are to be considered
    unsigned log_q;
    std::unordered_map<int, std::vector<int>> I; // the designs

    /* init */
    DesignsPolynomials(unsigned l, unsigned m);

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
    unsigned n; // n
    DesignsPolynomials *designs; // the designs used. One can use any combinatorial designs

    /* init */
    NW(evaluation_function hard_function, unsigned log_security_param, unsigned n, DesignsPolynomials *designs=nullptr);

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
bit locally_encode_explicit_calc(uint64_t inp);

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
    d: ? // TODO
    k: n - d + 1
*/
std::vector<bit> apply_RS(std::vector<bit> msg);


/* returns the field GF(2^field_descriptor) */
inline schifra::galois::field* getGFOverF2(unsigned field_descriptor) {
    #define OPEN_CASE(index, inp, X) case index: X(inp) break;
    #define ALL_POSS_DESC(X) \
    switch(field_descriptor - 2){ \
        OPEN_CASE(0, 0##0, X) \
        OPEN_CASE(1, 0##1, X) \
        OPEN_CASE(2, 0##2, X) \
        OPEN_CASE(3, 0##3, X) \
        OPEN_CASE(4, 0##4, X) \
        OPEN_CASE(5, 0##5, X) \
        OPEN_CASE(6, 0##6, X) \
        OPEN_CASE(7, 0##7, X) \
        OPEN_CASE(8, 0##8, X) \
        OPEN_CASE(9, 0##9, X) \
        OPEN_CASE(10, 10, X) \
        OPEN_CASE(11, 11, X) \
        OPEN_CASE(12, 12, X) \
        OPEN_CASE(14, 14, X) \
    }

    #define GEN_FINITE_FIELD(desc) field = new schifra::galois::field(field_descriptor, schifra::galois::primitive_polynomial_size##desc, schifra::galois::primitive_polynomial##desc);

    schifra::galois::field *field;

    ALL_POSS_DESC(GEN_FINITE_FIELD)

    return field;
}

#endif