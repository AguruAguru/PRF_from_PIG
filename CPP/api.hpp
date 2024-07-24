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


typedef struct Bit {
    char val : 1;

    Bit(int _val) : val(_val & 1) {};
    Bit() : val(0) {};
} bit;

const int NUM_TAPES = 4;
const int N = (1 << 8), T = 512;
const int log_RS_q = 8; // q: RS field size - TODO: calculate optimal
constexpr int RS_q = 1 << log_RS_q;
constexpr long long l = 32; // size of the design sets, bit length of each NW input
constexpr long long max_NW_input = -1; // (1 << l) - 1;

int TM[N][1 << NUM_TAPES];
const int PAD_LENGTH = 32, INP_LENGTH = 15;
int random_pad[NUM_TAPES][PAD_LENGTH];

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
    bit (*hard_function)(const std::vector<bit>&);
    unsigned log_security_param;
    unsigned n;
    DesignsPolynomials *designs;

    NW(bit (*hard_function)(const std::vector<bit>&), unsigned log_security_param, unsigned n, DesignsPolynomials *designs=nullptr);

    ~NW();

    bit explicit_calculation(unsigned i, const std::vector<bit>& y);
    
    std::vector<bit> restrict_y(unsigned i, const std::vector<bit>& y);
};


/* generate a random TM with tapes*/
void random_TM(int TM[][1 << NUM_TAPES]);

void gen_random_pad(int random_pad[][PAD_LENGTH]);

/* emulate the TM on the given input and return its output bit (last bit of final state)*/
bit emulate_TM(const int TM[][1 << NUM_TAPES], int64_t inp);


