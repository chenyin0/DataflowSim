#include "./Lud.h"

using namespace DFSimTest;

vector<int> LudTest::matrix;
uint LudTest::matrix_size = 20;

void LudTest::generateData()
{
    matrix.resize(matrix_size * matrix_size);

    for (auto& i : matrix)
    {
        i = 1;
    }
}