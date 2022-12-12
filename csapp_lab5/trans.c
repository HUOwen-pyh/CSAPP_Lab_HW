/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	if(M!=32||N!=32)return;
	for(int i = 0;i<4;i++)for(int j = 0;j<4;j++){
		if(i==j)continue;
		int idxi = (i<<3), idxj = (j<<3);
		for(int p = 0;p<8;p++)for(int q = 0;q<8;q++){
			B[idxj+q][idxi+p] = A[idxi+p][idxj+q];
		}
	}
	for(int i = 0;i<4;i++){
		int idx = (i<<3);
		for(int j = 0;j<8;j++){
			int t0 = A[idx+j][idx];
			int t1 = A[idx+j][idx+1];
			int t2 = A[idx+j][idx+2];
			int t3 = A[idx+j][idx+3];
			int t4 = A[idx+j][idx+4];
			int t5 = A[idx+j][idx+5];
			int t6 = A[idx+j][idx+6];
			int t7 = A[idx+j][idx+7];
			B[idx][idx+j] = t0;
			B[idx+1][idx+j] = t1;
			B[idx+2][idx+j] = t2;
			B[idx+3][idx+j] = t3;
			B[idx+4][idx+j] = t4;
			B[idx+5][idx+j] = t5;
			B[idx+6][idx+j] = t6;
			B[idx+7][idx+j] = t7;
		}
	}
	return;

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

