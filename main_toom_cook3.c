#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

//import
#include "toom_cook3_code.h"


#define q 508
#define n pow(2,14)


//show binary 
void print_binary16(uint32_t val, int size) {
    for (int i = size-1; i >= 0; i--) {
        printf("%d", (val >> i) & 1);
        if (i%4 == 0 && i != 0)
        {
            printf("-");
        }
        if (i == 16)
        {
            printf("   ");
        }
    }
    printf("\n");
}

//mod
uint16_t mod(int32_t val) {
    int32_t result = val % (int32_t)n;
    return (uint16_t)((result < 0) ? result + (int32_t)n : result);
}

// Extended Euclidean algorithm to find inverse of a mod m
uint16_t modinv(uint16_t a, uint16_t m) {
    int m0 = m, t, q_1;
    int x0 = 0, x1 = 1;

    if (m == 1)
        return 0;

    while (a > 1) {
        q_1 = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q_1 * x0;
        x1 = t;
    }

    // Make x1 positive
    if (x1 < 0)
        x1 += m0;

    return x1;
}


int main(){

    //toom-cook3_5
    
    FILE *fp = fopen("data_save.csv","r");
    
    srand(time(0));
    //check size
    int size = 288; //2592 -> 864 -> 288 -> 96 -> 32
    // int size = q;
    // if (size % 8 != 0)
    // {
    //     size += size%8;
    // }

    //create A ,B ,C array 
    uint16_t *A = (uint16_t *)calloc(size,sizeof(uint16_t));
    uint16_t *B = (uint16_t *)calloc(size,sizeof(uint16_t));
    uint16_t *C = (uint16_t *)calloc(2*size, sizeof(uint16_t));


    //read same data from csv
    //each coefficient of array A and B in mod 2048
    // for (int i = 0; i < size; i++) {
    //     fscanf(fp, "%d,%d", &A[i], &B[i]);
    //     // printf("A[%4d] = %5d , B[%4d] = %5d \n",i,A[i],i,B[i]);

    // }

    
    //random coefficient
    for (int i = 0; i < size; i++)
    {
        // A[i] = rand()%(int)(n -1);
        // B[i] = rand()%(int)(n -1);
        A[i] = i+1;
        B[i] = 1;
        // printf("A[%4d] = %5d , B[%4d] = %5d \n",i,A[i],i,B[i]);
    }

    clock_t start , end;
    clock_t start_1, end_1;

    //call toom-cook3

    start = clock();
    toom_cook_n_n(A,B,C,size,fp);
    end = clock();
    for (int i = 0; i < 2*size -1; i++)
    {
        C[i] = C[i] & (uint16_t)n-1;
    }
    
    //clock time
    printf("use toom-cook3 function = %0.20f\n",((double) (end - start)) / CLOCKS_PER_SEC);


    //normal multiple
    int *D = (int*)calloc(2*size -1 , sizeof(int));
    start_1 = clock();
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            D[i+j] += A[i]*B[j];
        }
    }
    end_1 = clock();
    //clock time
    printf("use normal function     = %0.20f\n",((double) (end_1 - start_1)) / CLOCKS_PER_SEC);

    printf("\n------------------------------------------------------------\n");

    // print result
    printf("        toom-cook4             normal           \n");
    for (int i = 0; i < (2*size); i++)
    {
        printf("C[%5d] = %10d , = ",i,C[i]); print_binary16(C[i],16);
        printf("D[%5d] = %10d , = ",i,D[i] & (uint16_t)n-1); print_binary16(D[i]&(uint16_t)n-1,16);
    }

    
    //print check C == D yes or no
    for (int i = 0; i < (2*size)-1; i++)
    {
        if (C[i] != (D[i] & (uint16_t)n-1))
        {
            printf("C != D\n");
            printf("i = %d\n",i);
            break;
        }
    }

    
    
    fclose(fp);
    free(A);
    free(B);
    free(C);
    free(D);    

    return 0;
}