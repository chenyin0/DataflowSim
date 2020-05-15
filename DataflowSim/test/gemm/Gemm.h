#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
//#include "../src/module/mem/Spm.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/ClkSys.h"
//#include "../src/module/execution/Lc.h"
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

namespace DFSim
{
	class GemmTest
	{
	public:
		static void gemm_base(Debug* debug);
	
	private:
		static void generateData();  // Generate benchmark data

		static const uint matrix_width = 500;
		static const uint matrix_height = matrix_width;

		static vector<int> m1;
		static vector<int> m2;
		static vector<int> result;
	};

	void GemmTest::generateData() 
	{
		uint size = matrix_width * matrix_height;
		m1.resize(size);
		m2.resize(size);
		result.resize(size);

		for (size_t i = 0; i < size; ++i)
		{
			m1[i] = i;
			m2[i] = i;
		}
	}
}