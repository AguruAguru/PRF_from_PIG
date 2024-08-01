#include <cstdlib>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include <vector>

#include "api.hpp"

DesignsPolynomials::DesignsPolynomials(unsigned l, unsigned m) {
    this->l = l;
    this->m = m;
    this->log_q = (int)log2(l-1) + 1; // smallest power of two that is ge to l
    this->q = 1 << this->log_q;
    this->d = this->l*this->q;
    this->I = {};
}

std::vector<bit> DesignsPolynomials::explicit_calculation(unsigned i, const std::vector<bit>& y){
    // index in the design and the string to operate on

    if (this->I.find(i) == I.end()){
        std::size_t field_descriptor = this->log_q;

        const schifra::galois::field *GF_q = getGFOverF2(field_descriptor);
        
        unsigned deg = (int)(((int)log2(i)+1) / this->log_q) + ((((int)log2(i)+1) % this->log_q) != 0) - 1;
        std::vector<schifra::galois::field_element> coeffs(0, schifra::galois::field_element(*GF_q, 0));

        for (int j = deg; j >= 0; --j)
            coeffs.push_back(schifra::galois::field_element(*GF_q, ((i >> (this->log_q * j)) % (this->q))));  // i in base q, determines the coeffs for the polynomial

        schifra::galois::field_polynomial poly = schifra::galois::field_polynomial(*GF_q, deg, &coeffs[0]); // should be safe to do

        std::vector<int> I_i = {}; // the i'th design in the designs collection
        for (int k = 0; k < this->l; ++k)
            I_i.push_back(k*this->q + static_cast<int>(poly(schifra::galois::field_element(*GF_q, k)).index()));

        std::sort(I_i.begin(), I_i.end());

        this->I[i] = I_i;

        delete GF_q;
    }
    

    std::vector<bit> result = {};
    for (int index = 0; index < this->l; ++index)
        result.push_back(y[this->I[i][index]].val);
    
    return result;
}


NW::NW(bit (*hard_function)(const std::vector<bit>&), unsigned log_security_param, unsigned n, DesignsPolynomials *designs) {
    this->hard_function = hard_function;
    this->log_security_param = log_security_param;
    if (designs == nullptr){
        this->designs = new DesignsPolynomials(
            log_security_param, // what params??
            (int)std::pow(n, 1/8)
        );
    }
    else
        this->designs = designs;
}

NW::~NW() {
    delete designs;
}

bit NW::explicit_calculation(unsigned i, const std::vector<bit>& y) {
    auto y_restricted = this->designs->explicit_calculation(i,y);
    return this->hard_function(y_restricted);
}

std::vector<bit> NW::restrict_y(unsigned i, const std::vector<bit>& y) {
    return this->designs->explicit_calculation(i,y);
}


/*
int main() {
    DesignsPolynomials design(32, 1 << 10);
    std::vector<bit> y = {};
    for (int i = 0; i < design.d; ++i)
        y.push_back(rand() & 1);

    auto res = design.explicit_calculation(2, y);
    std::cout << sizeof(Bit) << std::endl;
    for (auto elem : res) {
        std::cout << (elem.val & 1) << std::endl;
    }
    return 0;
}*/