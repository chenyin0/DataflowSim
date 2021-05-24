// void lud_base(float *a, int size)
// {
//     int i,j,k;
//     float sum;

//     for (i=0; i<size; i++)
//     {
//         for (j=i; j<size; j++)
//         {
//             sum=a[i*size+j];  // 2 pe
//             for (k=0; k<i; k++) 
//                 sum -= a[i*size+k]*a[k*size+j];  // 6 pe
//             a[i*size+j]=sum;
//         }

//         for (j=i+1;j<size; j++)
//         {
//             sum=a[j*size+i];  // 2 pe
//             for (k=0; k<i; k++) 
//                 sum -=a[j*size+k]*a[k*size+i];  // 6 pe
//             a[j*size+i]=sum/a[i*size+i];  // 3 pe 
//         }
//     }
// }

void lud_base(float *a, int size)
{
    int i,j,k;
    float sum;

    for (i=0; i<size; i++)
    {
        i1 = i;  // Fake node
        i2 = i;  // Fake node
        for (j=i1; j<size; j++)
        {
            i_size = i1 * size;
            i_size_j = i_size + j;
            sum=a[i_size_j];  // 2 pe
            for (k=0; k<i1; k++)
            {
                i_size_k = i_size + k;
                k1_size = k * size;
                k_size_j = k_size + j;
                a1 = a[i_size_k];
                a2 = a[k_size_j];
                a1_a2 = a1 * a2;
                sum = sum - a1_a2;  // 6 pe
            } 
            a[i_size_j] = sum;
        }

        for (j=i2+1;j<size; j++)
        {
            j_size = j*size;
            j_size_i = j_size + i2;
            sum=a[j_size_i];  // 2 pe
            for (k=0; k<i2; k++)
            {
                j_size_k = j_size +k;
                k2_size = k * size;
                k_size_i = k_size + i2;
                a3 = a[j_size_k];
                a4 = a[k_size_i];
                a3_a4 = a3 * a4;
                sum = sum - a3_a4;  // 6 pe
            } 

            i_size_ = i2 * size;
            i_size_i = i_size_ + i2;
            a[j_size_i]=sum/a[i_size_i];  // 3 pe 
        }
    }
}

