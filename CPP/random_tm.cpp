#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <bitset>
#include <cstring>

#include "api.hpp"

using namespace std;

int TM[N][1 << NUM_TAPES];
int random_pad[NUM_TAPES][PAD_LENGTH];


void random_TM(int TM[][1 << NUM_TAPES]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> state_dist(0, N - 1);
    std::uniform_int_distribution<int> tape_dist(0, (1 << NUM_TAPES) - 1);
    
    for (int node = 0; node < N; ++node)
        for (int idx = 0; idx < (1 << NUM_TAPES); ++idx)
        {
            int next_node = state_dist(gen), 
                vals = tape_dist(gen),
                dirs = tape_dist(gen);
            TM[node][idx] = (next_node << (NUM_TAPES << 1)) | (vals << NUM_TAPES) | dirs;
        }
}
/*
std::unordered_map<std::pair<int, int>, std::tuple<int, int, int>, std::hash<std::pair<int, int>>> random_TM(int n) {
    std::unordered_map<std::pair<int, int>, std::tuple<int, int, int>, std::hash<std::pair<int, int>>> TM;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> n_dist(0, n - 1);
    std::uniform_int_distribution<int> tape_dist(0, (1 << num_tapes) - 1);

    for (int node = 0; node < n; ++node) {
        for (int idx = 0; idx < (1 << num_tapes); ++idx) {
            TM[std::make_pair(node, idx)] = std::make_tuple(n_dist(gen), tape_dist(gen), tape_dist(gen));
        }
    }
    return TM;
}
*/

/*
int read_from_tapes(const std::vector<std::vector<int>>& tapes, const std::vector<int>& heads) {
    int head_content = 0;
    for (int i = 0; i < num_tapes; ++i) {
        const std::vector<int>& tape = tapes[i];
        int head = heads[i];
        head_content |= tape[head] << i;
    }
    return head_content;
}

std::tuple<std::vector<std::vector<int>>, std::vector<int>> update_tapes(
    const std::vector<std::vector<int>>& tapes,
    const std::vector<int>& heads,
    int vals,
    int directions
) {
    std::vector<std::vector<int>> updated_tapes = tapes;
    std::vector<int> updated_heads = heads;

    for (int i = 0; i < num_tapes; ++i) {
        std::vector<int>& tape = updated_tapes[i];
        int head = updated_heads[i];
        tape[head] = (vals >> i) & 1;
        int dir = (directions >> i) & 1;
        if (dir == 0) {
            dir = -1;
        }
        head += dir;
        if (head < 0) {
            head = 0;
        }
        if (head >= tape.size()) {
            tape.push_back(0);
        }
        updated_tapes[i] = tape;
        updated_heads[i] = head;
    }

    return std::make_tuple(updated_tapes, updated_heads);
}

int emulate_TM(
    const std::unordered_map<std::pair<int, int>, std::tuple<int, int, int>, std::hash<std::pair<int, int>>>& TM,
    std::vector<std::vector<int>>& tapes,
    int t,
    bool flag,
    int inp
) {
    std::vector<int> heads(num_tapes, 0);
    int state = 0;

    std::vector<int> max_heads(num_tapes, 0);
    std::unordered_set<int> visited_states;
    visited_states.insert(state);

    for (int _ = 0; _ < t; ++_) {
        int head_content = read_from_tapes(tapes, heads);

        if (flag) {
            for (int i = 0; i < num_tapes; ++i) {
                max_heads[i] = std::max(max_heads[i], heads[i]);
            }
            visited_states.insert(state);
        }

        auto result = TM.find(std::make_pair(state, head_content));
        if (result == TM.end()) {
            // Handle missing transition.
            // You can add your own logic here.
        } else {
            std::tie(state, int vals, int directions) = result->second;
            std::tie(tapes, heads) = update_tapes(tapes, heads, vals, directions);
        }
    }

    if (flag) {
        if (max_heads[1] < 10) {
            std::cout << max_heads[0] << " " << max_heads[1] << " " << inp << std::endl;
        }
    }
    return (state & 1);
}

std::vector<int> load_tape(int inp) {
    std::vector<int> tape;
    while (inp != 0) {
        tape.push_back(inp & 1);
        inp >>= 1;
    }
    tape.push_back(0);
    return tape;
}
*/

void gen_random_pad(int random_pad[][PAD_LENGTH])
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> bin_dist(0, 1);
    for (int i = 0; i < NUM_TAPES; ++i)
        for (int j = 0; j < PAD_LENGTH; ++j)
            random_pad[i][j] = bin_dist(gen);
}

int tapes[NUM_TAPES][T + 10];


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