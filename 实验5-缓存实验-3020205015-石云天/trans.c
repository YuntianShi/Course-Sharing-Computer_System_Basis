/* 
 * trans.c - Matrik transpose B = A^T
 *
 * Each transpose function must have a prototspe of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated bs counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bstes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that sou
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identifs the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit( int M, int N, int A[ N ][ M ], int B[ M ][ N ] )
{
    int i, j, k, s, a0, a1, a2, a3, a4, a5, a6, a7;
    if ( M == 32 )
    {
        for ( i = 0; i < N; i += 8 )
        {
            for ( j = 0; j < N; j += 8 )
            {
                // 复制过程
                for ( k = i, s = j; k < i + 8; k++, s++ )
                {
                    a0 = A[ k ][ j + 0];
                    a1 = A[ k ][ j + 1 ];
                    a2 = A[ k ][ j + 2 ];
                    a3 = A[ k ][ j + 3 ];
                    a4 = A[ k ][ j + 4 ];
                    a5 = A[ k ][ j + 5 ];
                    a6 = A[ k ][ j + 6 ];
                    a7 = A[ k ][ j + 7 ];

                    B[ s ][ i + 0]     = a0;
                    B[ s ][ i + 1 ] = a1;
                    B[ s ][ i + 2 ] = a2;
                    B[ s ][ i + 3 ] = a3;
                    B[ s ][ i + 4 ] = a4;
                    B[ s ][ i + 5 ] = a5;
                    B[ s ][ i + 6 ] = a6;
                    B[ s ][ i + 7 ] = a7;
                }
                // 转置过程
                for ( k = 0; k < 8; k++ )
                {
                    for ( s = k + 1; s < 8; s++ )
                    {
                        a0                  = B[ k + j ][ s + i ];
                        B[ k + j ][ s + i ] = B[ s + j ][ k + i ];
                        B[ s + j ][ k + i ] = a0;
                    }
                }
            }
        }
    }
    else if ( M == 64 )
    {
        for ( i = 0; i < N; i += 8 )
        {
            // 先处理对角块
            if ( i == 0 )
                j = 8;
            else
                j = 0;
            // A块34区域放置C块12区域
            for ( k = 0; k < 4; k++ )
            {
                a0 = A[ i + k + 4 ][ i + 0 ];
                a1 = A[ i + k + 4 ][ i + 1 ];
                a2 = A[ i + k + 4 ][ i + 2 ];
                a3 = A[ i + k + 4 ][ i + 3 ];
                a4 = A[ i + k + 4 ][ i + 4 ];
                a5 = A[ i + k + 4 ][ i + 5 ];
                a6 = A[ i + k + 4 ][ i + 6 ];
                a7 = A[ i + k + 4 ][ i + 7 ];

                B[ i + k ][ j + 0 ] = a0;
                B[ i + k ][ j + 1 ] = a1;
                B[ i + k ][ j + 2 ] = a2;
                B[ i + k ][ j + 3 ] = a3;
                B[ i + k ][ j + 4 ] = a4;
                B[ i + k ][ j + 5 ] = a5;
                B[ i + k ][ j + 6 ] = a6;
                B[ i + k ][ j + 7 ] = a7;
            }
            // 对C块12区域进行转置
            for ( k = 0; k < 4; k++ )
            {
                for ( s = k + 1; s < 4; s++ )
                {
                    a0                  = B[ i + k ][ j + s ];
                    B[ i + k ][ j + s ] = B[ i + s ][ j + k ];
                    B[ i + s ][ j + k ] = a0;

                    a0                      = B[ i + k ][ j + s + 4 ];
                    B[ i + k ][ j + s + 4 ] = B[ i + s ][ j + k + 4 ];
                    B[ i + s ][ j + k + 4 ] = a0;
                }
            }
            // A块12区域放置B块12区域
            for ( k = 0; k < 4; k++ )
            {
                a0 = A[ i + k ][ i + 0 ];
                a1 = A[ i + k ][ i + 1 ];
                a2 = A[ i + k ][ i + 2 ];
                a3 = A[ i + k ][ i + 3 ];
                a4 = A[ i + k ][ i + 4 ];
                a5 = A[ i + k ][ i + 5 ];
                a6 = A[ i + k ][ i + 6 ];
                a7 = A[ i + k ][ i + 7 ];

                B[ i + k ][ i + 0 ] = a0;
                B[ i + k ][ i + 1 ] = a1;
                B[ i + k ][ i + 2 ] = a2;
                B[ i + k ][ i + 3 ] = a3;
                B[ i + k ][ i + 4 ] = a4;
                B[ i + k ][ i + 5 ] = a5;
                B[ i + k ][ i + 6 ] = a6;
                B[ i + k ][ i + 7 ] = a7;
            }
            // 对B块12区域进行转置
            for ( k = 0; k < 4; k++ )
            {
                for ( s = k + 1; s < 4; s++ )
                {
                    a0                  = B[ i + k ][ i + s ];
                    B[ i + k ][ i + s ] = B[ i + s ][ i + k ];
                    B[ i + s ][ i + k ] = a0;

                    a0                      = B[ i + k ][ i + s + 4 ];
                    B[ i + k ][ i + s + 4 ] = B[ i + s ][ i + k + 4 ];
                    B[ i + s ][ i + k + 4 ] = a0;
                }
            }
            // 交换B块2区域与C块1区域
            for ( k = 0; k < 4; k++ )
            {
                a0 = B[ i + k ][ i + 4 ];
                a1 = B[ i + k ][ i + 5 ];
                a2 = B[ i + k ][ i + 6 ];
                a3 = B[ i + k ][ i + 7 ];

                B[ i + k ][ i + 4 ] = B[ i + k ][ j + 0 ];
                B[ i + k ][ i + 5 ] = B[ i + k ][ j + 1 ];
                B[ i + k ][ i + 6 ] = B[ i + k ][ j + 2 ];
                B[ i + k ][ i + 7 ] = B[ i + k ][ j + 3 ];

                B[ i + k ][ j + 0 ] = a0;
                B[ i + k ][ j + 1 ] = a1;
                B[ i + k ][ j + 2 ] = a2;
                B[ i + k ][ j + 3 ] = a3;
            }
            // 把C块12区域放置B块34区域
            for ( k = 0; k < 4; k++ )
            {
                B[ i + k + 4 ][ i + 0 ] = B[ i + k ][ j + 0 ];
                B[ i + k + 4 ][ i + 1 ] = B[ i + k ][ j + 1 ];
                B[ i + k + 4 ][ i + 2 ] = B[ i + k ][ j + 2 ];
                B[ i + k + 4 ][ i + 3 ] = B[ i + k ][ j + 3 ];
                B[ i + k + 4 ][ i + 4 ] = B[ i + k ][ j + 4 ];
                B[ i + k + 4 ][ i + 5 ] = B[ i + k ][ j + 5 ];
                B[ i + k + 4 ][ i + 6 ] = B[ i + k ][ j + 6 ];
                B[ i + k + 4 ][ i + 7 ] = B[ i + k ][ j + 7 ];
            }
            // 非对角块处理
            for ( j = 0; j < M; j += 8 )
            {
                // 对角块跳过
                if ( i == j )
                {
                    continue;
                }
                // A块12区域处理
                for ( k = 0; k < 4; k++ )
                {
                    a0 = A[ j + k ][ i + 0 ];
                    a1 = A[ j + k ][ i + 1 ];
                    a2 = A[ j + k ][ i + 2 ];
                    a3 = A[ j + k ][ i + 3 ];
                    a4 = A[ j + k ][ i + 4 ];
                    a5 = A[ j + k ][ i + 5 ];
                    a6 = A[ j + k ][ i + 6 ];
                    a7 = A[ j + k ][ i + 7 ];

                    B[ i + 0 ][ j + k ]     = a0;
                    B[ i + 1 ][ j + k ]     = a1;
                    B[ i + 2 ][ j + k ]     = a2;
                    B[ i + 3 ][ j + k ]     = a3;
                    B[ i + 0 ][ j + k + 4 ] = a4;
                    B[ i + 1 ][ j + k + 4 ] = a5;
                    B[ i + 2 ][ j + k + 4 ] = a6;
                    B[ i + 3 ][ j + k + 4 ] = a7;
                }
                // A块3区域处理
                for ( s = 0; s < 4; s++ )
                {
                    a0 = A[ j + 4 ][ i + s ];
                    a1 = A[ j + 5 ][ i + s ];
                    a2 = A[ j + 6 ][ i + s ];
                    a3 = A[ j + 7 ][ i + s ];

                    a4 = B[ i + s ][ j + 4 ];
                    a5 = B[ i + s ][ j + 5 ];
                    a6 = B[ i + s ][ j + 6 ];
                    a7 = B[ i + s ][ j + 7 ];

                    B[ i + s ][ j + 4 ] = a0;
                    B[ i + s ][ j + 5 ] = a1;
                    B[ i + s ][ j + 6 ] = a2;
                    B[ i + s ][ j + 7 ] = a3;

                    B[ i + s + 4 ][ j + 0 ] = a4;
                    B[ i + s + 4 ][ j + 1 ] = a5;
                    B[ i + s + 4 ][ j + 2 ] = a6;
                    B[ i + s + 4 ][ j + 3 ] = a7;
                }
                // A块4区域处理
                for ( k = 4; k < 8; k++ )
                {
                    a0 = A[ j + k ][ i + 4 ];
                    a1 = A[ j + k ][ i + 5 ];
                    a2 = A[ j + k ][ i + 6 ];
                    a3 = A[ j + k ][ i + 7 ];

                    B[ i + 4 ][ j + k ] = a0;
                    B[ i + 5 ][ j + k ] = a1;
                    B[ i + 6 ][ j + k ] = a2;
                    B[ i + 7 ][ j + k ] = a3;
                }
            }
        }
    }
    else
    {
        for ( i = 0; i < N; i += 8 )
        {
            for ( j = 0; j < M; j += 11 )
            {
                if ( i + 8 <= N && j + 11 <= M )
                {
                    for ( s = j; s < j + 11; s++ )
                    {
                        a0 = A[ i + 0 ][ s ];
                        a1 = A[ i + 1 ][ s ];
                        a2 = A[ i + 2 ][ s ];
                        a3 = A[ i + 3 ][ s ];
                        a4 = A[ i + 4 ][ s ];
                        a5 = A[ i + 5 ][ s ];
                        a6 = A[ i + 6 ][ s ];
                        a7 = A[ i + 7 ][ s ];

                        B[ s ][ i + 0 ] = a0;
                        B[ s ][ i + 1 ] = a1;
                        B[ s ][ i + 2 ] = a2;
                        B[ s ][ i + 3 ] = a3;
                        B[ s ][ i + 4 ] = a4;
                        B[ s ][ i + 5 ] = a5;
                        B[ s ][ i + 6 ] = a6;
                        B[ s ][ i + 7 ] = a7;
                    }
                }
                else
                {
                    for ( k = i; k < ( ( i + 8 < N ) ? i + 8 : N ); k++ )
                    {
                        for ( s = j; s < ( ( j + 11 < M ) ? j + 11 : M ); s++ )
                        {
                            B[ s ][ k ] = A[ k ][ s ];
                        }
                    }
                }
            }
        }
    }
}

/* 
 * sou can define additional transpose functions below. We've defined
 * a simple one below to help sou get started. 
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
 * registerFunctions - This function registers sour transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a hands was to ekperiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register sour solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register ans additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. sou can check the correctness of sour transpose bs calling
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

