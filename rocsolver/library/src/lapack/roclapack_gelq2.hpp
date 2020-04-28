/************************************************************************
 * Derived from the BSD3-licensed
 * LAPACK routine (version 3.9.0) --
 *     Univ. of Tennessee, Univ. of California Berkeley,
 *     Univ. of Colorado Denver and NAG Ltd..
 *     November 2019
 * Copyright 2019-2020 Advanced Micro Devices, Inc.
 * ***********************************************************************/

#ifndef ROCLAPACK_GELQ2_H
#define ROCLAPACK_GELQ2_H

#include "rocblas.hpp"
#include "rocsolver.h"
#include "common_device.hpp"
#include "../auxiliary/rocauxiliary_larfg.hpp"
#include "../auxiliary/rocauxiliary_larf.hpp"

template <typename T, bool BATCHED>
void rocsolver_gelq2_getMemorySize(const rocblas_int m, const rocblas_int n, const rocblas_int batch_count,
                                  size_t *size_1, size_t *size_2, size_t *size_3, size_t *size_4)
{
    rocsolver_larf_getMemorySize<T,BATCHED>(rocblas_side_right,m,n,batch_count,size_1,size_2,size_3);
    *size_4 = sizeof(T)*batch_count;
}

template <typename T, typename U>
rocblas_status rocsolver_gelq2_template(rocblas_handle handle, const rocblas_int m,
                                        const rocblas_int n, U A, const rocblas_int shiftA, const rocblas_int lda, 
                                        const rocblas_stride strideA, T* ipiv,  
                                        const rocblas_stride strideP, const rocblas_int batch_count,
                                        T* scalars, T* work, T** workArr, T* diag)
{
    // quick return
    if (m == 0 || n == 0 || batch_count == 0) 
        return rocblas_status_success;

    hipStream_t stream;
    rocblas_get_stream(handle, &stream);

    rocblas_int dim = min(m, n);    //total number of pivots    

    for (rocblas_int j = 0; j < dim; ++j) {
        // generate Householder reflector to work on row j
        rocsolver_larfg_template(handle,
                                 n - j,                                 //order of reflector
                                 A, shiftA + idx2D(j,j,lda),            //value of alpha
                                 A, shiftA + idx2D(j,min(j+1,n-1),lda), //vector x to work on
                                 lda, strideA,                          //inc of x    
                                 (ipiv + j), strideP,                   //tau
                                 batch_count, diag);

        // insert one in A(j,j) tobuild/apply the householder matrix 
        hipLaunchKernelGGL(set_one_diag,dim3(batch_count,1,1),dim3(1,1,1),0,stream,diag,A,shiftA+idx2D(j,j,lda),strideA);

        // Apply Householder reflector to the rest of matrix from the right 
        if (j < m - 1) {
            rocsolver_larf_template(handle,rocblas_side_right,          //side
                                    m - j - 1,                          //number of rows of matrix to modify
                                    n - j,                              //number of columns of matrix to modify    
                                    A, shiftA + idx2D(j,j,lda),         //householder vector x
                                    lda, strideA,                       //inc of x
                                    (ipiv + j), strideP,                //householder scalar (alpha)
                                    A, shiftA + idx2D(j+1,j,lda),       //matrix to work on
                                    lda, strideA,                       //leading dimension
                                    batch_count, scalars, work, workArr);
        }

        // restore original value of A(j,j)
        hipLaunchKernelGGL(restore_diag,dim3(batch_count,1,1),dim3(1,1,1),0,stream,diag,A,shiftA+idx2D(j,j,lda),strideA);
    }

    return rocblas_status_success;
}

#endif /* ROCLAPACK_GELQ2_H */