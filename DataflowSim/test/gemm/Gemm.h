#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/execution/Lc.h"
#include "../../src/module/execution/Mux.h"
#include "../../src/module/ClkSys.h"
#include "../../src/sim/Debug.h"

/*  gemm src code 
	from MachSuite, blocked

void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
	int i, k, j, jj, kk;
	int i_row, k_row;
	TYPE temp_x, mul;

	loopjj:for (jj = 0; jj < row_size; jj += block_size){
		loopkk:for (kk = 0; kk < row_size; kk += block_size){
			loopi:for ( i = 0; i < row_size; ++i){
				loopk:for (k = 0; k < block_size; ++k){
					i_row = i * row_size;
					k_row = (k  + kk) * row_size;
					temp_x = m1[i_row + k + kk];
					loopj:for (j = 0; j < block_size; ++j){
						mul = temp_x * m2[k_row + j + jj];
						prod[i_row + j + jj] += mul;
					}
				}
			}
		}
	}
}

*/

namespace DFSimTest
{
	using namespace DFSim;

	class GemmTest
	{
	public:
		static void gemm_base(Debug* debug);
	
	private:
		static void generateData();  // Generate benchmark data

		static const uint matrix_width;
		static const uint matrix_height;
		static const uint block_size;

		static vector<vector<int>> m1;
		static vector<vector<int>> m2;
		static vector<vector<int>> result;
	};

//	void GemmTest::generateData() 
//	{
//		uint size = matrix_width * matrix_height;
//		m1.resize(matrix_height);
//		m2.resize(matrix_height);
//		result.resize(matrix_height);
//
//		for (size_t i = 0; i < matrix_height; ++i)
//		{
//			m1[i].resize(matrix_width);
//			m2[i].resize(matrix_width);
//			result.resize(matrix_width);
//
//			for (size_t j = 0; j < matrix_width; ++i)
//			{
//				m1[i][j] = i * matrix_width + j;
//				m2[i][j] = i * matrix_width + j;
//			}
//		}
//	}
}