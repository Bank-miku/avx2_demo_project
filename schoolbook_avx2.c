#include "schoolbook_avx2.h"

//function
// AVX2: 16-bit lane version, correct modulo 2^14
void multiple_32_32(const uint16_t *a, const uint16_t *b, uint16_t *c, int size) {
    // c = 0
    memset(c, 0, (2*size) * sizeof(uint16_t));

    for (int i = 0; i < size; i++) {
        __m256i ai = _mm256_set1_epi16((short)a[i]); // broadcast 16 lanes

        // process B and C in 16-coefficient chunks
        for (int j = 0; j < size; j += 16) {
            // load 16 x uint16_t from B
            __m256i bj   = _mm256_loadu_si256((const __m256i*)&b[j]);       // b[j..j+15]
            // prod = (a[i] * b[j..j+15]) mod 2^16 (low 16 bits kept)
            __m256i prod = _mm256_mullo_epi16(ai, bj);

            // accumulate into c[i+j .. i+j+15] with 16-bit wrap
            __m256i acc  = _mm256_loadu_si256((__m256i*)&c[i + j]);
            acc = _mm256_add_epi16(acc, prod);  // wraps mod 2^16 (OK for final mod 2^14)
            _mm256_storeu_si256((__m256i*)&c[i + j], acc);
        }
    }

}