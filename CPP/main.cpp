#include "api.hpp"

#include <fstream> 
#include <iostream> 
#include <cassert>
#define assertm(exp, msg) assert(((void)msg, exp))

int64_t bitsToInt(const std::vector<bit>& inp) {
    int64_t int_res = 0;
    for (int i = 0; i < inp.size(); ++i) {
        int_res = (int_res << 1) + (inp[i].val & 1);
    }
    return int_res;
}

bit eval_TM(const std::vector<bit>& inp){
    int64_t int_inp = bitsToInt(inp);
    
    return emulate_TM(TM, int_inp);
}

bit eval_TM_RS(const std::vector<bit>& inp) {
    static auto ecc_tt = apply_hadamard(apply_RS(get_TM_tt(1 << l)));
    assertm(l < 25, "Used l value is too high to use with Reed Solomon");
    return ecc_tt[bitsToInt(inp)];
}

bit evalLocalEnc(const std::vector<bit>& inp) {
    return locally_encode_explicit_calc(bitsToInt(inp));
}

evaluation_function get_hard_func(running_mode mode) {
    switch (mode) {
        case NW_UNIVERSAL: return eval_TM;
        case NW_RS: return eval_TM_RS;
        case NW_LOCAL_ENC: return evalLocalEnc;
    }
    return nullptr;
}

std::vector<bit> runNW(running_mode mode, unsigned outLen) {
    NW *NW_gen = new NW(get_hard_func(mode), l, 1 << 10);

    std::vector<bit> y = {};
    for (int i = 0; i < NW_gen->designs->d; ++i)
        y.push_back(rand() & 1);

    std::vector<bit> outputs = {};

    for (int i = 1; i <= outLen; ++i) {
        outputs.push_back(NW_gen->explicit_calculation(i, y).val & 1);
        if (! (i % (1 + (int)(outLen/100))))
            printf("%d%%\n", (int) (100*i)/outLen);
    }

    delete NW_gen;

    return outputs;   
}

int main(int argc, char *argv[]) {
    std::ofstream out("./outputs/output.txt", std::ios::out); 

    random_TM(TM);
    gen_random_pad(random_pad);

    running_mode mode = (running_mode)atoi(argv[1]);
    const unsigned outLen = atoi(argv[2]);

    std::vector<bit> outputs;

    if (mode == running_mode::RAW_UNIVERSAL) {
        outputs = get_TM_tt(outLen, true);
    } else {
        outputs = runNW(mode, outLen);
    }    

    float ones = 0;

    for (bit b : outputs) {
        ones += (b.val & 1);
        out << (b.val & 1);
    }

    std::cout << "Balance: " << ones/outLen;
    out.close();    

    return 0;
}