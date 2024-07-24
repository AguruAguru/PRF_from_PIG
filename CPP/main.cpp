#include "api.hpp"

int main() {
    DesignsPolynomials design(32, 1 << 10);
    std::vector<bit> y = {};
    for (int i = 0; i < design.d; ++i)
        y.push_back(rand() & 1);

    random_TM(TM);

    gen_random_pad(random_pad);

    vector<int> outputs;
}