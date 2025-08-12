#include "toom_cook3_code.h"
#include "schoolbook_avx2.h"

uint16_t int_pow(uint16_t base, int exp) {
    uint16_t result = 1;
    while (exp > 0) {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return result;
}

uint16_t *toom_cook_n_n(uint16_t *A,uint16_t *B, uint16_t *C,int size, FILE *fp){

    // fprintf(fp,"size = %d\n",size);
    // printf("size = %d\n",size);
    //check_size
    if (size%3 != 0) //this time size == 32
    {   
        multiple_32_32(A,B,C,size);
        return 0;
    }

    //step1: splite 4 part
    // printf("step 1 : \n");
    // fprintf(fp,"step1 : splite 3 part\n");
    uint16_t **A_1 = (uint16_t **)malloc(3*sizeof(uint16_t*));
    uint16_t **B_1 = (uint16_t **)malloc(3*sizeof(uint16_t*));
    for (int i = 0; i < 3; i++)
    {
        A_1[i] = (uint16_t*)calloc(size/3,sizeof(uint16_t));
        B_1[i] = (uint16_t*)calloc(size/3,sizeof(uint16_t));
        for (int j = 0; j < (size/3); j++)
        {
            A_1[i][j] = A[i * (size/3) + j];      //save pointer of index 0 , 128 , 256 , 384
            B_1[i][j] = B[i * (size/3) + j]; 
            // printf("A_1[%3d][%3d] = %5p , B_1[%3d][%3d] = %5p\n",i,j , &A_1[i][j], i,j , &B_1[i][j]);
            // printf("A_1[%3d][%3d] = %p , A[%3d] = %p\n",i,j,&A_1[i][j],i*(size/4)+j, &A[i*(size/4)+j]);
        }
    }
    // for (int i = 0; i < 3; i++)
    // {
    //     for (int j = 0; j < (size/3); j++)
    //     {
    //         // printf("size of A_1[%3d][%3d] = %d\n",i,j,sizeof(A_1[i][j]));
    //         // printf("       A[%3d] = %p ,        B[%3d] = %p\n",i*(size/4)+j , &A[i*(size/4)+j], i*(size/4)+j , &B[i*(size/4)+j]);
    //         // printf("A_1[%3d][%3d] = %5p , B_1[%3d][%3d] = %5p\n",i,j , &A_1[i][j], i,j , &B_1[i][j]);            
    //         printf("A_1[%3d][%3d] = %5d , B_1[%3d][%3d] = %5d\n",i,j , A_1[i][j], i,j , B_1[i][j]);
    //         // fprintf(fp,"A_1[%3d][%3d] = %5d , B_1[%3d][%3d] = %5d\n",i,j , A_1[i][j], i,j , B_1[i][j]);
    //     }
    //     printf("------------------------------------------------------------------------\n\n");
    //     // fprintf(fp,"------------------------------------------------------------------------\n\n");
    // }

    // //step2: evalution at 7 point {0,1,-1,-2,∞}
    // printf("step 2 :\n");
    // fprintf(fp,"step2 : evalution\n");
    int e_point[] = {0,1,-1,-2};
    uint16_t **X_A = (uint16_t **)malloc(5*sizeof(uint16_t*));
    uint16_t **X_B = (uint16_t **)malloc(5*sizeof(uint16_t*));
    for (int i = 0; i < 5; i++)
    {
        // printf("e_point = %d\n",e_point[i]);
        X_A[i]= (uint16_t *)calloc(size/3,sizeof(uint16_t));
        X_B[i] = (uint16_t *)calloc(size/3,sizeof(uint16_t));

        //e = infinite
        if (i == 4)
        {
            for (int j = 0; j < size/3; j++)
            {
                X_A[i][j] += A_1[2][j];
                X_B[i][j] += B_1[2][j];
            }
            break;
        }

        for (int k = 0; k < 3; k++)
        {
            uint16_t e = int_pow(e_point[i],k);
            for (int j = 0; j < size/3; j++)
            {                
                X_A[i][j] += A_1[k][j]*e;
                X_B[i][j] += B_1[k][j]*e;
            }
        }
    }
    // for (int i = 0; i < 5; i++)
    // {
    //     if (i == 4)
    //     {
    //         printf("e = infinite \n");
    //         // fprintf(fp,"e = infinite \n");
    //     }else{   
    //         // fprintf(fp,"e = %d \n",e_point[i]);
    //         printf("e = %d \n",e_point[i]);
    //     }
    //     for (int j = 0; j < size/3; j++)
    //     {
    //         // printf("size of X_A = %d\n ",sizeof(X_A[i][j]));
    //         // fprintf(fp,"X_A[%3d][%3d] = %15d , X_B[%3d][%3d] = %15d\n",i,j,X_A[i][j],i,j,X_B[i][j]);
    //         printf("X_A[%3d][%3d] = %11u = ",i,j,X_A[i][j]); print_binary16(X_A[i][j],16);
    //         printf("X_B[%3d][%3d] = %11u = ",i,j,X_B[i][j]); print_binary16(X_B[i][j],16); 
    //         printf("\n");
    //         // printf("X_A[%3d][%3d] = %p , X_B[%3d][%3d] = %p \n",i,j,&X_A[i][j], i,j,&X_B[i][j]);
    //     }
    //     printf("------------------------------------------------------------\n\n");
    //     // fprintf(fp,"------------------------------------------------------------------------\n\n");
        
    // }

    //step3; point multiple
    // printf("step 3 : \n");
    // fprintf(fp,"step3 : point multiple\n");
    uint16_t **r = (uint16_t **)malloc(5*sizeof(uint16_t *));
    for (int i = 0; i < 5; i++)
    {
        uint16_t *tmp = (uint16_t *)calloc(2*(size/3)-1,sizeof(uint16_t));
        toom_cook_n_n(X_A[i],X_B[i],tmp,size/3,fp);
        // r[i] = &tmp[0];
        r[i] = (uint16_t *)calloc(2*(size/3)-1,sizeof(uint16_t));
        for (uint16_t j = 0; j < 2*(size/3)-1; j++)
        {
            r[i][j] = tmp[j];
            // fprintf(fp,"r[%3d][%3d] = %5lld\n",i,j,r[i][j]);
            // // printf("size of r[%d][%d] = %d\n",i,j,sizeof(r[i][j]));
        }
        // fprintf(fp,"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    }
    // for (int i = 0; i < 5; i++)
    // {
    //     for (int j = 0; j < 2*(size/3)-1; j++)
    //     {
    //         // fprintf(fp,"r[%3d][%3d] = %10lld \n",i,j,r[i][j]);
    //         printf("r[%3d][%3d] = %10d = ",i,j,r[i][j]); print_binary16(r[i][j], 16);
    //         // printf("r in 16 bit = %10u = ",(uint16_t)r[i][j]); print_binary16(r[i][j] , 16);
    //     }
    //     // fprintf(fp,"---------------------------------------------\n");
    //     printf("---------------------------------------------\n");
    // }
    
    //step4: interpolation
    // printf("step 4 : \n");
    // fprintf(fp,"step4 : interpolation\n");
    uint16_t **C_1 = (uint16_t **)malloc((2*(size/3) -1)*sizeof(uint16_t *));       //save value from each coefficient of r[i]
    // printf("sizeof(r) = %d , sizeof(r[0]) = %d\n",sizeof(r), sizeof(r[0]));
    for (int i = 0; i <(2*(size/3) -1) ; i++)
    {

        // printf("i = %d\n",i);
        C_1[i] = (uint16_t*)calloc(5,sizeof(uint16_t));

        //c0
        C_1[i][0] = r[0][i];
        // printf("c0   = %11d = ",C_1[i][0]); print_binary16(C_1[i][0],16);

        //c1 11bit ok
        // uint16_t c1_1 = (3*r[0][i] + 2*r[1][i] - 6*r[2][i] + r[3][i] - 12*r[4][i]) & (uint16_t)n-1;
        // print_binary16(3*r[0][i] + 2*r[1][i] - 6*r[2][i] + r[3][i] - 12*r[4][i],32);
        // print_binary16(c1_1*683,32);  
        // print_binary16((3*r[0][i] + 2*r[1][i] - 6*r[2][i] + r[3][i] - 12*r[4][i])*683,32); // check *683

        C_1[i][1] = ((3*r[0][i] + 2*r[1][i] - 6*r[2][i] + r[3][i] - 12*r[4][i])*43691)>>1;
        // printf("c1   = %11d = ",C_1[i][1]); print_binary16(C_1[i][1],16);

        //c2
        C_1[i][2] = (-2*r[0][i] + r[1][i] + r[2][i] -2*r[4][i])>>1;
        // printf("c2   = %11d = ",C_1[i][2]); print_binary16(C_1[i][2],16);

        //c3
        C_1[i][3] = ((-3*r[0][i] + r[1][i] + 3*r[2][i] -r[3][i] + 12*r[4][i])*43691)>>1;
        // printf("c3   = %11d = ",C_1[i][3]); print_binary16(C_1[i][3],16);

        //c4
        C_1[i][4] = r[4][i];
        // printf("c4   = %11d = ",C_1[i][4]); print_binary16(C_1[i][4],16);

        // printf("----------------------------------------------------------------------------\n");
    }
    
    // //step5: recomposition
    // printf("step 5 : \n");
    // fprintf(fp,"step5 : recomposition\n");
    for (int i = 0; i < (2*(size/3)-1); i++)
    {
        for (int j = 0; j < 5; j++)
        {
            // if ((int)(C_1[i][j]*10)%10 >= 5)
            // {
            //     C[(size/4)*j + i] += (int)(C_1[i][j]) +1;
            // }else{
            //     C[(size/4)*j + i] += (int)(C_1[i][j]);
            // }
            C[(size/3)*j+i] = C[(size/3)*j+i]+C_1[i][j] ;
            // printf("C_1[%3d][%3d] = %5d\n",i,j,(int)(C_1[i][j]));
            // C[(size/4)*j + i] += (C_1[i][j]);
            // printf("C[%3d] = %5d \n",(size/4)*j + i , C[(size/4)*j + i]);
        }
    }


    // fprintf(fp,"########################################################################\n\n");
    // fprintf(fp,"result = \n");
    // for (int i = 0; i < (2*(size)-1); i++)
    // {
    //     fprintf(fp,"C[%5d] = %10d \n",i , C[i]);
    //     // printf("C[%5d] = %10lld \n",i , C[i]);
    // }
    // fprintf(fp,"########################################################################\n\n");
    
    free(A_1);
    free(B_1);
    free(X_A);
    free(X_B);
    free(r);
    free(C_1);

}