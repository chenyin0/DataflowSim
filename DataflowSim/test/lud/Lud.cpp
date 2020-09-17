#include "./Lud.h"

using namespace DFSimTest;

vector<int> LudTest::matrix;
uint LudTest::matrix_size = 20;

uint LudTest::Base_outer_loop_speedup = 1;
uint LudTest::Base_inner_loop_speedup = 4;

uint LudTest::DGSF_outer_loop_speedup = 2;
uint LudTest::DGSF_inner_loop_speedup = 4;

uint LudTest::DGSF_outer_loop_buffer_size = 2;

void LudTest::generateData()
{
    matrix.resize(matrix_size * matrix_size);

    for (auto& i : matrix)
    {
        i = 1;
    }
}