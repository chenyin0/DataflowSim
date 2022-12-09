#include "./Aes.h"

using namespace DFSimTest;

uint64_t AesTest::segment_size = 100;

// Performance parameter
// Base
uint64_t AesTest::Base_speedup = 1;

// DGSF
uint64_t AesTest::DGSF_non_branch_speedup = 1;
uint64_t AesTest::DGSF_truePath_speedup = 10;
uint64_t AesTest::DGSF_falsePath_speedup = 5;