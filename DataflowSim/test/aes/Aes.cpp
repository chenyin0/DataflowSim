#include "./Aes.h"

using namespace DFSimTest;

// Performance parameter
// Base
uint AesTest::Base_speedup = 1;

// DGSF
uint AesTest::DGSF_non_branch_speedup = 2;
uint AesTest::DGSF_truePath_speedup = 5;
uint AesTest::DGSF_falsePath_speedup = 2;