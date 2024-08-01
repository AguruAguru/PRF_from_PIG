#include "api.hpp"

std::vector<bit> apply_RS(std::vector<bit> msg) {
    constexpr std::size_t n = (RS_q - 1);
    constexpr std::size_t d = RS_d; // TODO: select dynamically
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
    const schifra::galois::field *field = getGFOverF2(field_descriptor);                                

    schifra::galois::field_polynomial generator_polynomial(*field);

    if (
            !schifra::make_sequential_root_generator_polynomial
            (
            *field,
            generator_polynomial_index,
            generator_polynomial_root_count,
            generator_polynomial
            )
        )
    {
        std::cout << "Error - Failed to create sequential root generator!" << std::endl;
    }

    /* Instantiate Encoder (Codec) */
    typedef schifra::reed_solomon::encoder<code_length,fec_length,data_length> encoder_t;

    const encoder_t encoder(*field, generator_polynomial);
        
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

    delete field;
    return code_word;
}

std::vector<bit> apply_hadamard(std::vector<bit> tt) {
    int table_length = tt.size();    
    
    std::vector<bit> hadamard(0);
    for (int i = 0; i < table_length / log_RS_q; ++i) {

        for (int v = 0; v < RS_q; ++v) {
            int inn_prod = 0;
            
            for (int index = 0; index < log_RS_q; ++index) {
                inn_prod += ( (( v >> index) & 1 ) * (tt[(i + 1) * log_RS_q - index - 1].val & 1) );
            }

            hadamard.push_back(bit(inn_prod & 1)); 
        }

    }
    
    return hadamard;
}

/*
    Treat 'inp' as an array of w k-bit long input strings, and takes the inner product of the result of the TM of those
    with the next w bits of 'inp' (r)
*/
bit locally_encode_explicit_calc(uint64_t inp) {
    static constexpr int w = 6;
    static constexpr int k = (l/w) - 1; // w*k + w = design's l

    uint64_t r = (inp >> (w*k)) & BIT_PREFIX_MASK(w);
    int inner_prod = 0;

    for (int i = 0; i < w; ++i) {
        inner_prod += ( ( r >> i ) & 1) ? (emulate_TM(TM, (inp >> (i * k)) & BIT_PREFIX_MASK(k)).val & 1) : 0;
    }

    return bit(inner_prod & 1);
}