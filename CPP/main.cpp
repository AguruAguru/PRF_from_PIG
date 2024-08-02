#include "api.hpp"

#include <fstream>
#include <iostream>

/* ----------- the hard function cadidates - the callbacks provided to the NW gen ----------- */

bit eval_TM(const std::vector<bit>& inp){
    int64_t int_inp = bitsToInt(inp);
    
    return emulate_TM(TM, int_inp);
}

bit eval_TM_RS(const std::vector<bit>& inp) {
    static auto ecc_tt = apply_hadamard(apply_RS(get_TM_tt((RS_q - RS_d + 1)*log_RS_q)));
    assertm(l < 20, "Used l value is too high to use with Reed Solomon");
    return ecc_tt[bitsToInt(inp)];
}

bit evalLocalEnc(const std::vector<bit>& inp) {
    return locally_encode_explicit_calc(inp);
}

evaluation_function get_hard_func(running_mode mode) {
    switch (mode) {
        case NW_UNIVERSAL: return eval_TM;
        case NW_RS: return eval_TM_RS;
        case NW_LOCAL_ENC: return evalLocalEnc;
    }
    return nullptr;
}


/* 
    Runs the NW generator and returns a truth table of length outLen

    * init the generator with the corresponding hard function
    * init 'y', the random string (taken from the seed) to which the NW treat as the input
    * concatenate the explicit computation results from all inputs through outLen
    * return result
*/
std::vector<bit> runNW(running_mode mode, unsigned outLen, bool printProgress = true) {
    NW *NW_gen = new NW(get_hard_func(mode), l);

    std::vector<bit> y = {};
    for (int i = 0; i < NW_gen->designs->d; ++i)
        y.push_back(randBit());

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

/*
    Runs as a stream cipher (encoding "./plaintext.txt" currently), with seed length 1344 (should match config)

    * Sequencially apply the RS version with an expansion factor of 2
    * Each time tak the first half and xor it with the plaintext (and save the results of course), and take the second half as the seed to the next iteration
    * When the plaintext has been processed in its entirety, return the concatenation of all the xored values
*/
std::vector<bit> streamCipher() {
    std::ifstream pt = std::ifstream("./plaintext.txt", std::ios::in);

    std::vector<bit> result = {};

    char curr;

    while(pt.good()) {
        std::vector<bit> currBits = {};

        for (int i = 0; i < seedLen/8 && pt.get(curr); ++i)
            for (int j = 0; j < 8; ++j)
                currBits.push_back((curr >> j) & 1);

        auto PRG_tt = runNW(running_mode::NW_RS, seedLen*2, false);

        for (int i = 0; i < currBits.size(); ++i)
            result.push_back((currBits[i].val ^ PRG_tt[i].val) & 1); // first half used to xor

        setSeed(std::vector<bit>(PRG_tt.begin() + seedLen, PRG_tt.end())); // second half used as future seed

        random_TM(TM);
        gen_random_pad(random_pad);
    }

    return result;
}

/*
    Cyclicly apply the RS vesion on itself (with exapnsion factor of 1), and return the resulting string after 100000 applications
*/
std::vector<bit> cyclicApplication() {
    std::vector<bit> result = {};

    char curr;

    int bucket = 100;

    for (int i = 0; i < 100*bucket; ++i) {
        if (!(i%bucket))
            std::cout << (int)i/bucket << "%" << std::endl;

        auto PRG_tt = runNW(running_mode::NW_RS, seedLen, false);

        setSeed(std::vector<bit>(PRG_tt.begin(), PRG_tt.end())); // output becomes next seed

        random_TM(TM);
        gen_random_pad(random_pad);
    }

    return runNW(running_mode::NW_RS, seedLen, false);;
}

int main(int argc, char *argv[]) {
    std::ofstream out;
    if (argc > 3)
        out = std::ofstream(argv[3], std::ios::out);
    else if (argc >= 2)
        out = std::ofstream("./output.txt", std::ios::out);
    else {
        std::cout << "USAGE <program> <running mode> [output length] [output file]\n"
                    << "0: Universal function truth table\n"
                    << "1: NW generator, with universal function oracle\n"
                    << "2: NW generator, with Reed Solomon ECC\n"
                    << "3: NW generator, with local encoding ECC\n"
                    << "4: Stream cipher, encrypts contents of ./plaintext.txt, using NW with Reed Solomon\n"
                    << "5: Cyclically apply PRG 10000 times, using NW with Reed Solomon\n\n" 
                    << "Outputs to ./output.txt unless stated otherwise; configuration of other paramteres requires recompiling" << std::endl;
        return 1;
    }

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
            if (argc >= 3)
                out = std::ofstream(argv[2], std::ios::out);
            outputs = streamCipher();
            outLen = outputs.size();
            break;
        case running_mode::CYCLICALLY_APPLY: 
            if (argc >= 3)
                out = std::ofstream(argv[2], std::ios::out);
            outputs = cyclicApplication(); 
            outLen = outputs.size();
            break;
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