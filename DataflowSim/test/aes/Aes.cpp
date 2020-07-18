#include "./Aes.h"

using namespace DFSimTest;

uint AesTest::segment_size = 100;

// Performance parameter
// Base
uint AesTest::Base_speedup = 1;

// DGSF
uint AesTest::DGSF_non_branch_speedup = 1;
uint AesTest::DGSF_truePath_speedup = 10;
uint AesTest::DGSF_falsePath_speedup = 5;