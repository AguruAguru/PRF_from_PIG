#include "api.hpp"

/* returns the field GF(2^field_descriptor) */
schifra::galois::field* getGFOverF2(unsigned field_descriptor) {
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

std::vector<bit> streamRand = {};
std::vector<bit> usedSeed = {};
std::vector<bit>::iterator itRand = streamRand.end();


void setSeed(const std::vector<bit>& stream) {
    streamRand = stream;
    itRand = streamRand.begin();
}


bit randBit() {    
    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int> bin_dist(0, 1);

    if (itRand != streamRand.end())
        return *(itRand++);    

    // Using "true" randomness        
    int res = bin_dist(gen);
    usedSeed.push_back(res & 1);
    return res & 1;
}

int randIntMod(int mod) {
    assertm(__builtin_popcount(mod) <= 1, "Random numbers requested should only be mod powers of two");

    int res = 0;

    while ((mod >>= 1)) {
        res = (res << 1) + (randBit().val & 1);
    }

    return res;
}

void printRandomUsageStats() {
    std::cout << "Used seed: ";
    for (bit b : usedSeed)
        std::cout << (b.val & 1);
    std::cout << std::endl << "Random bits used: " << usedSeed.size() << std::endl;
}