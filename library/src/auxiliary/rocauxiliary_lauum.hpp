/************************************************************************
 * Copyright (c) 2022 Advanced Micro Devices, Inc.
 * ***********************************************************************/

#pragma once

#include "rocblas.hpp"
#include "rocsolver/rocsolver.h"

template <typename T>
rocblas_status rocsolver_lauum_argCheck(rocblas_handle handle,
                                        const rocblas_fill uplo,
                                        const rocblas_int n,
                                        T A,
                                        const rocblas_int lda,
                                        rocblas_int* info)
{
    // order is important for unit tests:

    // 1. invalid/non-supported values
    if(uplo != rocblas_fill_upper && uplo != rocblas_fill_lower)
        return rocblas_status_invalid_value;

    // 2. invalid size
    if(n < 0 || lda < n)
        return rocblas_status_invalid_size;

    // skip pointer check if querying memory size
    if(rocblas_is_device_memory_size_query(handle))
        return rocblas_status_continue;

    // 3. invalid pointers
    if((n && !A) || !info)
        return rocblas_status_invalid_pointer;

    return rocblas_status_continue;
}

template <typename T>
void rocsolver_lauum_getMemorySize(const rocblas_int n, const rocblas_int batch_count, size_t* size_work)
{
    *size_work = 0;

    // if quick return, no workspace is needed
    if(n == 0 || batch_count == 0)
        return;

    // size of workspace
    *size_work = sizeof(T) * n * n * batch_count;
}

template <typename T, typename U>
rocblas_status rocsolver_lauum_template(rocblas_handle handle,
                                        const rocblas_fill uplo,
                                        const rocblas_int n,
                                        U* A,
                                        const rocblas_int shiftA,
                                        const rocblas_int lda,
                                        rocblas_int* info,
                                        const rocblas_stride strideA,
                                        const rocblas_int batch_count,
                                        U* work,
                                        size_t size_work)
{
    ROCSOLVER_ENTER("lauum", "uplo:", uplo, "n:", n, "shiftA:", shiftA, "lda:", lda,
                    "strideA:", strideA, "bc:", batch_count);

    // quick return
    if(n == 0 || batch_count == 0)
        return rocblas_status_success;

    hipStream_t stream;
    rocblas_get_stream(handle, &stream);

    rocblas_int blocks = (n - 1) / 32 + 1;
    dim3 grid(blocks, blocks, batch_count);
    dim3 threads(32, 32);
    T one = 1;
    T zero = 0;

    rocblas_fill uploC = (uplo == rocblas_fill_upper) ? rocblas_fill_lower : rocblas_fill_upper;

    // put the triangular factor of interest in work
    ROCSOLVER_LAUNCH_KERNEL(set_zero<T>, grid, threads, 0, stream, n, n, work, shiftA, lda, strideA,
                            uploC);
    ROCSOLVER_LAUNCH_KERNEL(copy_mat<T>, grid, threads, 0, stream, n, n, A, shiftA, lda, strideA,
                            work, shiftA, lda, strideA, no_mask{}, uplo);

    rocblas_side side;
    if(uplo == rocblas_fill_upper)
    {
        side = rocblas_side_right;
    }
    else
    {
        side = rocblas_side_left;
    }

    // work = work * A' or work = A' * work
    rocblasCall_trmm<false, true, T>(handle, side, uplo, rocblas_operation_conjugate_transpose,
                                     rocblas_diagonal_non_unit, lda, n, &one, 0, A, shiftA, lda,
                                     strideA, work, shiftA, lda, strideA, batch_count);

    // copy the new factor into the relevant triangle of A leaving the rest untouched
    ROCSOLVER_LAUNCH_KERNEL(copy_mat<T>, grid, threads, 0, stream, n, n, work, shiftA, lda, strideA,
                            A, shiftA, lda, strideA, no_mask{}, uplo);

    return rocblas_status_success;
}
