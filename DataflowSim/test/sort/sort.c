#include "sort.h"

void merge(TYPE a[SIZE], int start, int m, int stop){
    TYPE temp[SIZE];
    int i, j, k;

    merge_label1 : for(i=start; i<=m; i++){
        temp[i] = a[i];
    }

    merge_label2 : for(j=m+1; j<=stop; j++){
        temp[m+1+stop-j] = a[j];
    }

    i = start;
    j = stop;

    merge_label3 : for(k=start; k<=stop; k++){
        TYPE tmp_j = temp[j];
        TYPE tmp_i = temp[i];
        if(tmp_j < tmp_i) {
            a[k] = tmp_j;
            j--;
        } else {
            a[k] = tmp_i;
            i++;
        }
    }
}

// void ms_mergesort(TYPE a[SIZE]) {
//     int start, stop;
//     int i, m, from, mid, to;

//     start = 0;
//     stop = SIZE;

//     mergesort_label1 : for(m=1; m<stop-start; m+=m) {
//         mergesort_label2 : for(i=start; i<stop; i+=m+m) {
//             from = i;
//             mid = i+m-1;
//             to = i+m+m-1;
//             if(to < stop){
//                 merge(a, from, mid, to);
//             }
//             else{
//                 merge(a, from, mid, stop);
//             }
//         }
//     }
// }

void ms_mergesort(TYPE a[SIZE]) {
    int start, stop;
    int i, m, from, mid, to;

    for(m = 1; m < SIZE; m += m) 
    {
        for(i = 0; i < SIZE; i += m + m) 
        {
            m_m = m + m;
            i_m_m = i + m_m;
            //mid = i_m-1;
            to = i_m_m-1;
            if(to < SIZE)
            {
                // merge(a, from, mid, to);              
                for(k1=i; k1<=to; k1++)
                {
                    tmp_j1 = temp[to];
                    tmp_i1 = temp[i];
                    if(tmp_j1 < tmp_i1)
                    {
                        a[k1] = tmp_j1;
                        to--;
                    } 
                    else 
                    {
                        a[k1] = tmp_i1;
                        i++;
                    }
                }
            }
            else
            {
                s = SIZE;
                // merge(a, from, mid, stop);
                for(k2=i; k2<=SIZE; k2++)
                {
                    tmp_j2 = temp[s];
                    tmp_i2 = temp[i];
                    if(tmp_j2 < tmp_i2)
                    {
                        a[k2] = tmp_j2;
                        s--;
                    } 
                    else 
                    {
                        a[k2] = tmp_i2;
                        i++;
                    }
                }
            }
        }
    }
}