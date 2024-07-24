#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <bitset>
#include <cstring>

#include "api.hpp"

using namespace std;

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

std::vector<bit> get_TM_tt(int table_length = 65536) { // defualt value is case dependent
    std::vector<bit> tt = {};

    for (int inp = 0; inp < table_length; ++inp) {
        tt.push_back(emulate_TM(TM, inp));
    }

    return tt;
}

std::vector<bit> apply_RS(std::vector<bit> msg) {
   constexpr std::size_t n = (RS_q - 1);
   constexpr std::size_t d = 32;
   constexpr std::size_t k = n - d + 1;

   /* Finite Field Parameters */
   constexpr std::size_t field_descriptor                = log_RS_q;
   constexpr std::size_t generator_polynomial_index      = 0;
   constexpr std::size_t generator_polynomial_root_count = d; // n - k + 1; // n = q - 1, d = n - k + 1 

   /* Reed Solomon Code Parameters */
   constexpr std::size_t code_length = n;
   constexpr std::size_t fec_length  = generator_polynomial_root_count;
   constexpr std::size_t data_length = code_length - fec_length;

   /* Instantiate Finite Field and Generator Polynomials */
   const schifra::galois::field field
                                (
                                   field_descriptor,
                                   schifra::galois::primitive_polynomial_size06,
                                   schifra::galois::primitive_polynomial06
                                );

   schifra::galois::field_polynomial generator_polynomial(field);

   if (
        !schifra::make_sequential_root_generator_polynomial
        (
           field,
           generator_polynomial_index,
           generator_polynomial_root_count,
           generator_polynomial
        )
      )
   {
      std::cout << "Error - Failed to create sequential root generator!" << std::endl;
   }

   /* Instantiate Encoder and Decoder (Codec) */
   typedef schifra::reed_solomon::encoder<code_length,fec_length,data_length> encoder_t;

   const encoder_t encoder(field, generator_polynomial);
    
    /* Instantiate RS Block For Codec */
    schifra::reed_solomon::block<code_length,fec_length> block;
    
    for (int i = 0; i < k; ++i) {
        int curr = 0;
        for (int j = 0; j < log_RS_q; ++j) {
            curr += ((msg[i * log_RS_q + j].val & 1) << j);
        }
        block[i] = static_cast<int>(curr & BIT_PREFIX_MASK(field_descriptor));
    }
    
   /* Transform message into Reed-Solomon encoded codeword */
   if (!encoder.encode(block))
   {
      std::cout << "Error - Critical encoding failure!" << std::endl;
   }

    std::vector<bit> code_word(0);

   for (int field_element_index = 0; field_element_index < code_length; ++field_element_index) {
        for (int i = 0; i < log_RS_q; ++i) {
            code_word.push_back(bit((static_cast<int>(block[field_element_index]) >> i) & 1));
        }
   }

   return code_word;
}

std::vector<bit> apply_hadamard(std::vector<bit> tt) {
    int table_length = tt.size();    
    
    std::vector<bit> hadamard(0);
    for (int i = 0; i < table_length / log_RS_q; ++i) {

        for (int v = 0; v < RS_q; ++v) {
            int inn_prod = 0;
            
            // std::cout << v << " * tt[] = ";
            for (int index = 0; index < log_RS_q; ++index) {
                // std::cout <<(( v >> index) & 1 ) << "*" << (int)tt[(i + 1) * log_q - index - 1] << " ";
                inn_prod += ( (( v >> index) & 1 ) * (tt[(i + 1) * log_RS_q - index - 1].val & 1) );
            }

            // std::cout << " = " << (inn_prod & 1) << std::endl;
            hadamard.push_back(bit(inn_prod & 1)); 
        }

    }
    
    return hadamard;
}

/*
    Treat 'inp' as an array of w k-bit long input strings, and takes the inner product of the result of the TM of those
    with the next w bits of 'inp' (r)
*/
bit locally_encode_explicit_calc(long long inp) {
    static constexpr int w = 4;
    static constexpr int k = (l/w) - 1; // w*k + w = design's l

    int r = (inp >> (w*k)) & BIT_PREFIX_MASK(w);
    int inner_prod = 0;

    for (int i = 0; i < w; ++i) {
        inner_prod += ( ( r >> i ) & 1) ? (emulate_TM(TM, (inp >> (i * k)) & BIT_PREFIX_MASK(k)).val & 1) : 0;
    }

    return bit(inner_prod & 1);
}

int main() {
    //int out = dup(stdout);
    freopen("output.txt", "w", stdout);

    freopen("./../NaiveUTM/NW_inputs_2.txt", "r", stdin);

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
    printf("%ld\n", ecc_tt.size());
    while (cin >> inp)
        // outputs.push_back(locally_encode_explicit_calc(inp));
        outputs.push_back(ecc_tt[inp]);
        // outputs.push_back(emulate_TM(TM, inp));
    printf("D\n");
    fflush(stdout);
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
    */

    return 0;
}
