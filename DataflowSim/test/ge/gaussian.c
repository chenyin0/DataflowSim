#include<stdio.h>
int main()
{
    int i,j,k,n;
    float A[20][20],c,x[10],sum=0.0;
    printf("\nEnter the order of matrix: ");
    scanf("%d",&n);
    printf("\nEnter the elements of augmented matrix row-wise:\n\n");
    for(i=1; i<=n; i++)
    {
        for(j=1; j<=(n+1); j++)
        {
            printf("A[%d][%d] : ", i,j);
            scanf("%f",&A[i][j]);
        }
    }
    
    // for(j=1; j<=n; j++) /* loop for the generation of upper triangular matrix*/
    // {
    //     for(i=1; i<=n; i++)
    //     {
    //         if(i>j)
    //         {
    //             c=A[i][j]/A[j][j];
    //             for(k=1; k<=n+1; k++)
    //             {
    //                 A[i][k]=A[i][k]-c*A[j][k];
    //             }
    //         }
    //     }
    // }
    // x[n]=A[n][n+1]/A[n][n];
    // /* this loop is for backward substitution*/
    // for(i=n-1; i>=1; i--)
    // {
    //     sum=0;
    //     for(j=i+1; j<=n; j++)
    //     {
    //         sum=sum+A[i][j]*x[j];
    //     }
    //     x[i]=(A[i][n+1]-sum)/A[i][i];
    // }

for(t=1;t<=1;t++)
{
    for(j=1; j<=n; j++) /* loop for the generation of upper triangular matrix*/
    {
        j_n = j*n;
        addr_jj = j_n + j;
        for(i=j+1; i<=n; i++)
        {
            i_n = i*n;
            addr_ij = i_n+j;
            c=A[addr_ij]/A[addr_jj];
            for(k=1; k<=n+1; k++)
            {
                addr_ik = i_n+k;
                addr_jk = j_n+k;
                A[addr_ik]=A[addr_ik]-c*A[addr_jk];
            }
        }
    }
    //x[n]=A[n][n+1]/A[n][n];  // x[n] is a constant. It can be figured out in the preprocess
    /* this loop is for backward substitution*/
    for(i1=n-1; i1>=1; i1--)
    {
        sum=0;
        i1_n = i1*n;
        addr_i1n1 = i1_n + n+1;
        addr_i1i1 = i1_n + i1;
        for(j1=i1+1; j1<=n; j1++)
        {
            addr_i1j1 = i1_n+j1;
            sum=sum+A[addr_i1j1]*x[j1];
        }
        a_sum = A[addr_i1n1]-sum;
        x[i1] = a_sum/A[addr_i1i1];
    }
}
    // printf("\nThe solution is: \n");
    // for(i=1; i<=n; i++)
    // {
    //     printf("\nx%d=%f\t",i,x[i]); /* x1, x2, x3 are the required solutions*/
    // }
    return(0);
}

