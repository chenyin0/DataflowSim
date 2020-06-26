#include "./Gemm.h"

using namespace DFSimTest;

const uint GemmTest::matrix_width = 10;
const uint GemmTest::matrix_height = matrix_width;
const uint GemmTest::block_size = 5;

// Address map: 
// 0x00 -> matrix_m1 -> matrix_m2 -> matrix_paritial
const uint GemmTest::m1_BaseAddr = 0;
const uint GemmTest::m2_BaseAddr = matrix_width * matrix_height;
const uint GemmTest::partialSum_BaseAddr = matrix_width * matrix_height * 2;

vector<vector<int>> GemmTest::m1;
vector<vector<int>> GemmTest::m2;
vector<vector<int>> GemmTest::result;


void GemmTest::generateData()
{
    uint size = matrix_width * matrix_height;
    m1.resize(matrix_height);
    m2.resize(matrix_height);
    result.resize(matrix_height);

    for (size_t i = 0; i < matrix_height; ++i)
    {
        m1[i].resize(matrix_width);
        m2[i].resize(matrix_width);
        result[i].resize(matrix_width);

        for (size_t j = 0; j < matrix_width; ++j)
        {
            m1[i][j] = i * matrix_width + j;
            m2[i][j] = i * matrix_width + j;
        }
    }
}