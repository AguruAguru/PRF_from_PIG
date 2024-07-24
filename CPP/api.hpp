#pragma once

#ifndef API_H
#define API_H

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
    RAW_UNIVERSAL = 0,
    NW_UNIVERSAL = 1,
    NW_RS = 2,
    NW_LOCAL_ENC = 3,
} running_mode;

typedef struct Bit {
    char val : 1;

    Bit(int _val) : val(_val & 1) {};
    Bit() : val(0) {};
} bit;

typedef bit (*evaluation_function)(const std::vector<bit>&);

const int NUM_TAPES = 4;
const int N = (1 << 8), T = 512;
const int log_RS_q = 8; // q: RS field size - TODO: calculate optimal
constexpr int RS_q = 1 << log_RS_q;
constexpr long long l = 16; // size of the design sets, bit length of each NW input
constexpr long long max_NW_input = -1; // (1 << l) - 1;

const int PAD_LENGTH = 32, INP_LENGTH = 15;
extern int TM[N][1 << NUM_TAPES];
extern int random_pad[NUM_TAPES][PAD_LENGTH];

/*
class Automata:
        self.d // regularity
        self.n // vertices count
        self.vertices 
        self.edges 

    def __init__(self, d, enc):
        init
    
    def run(self, input_str):
        Runs the DFA on the input string and returns the resulting bit

class DesignsPolynomials:
    A class to generate combinatorial designs, using the polynomials algorithm

    l: int
    q: int
    d: int # universe
    m: int
    log_q: int
    I: typing.Dict[int,typing.List[int]]

    def __init__(self, l, m):
        // init

        self.l = l
        self.m = m
        self.log_q = (l-1).bit_length() 
        self.q = 1<<self.log_q
        self.d = l*self.q
        self.I = {}

    def explicit_calculation(self, i, y):
        index in the design and the string to operate on, returns y restricted to the bits in the i'th design set

class NW:
    def __init__(self, hard_function, security_param, n, designs=None):
        init
    
    def explicit_calculation(self, i, y):
        get the i'th bit of the generator output, using y
    
    def restrict_y(self, i, y):
        get the input for the i'th call to the hard function, using y

def designs_for_NW(security_param, n)
    generate designs for the NW generator

def h(pis, y, i, security_param, n)
    with XOR amplification

*/

class DesignsPolynomials {
    public:
    unsigned l;
    unsigned q;
    unsigned d;
    unsigned m;
    unsigned log_q;
    std::unordered_map<int, std::vector<int>> I;

    DesignsPolynomials(unsigned l, unsigned m);

    std::vector<bit> explicit_calculation(unsigned i, const std::vector<bit>& y);

};

    
class NW {
    public:
    bit (*hard_function)(const std::vector<bit>&);
    unsigned log_security_param;
    unsigned n;
    DesignsPolynomials *designs;

    NW(evaluation_function hard_function, unsigned log_security_param, unsigned n, DesignsPolynomials *designs=nullptr);

    ~NW();

    bit explicit_calculation(unsigned i, const std::vector<bit>& y);
    
    std::vector<bit> restrict_y(unsigned i, const std::vector<bit>& y);
};


bit locally_encode_explicit_calc(long long inp);
std::vector<bit> get_TM_tt(int table_length = 65536, bool printProgress = false);
std::vector<bit> apply_hadamard(std::vector<bit> tt);
std::vector<bit> apply_RS(std::vector<bit> msg);


/* generate a random TM with tapes*/
void random_TM(int TM[][1 << NUM_TAPES]);

void gen_random_pad(int random_pad[][PAD_LENGTH]);

/* emulate the TM on the given input and return its output bit (last bit of final state)*/
bit emulate_TM(const int TM[][1 << NUM_TAPES], int64_t inp);

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