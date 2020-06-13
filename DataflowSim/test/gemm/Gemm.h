#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/execution/Lc.h"
#include "../../src/module/execution/Mux.h"
#include "../../src/module/ClkSys.h"
#include "../../src/sim/Debug.h"
#include "../../src/module/Registry.h"

/*  Gemm src code 
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

/*
//************************
DFG analyze:
    DFG: no unrolling 

    Pe_lc = 10; (Lc = 2pe)
    PE_comp = 11;
        loop_4 pe = 5;
        loop_5 pe = 6;

    Ld = 3;
    St = 1;

    No-unroll:
        total pe = 21;

//*************************
Config parameter:
    Base: 
        Unroll loop_5 8 times:
        total pe = 63;

    SGMF:
        Unroll loop_5 2 times:
        total pe = 27;

    DGSF:
        loop_4: unroll 7 times;  (48pe - 4lc * 2)/5 = 8 times
        loop_5: unroll 8 times;

*/

namespace DFSimTest
{
    using namespace DFSim;

    class GemmTest
    {
    public:
        static void gemm_Base(Debug* debug);
        static void gemm_DGSF(Debug* debug);
    
    private:
        static void generateData();  // Generate benchmark data

        static const uint matrix_width;
        static const uint matrix_height;
        static const uint block_size;

        static vector<vector<int>> m1;
        static vector<vector<int>> m2;
        static vector<vector<int>> result;
    };
}