#include "api.hpp"

#include <fstream>
#include <iostream>


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
    assertm(l < 20, "Used l value is too high to use with Reed Solomon");
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

std::vector<bit> runNW(running_mode mode, unsigned outLen, bool printProgress = true) {
    NW *NW_gen = new NW(get_hard_func(mode), l, 1 << 10);

    std::vector<bit> y = {};
    for (int i = 0; i < NW_gen->designs->d; ++i)
        y.push_back(randBit());
    // for (auto b : y)
    //     std::cout << (b.val & 1);
    if (printProgress)
        std::cout << "NW randomness: " << NW_gen->designs->d << std::endl;

    std::vector<bit> outputs = {};

    for (int i = 1; i <= outLen; ++i) {
        outputs.push_back(NW_gen->explicit_calculation(i, y).val & 1);
        if (printProgress && ! (i % (1 + (int)(outLen/100))))
            printf("%d%%\n", (int) (100*i)/outLen);
    }

    delete NW_gen;

    return outputs;   
}

std::vector<bit> streamCipher() {
    static const int seedLen = 1344;
    std::ifstream pt = std::ifstream("./alice.txt", std::ios::in);

    std::vector<bit> result = {};

    char curr;

    while(pt.good()) {
        std::vector<bit> currBits = {};

        for (int i = 0; i < seedLen/8 && pt.get(curr); ++i)
            for (int j = 0; j < 8; ++j)
                currBits.push_back((curr >> j) & 1);

        auto PRG_tt = runNW(running_mode::NW_RS, seedLen*2, false);

        for (int i = 0; i < currBits.size(); ++i)
            result.push_back((currBits[i].val ^ PRG_tt[i].val) & 1);
        setSeed(std::vector<bit>(PRG_tt.begin() + seedLen, PRG_tt.end()));

        random_TM(TM);
        gen_random_pad(random_pad);
    }

    return result;
}

std::vector<bit> cyclicApplication() {
    static const int seedLen = 1344;

    std::vector<bit> result = {};

    char curr;

    int bucket = 100;

    for (int i = 0; i < 100*bucket; ++i) {
        if (!(i%bucket))
            std::cout << (int)i/bucket << "%" << std::endl;

        auto PRG_tt = runNW(running_mode::NW_RS, seedLen, false);

        setSeed(std::vector<bit>(PRG_tt.begin(), PRG_tt.end()));

        random_TM(TM);
        // gen_random_pad(random_pad);
    }

    return runNW(running_mode::NW_RS, seedLen, false);;
}

int main(int argc, char *argv[]) {
    std::ofstream out;
    if (argc > 3)
        out = std::ofstream(argv[3], std::ios::out);
    else if (argc >= 2)
        out = std::ofstream("./outputs/output.txt", std::ios::out);
    else {
        std::cout << "USAGE <program> <running mode> <output length> [output file]" << std::endl;
        return 1;
    }

    /* 
    ------ for decrypting ------
    std::string str("110111111101110001010110001010111101100100100000001010111010000011100100010111010010001001001110001000011001110111110111000110000101100011110101000010100111010100100011011000111111100111101101000111101110100001111110001011111010000110111001111110101011100001100011010100101001100000010000010001100100111010100100100010110110101100001110101000010110100000000000101111010110100101100111011110111111010011100001100010001011001101101111010100100000000001100001010101010101110010010010100101110101111110111100110111011001100110101101101110101001100000011010010111101101000110111001001010101101111110001001010100110111111000100011011100100001000010101110101110101011101001010101110000011101101111010111010110000110001100111100010010001010001110101110011000111111011110001111101111010101111111100101111001000010011011011111011000011001001001111110011001110111100010101011101110111101100101011010001000000100001000010110110100010010110100000000011101000100001010100111110110111111101000110000110110011001100111000001000110000101100011100100011010110010110100010011011011100110100110011010101010101111101110010110010011111100000101000000111001001010101101011101010110001110011000100001010111010000000010011100010001000010111100111101010100101011100110110110010100111011110110000011011000001101011101111011111011110001100110001010011000100100010110101000");
    std::vector<bit> seed = {};
    for (char c : str)
        seed.push_back((c - '0') & 1);

    setSeed(seed); 
    */

    random_TM(TM);
    gen_random_pad(random_pad);

    running_mode mode = (running_mode)atoi(argv[1]);

    unsigned outLen;
    std::vector<bit> outputs;

    switch (mode) {
        case running_mode::RAW_UNIVERSAL:
            outLen = atoi(argv[2]);
            outputs = get_TM_tt(outLen, true);
            break;

        case running_mode::NW_UNIVERSAL:
        case running_mode::NW_RS:
        case running_mode::NW_LOCAL_ENC:
            outLen = atoi(argv[2]);
            outputs = runNW(mode, outLen);
            break;

        case running_mode::ENCRYPT_STREAM:
            outputs = streamCipher();
            outLen = outputs.size();
            break;
        default: outputs = cyclicApplication(); outLen = outputs.size();
    }

    float ones = 0;

    for (bit b : outputs) {
        ones += (b.val & 1);
        out << (b.val & 1);
    }

    std::cout << "Balance: " << 100*ones/outLen << "%" << std::endl;
    printRandomUsageStats();
    out.close();    

    return 0;
}