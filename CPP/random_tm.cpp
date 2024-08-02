#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <cstring>

#include "api.hpp"

int TM[N][1 << NUM_TAPES];
int random_pad[NUM_TAPES][PAD_LENGTH];
int tapes[NUM_TAPES][T + 10];

void random_TM(int TM[][1 << NUM_TAPES]) {
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<int> state_dist(0, N - 1);
    // std::uniform_int_distribution<int> tape_dist(0, (1 << NUM_TAPES) - 1);
    
    for (int node = 0; node < N; ++node)
        for (int idx = 0; idx < (1 << NUM_TAPES); ++idx)
        {
            int next_node = randIntMod(N), // state
                vals = randIntMod(1 << NUM_TAPES), // tape
                dirs = randIntMod(1 << NUM_TAPES); // tape
            TM[node][idx] = (next_node << (NUM_TAPES << 1)) | (vals << NUM_TAPES) | dirs;
        }
}

void gen_random_pad(int random_pad[][PAD_LENGTH])
{
    for (int i = 0; i < NUM_TAPES; ++i)
        for (int j = 0; j < PAD_LENGTH; ++j)
            random_pad[i][j] = (randBit().val & 1);
}


void load_tape(int tape[], uint64_t inp)
{
    for (int i = 0; i < INP_LENGTH; ++i, inp >>= 1)
        tape[i] = inp & 1;
}

int read_from_tapes(int heads[])
{
    int head_content = 0;
    for (int i = 0; i < NUM_TAPES; ++i)
        head_content |= tapes[i][heads[i]] << i;
    return head_content;
}

int edit_tape(int tape[], int head, int val, int dir)
{
    tape[head] = val;
    if (dir == 0) 
        dir = -1;
    head += dir;
    if (head < 0)
        head = 0;
    return head;
}

bit emulate_TM(const int TM[][1 << NUM_TAPES], int64_t inp) 
{
    // static std::string tt = "";
    // return bit((int)(tt[inp]) - '0');
    memset(tapes, 0, sizeof(tapes));
    for (int i = 0; i < NUM_TAPES; ++i)
    {
        load_tape(tapes[i], inp >> (i * INP_LENGTH));
        memcpy(tapes[i] + INP_LENGTH, random_pad[i], sizeof(random_pad[i]));
    }
    int heads[NUM_TAPES], state = 0;
    memset(heads, 0, sizeof(heads));
    
    for (int _ = 0; _ < T; ++_)
    {
        int head_content = read_from_tapes(heads);
        
        int next_state_tuple = TM[state][head_content];
        int next_state = next_state_tuple >> (NUM_TAPES << 1),
            vals = (next_state_tuple >> (NUM_TAPES)) & ((1 << NUM_TAPES) - 1),
            dirs = next_state_tuple & ((1 << NUM_TAPES) - 1);
        
        state = next_state;
        for (int j = 0; j < NUM_TAPES; ++j)
            heads[j] = edit_tape(tapes[j], heads[j], (vals >> j) & 1, (dirs >> j) & 1);
    }
    return bit(state & 1);
}

std::vector<bit> get_TM_tt(int table_length, bool printProgress) { // defualt value is case dependent
    std::vector<bit> tt = {};

    for (int inp = 0; inp < table_length; ++inp) {
        if (printProgress && (! (inp % (1 + (int)table_length/100))))
            printf("%d%%\n", (int)(100*inp)/table_length);

        tt.push_back(emulate_TM(TM, inp));
    }

    return tt;
}
/*
int main() {
    //int out = dup(stdout);
    freopen("./outputs/output.txt", "w", stdout);

    freopen("./../Python/NW_inputs_2.txt", "r", stdin);

    random_TM(TM);

    gen_random_pad(random_pad);

    std::vector<bit> outputs;

    auto tt = get_TM_tt();
    auto RS = apply_RS(tt);
    auto ecc_tt = apply_hadamard(RS);

    // for (int i = 0; i < 10; ++i) {
    //     for (int j = 0; j < log_q; ++j) {
    //         std::cout << (int)tt[i*log_q + j]; 
    //     }
    //     std::cout << ": ";
    //     for (int j = 0; j < q; ++j) {
    //         std::cout << had[i*q + j]; 
    //     }
    //     std::cout << std::endl;
    // }
    // return 0;

    int64_t inp = 0;

    while (cin >> inp)
        // outputs.push_back(locally_encode_explicit_calc(inp));
        outputs.push_back(ecc_tt[inp]);
        // outputs.push_back(emulate_TM(TM, inp));

    for (bit val : outputs)
        printf("%d", val.val & 1);

    //std::cout << static_cast<double>(sum) / outputs.size() << std::endl;
    /*
    int tape1_length = 32;
    std::vector<int> tape1(tape1_length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> tape1_dist(0, 1);
    for (int i = 0; i < tape1_length; ++i) {
        tape1[i] = tape1_dist(gen);
    }

    std::vector<int> outputs;
    for (int inp = 0; inp < 256; ++inp) {
        std::vector<int> tape2 = load_tape(inp);
        std::vector<std::vector<int>> tapes = {tape1, tape2};
        int output = emulate_TM(TM, tapes, 512, true, inp);
        outputs.push_back(output);
    }

    for (int val : outputs) {
        std::cout << val;
    }
    std::cout << std::endl;

    int sum = 0;
    for (int val : outputs) {
        sum += val;
    }
    std::cout << static_cast<double>(sum) / outputs.size() << std::endl;
    *

    return 0;
}
*/