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
    for (int node = 0; node < N; ++node)
        for (int idx = 0; idx < (1 << NUM_TAPES); ++idx)
        {
            int next_node = randIntMod(N), // next state
                vals = randIntMod(1 << NUM_TAPES), // new value per tape
                dirs = randIntMod(1 << NUM_TAPES); // movment direction per tape
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

std::vector<bit> get_TM_tt(int table_length, bool printProgress) {
    std::vector<bit> tt = {};

    for (int inp = 0; inp < table_length; ++inp) {
        if (printProgress && (! (inp % (1 + (int)table_length/100))))
            printf("%d%%\n", (int)(100*inp)/table_length);

        tt.push_back(emulate_TM(TM, inp));
    }

    return tt;
}