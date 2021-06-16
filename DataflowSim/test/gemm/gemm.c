/*
Implementation based on algorithm described in:
The cache performance and optimizations of blocked algorithms
M. D. Lam, E. E. Rothberg, and M. E. Wolf
ASPLOS 1991
*/

#include "gemm.h"

void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
    int i, k, j, jj, kk;
    int i_row, k_row;
    TYPE temp_x, mul;
    int k_kk;
    int j_jj;
    int m2_data;
    int m1_addr;
    int m2_addr;
    int prod_addr;
    int m1_data;

    // Loop 0
    loopjj:for (jj = 0; jj < row_size; jj += block_size){
        // Loop 1
        loopkk:for (kk = 0; kk < row_size; kk += block_size){
            // Loop 2
            loopi:for ( i = 0; i < row_size; ++i){
                i_row = i * row_size;
                // Loop 3
                loopk:for (k = 0; k < block_size; ++k){
                    //i_row = i * row_size;
                    // 5 inst(5 outer-loop)
                    k_kk = k + kk;
                    k_row = k_kk * row_size;
                    m1_addr = i_row + k_kk;
                    temp_x = m1[m1_addr];
                    m1_data = temp_x;
                    // Loop 4; 6 inst
                    loopj:for (j = 0; j < block_size; ++j){
                        j_jj = j + jj;
                        m2_addr = k_row + j_jj;
                        m2_data = m2[m2_addr];
                        mul = m1_data * m2_data;
                        prod_addr = i_row + j_jj;
                        prod[prod_addr] += mul;
                    }
                }
            }
        }
    }
}


// void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
//     int i, k, j, jj, kk;
//     int i_row, k_row;
//     TYPE temp_x, mul;

//     loopjj:for (jj = 0; jj < row_size; jj += block_size){
//         loopkk:for (kk = 0; kk < row_size; kk += block_size){
//             loopi:for ( i = 0; i < row_size; ++i){
//                 loopk:for (k = 0; k < block_size; ++k){
//                     i_row = i * row_size;
//                     k_row = (k  + kk) * row_size;
//                     temp_x = m1[i_row + k + kk];
//                     loopj:for (j = 0; j < block_size; ++j){
//                         mul = temp_x * m2[k_row + j + jj];
//                         prod[i_row + j + jj] += mul;
//                     }
//                 }
//             }
//         }
//     }
// }