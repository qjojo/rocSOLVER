/* **************************************************************************
 * Copyright (C) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * *************************************************************************/

#include "roclapack_potf2.hpp"

ROCSOLVER_BEGIN_NAMESPACE

template <typename T, typename I, typename U>
rocblas_status rocsolver_potf2_strided_batched_impl(rocblas_handle handle,
                                                    const rocblas_fill uplo,
                                                    const I n,
                                                    U A,
                                                    const I lda,
                                                    const rocblas_stride strideA,
                                                    I* info,
                                                    const I batch_count)
{
    ROCSOLVER_ENTER_TOP("potf2_strided_batched", "--uplo", uplo, "-n", n, "--lda", lda, "--strideA",
                        strideA, "--batch_count", batch_count);

    if(!handle)
        return rocblas_status_invalid_handle;

    // argument checking
    rocblas_status st = rocsolver_potf2_potrf_argCheck(handle, uplo, n, lda, A, info, batch_count);
    if(st != rocblas_status_continue)
        return st;

    // working with unshifted arrays
    rocblas_stride shiftA = 0;

    // memory workspace sizes:
    // size for constants in rocblas calls
    size_t size_scalars;
    // size of reusable workspace
    size_t size_work;
    // size to store pivots in intermediate computations
    size_t size_pivots;
    rocsolver_potf2_getMemorySize<T>(n, batch_count, &size_scalars, &size_work, &size_pivots);

    if(rocblas_is_device_memory_size_query(handle))
        return rocblas_set_optimal_device_memory_size(handle, size_scalars, size_work, size_pivots);

    // memory workspace allocation
    void *scalars, *work, *pivots;
    rocblas_device_malloc mem(handle, size_scalars, size_work, size_pivots);

    if(!mem)
        return rocblas_status_memory_error;

    scalars = mem[0];
    work = mem[1];
    pivots = mem[2];
    if(size_scalars > 0)
        init_scalars(handle, (T*)scalars);

    // execution
    return rocsolver_potf2_template<T>(handle, uplo, n, A, shiftA, lda, strideA, info, batch_count,
                                       (T*)scalars, (T*)work, (T*)pivots);
}

ROCSOLVER_END_NAMESPACE

/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

rocblas_status rocsolver_spotf2_strided_batched(rocblas_handle handle,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                float* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_int* info,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_potf2_strided_batched_impl<float>(handle, uplo, n, A, lda, strideA,
                                                                  info, batch_count);
}

rocblas_status rocsolver_dpotf2_strided_batched(rocblas_handle handle,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                double* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_int* info,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_potf2_strided_batched_impl<double>(handle, uplo, n, A, lda, strideA,
                                                                   info, batch_count);
}

rocblas_status rocsolver_cpotf2_strided_batched(rocblas_handle handle,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                rocblas_float_complex* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_int* info,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_potf2_strided_batched_impl<rocblas_float_complex>(
        handle, uplo, n, A, lda, strideA, info, batch_count);
}

rocblas_status rocsolver_zpotf2_strided_batched(rocblas_handle handle,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                rocblas_double_complex* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_int* info,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_potf2_strided_batched_impl<rocblas_double_complex>(
        handle, uplo, n, A, lda, strideA, info, batch_count);
}

rocblas_status rocsolver_spotf2_strided_batched_64(rocblas_handle handle,
                                                   const rocblas_fill uplo,
                                                   const int64_t n,
                                                   float* A,
                                                   const int64_t lda,
                                                   const rocblas_stride strideA,
                                                   int64_t* info,
                                                   const int64_t batch_count)
{
#ifdef HAVE_ROCBLAS_64
    return rocsolver::rocsolver_potf2_strided_batched_impl<float>(handle, uplo, n, A, lda, strideA,
                                                                  info, batch_count);
#else
    return rocblas_status_not_implemented;
#endif
}

rocblas_status rocsolver_dpotf2_strided_batched_64(rocblas_handle handle,
                                                   const rocblas_fill uplo,
                                                   const int64_t n,
                                                   double* A,
                                                   const int64_t lda,
                                                   const rocblas_stride strideA,
                                                   int64_t* info,
                                                   const int64_t batch_count)
{
#ifdef HAVE_ROCBLAS_64
    return rocsolver::rocsolver_potf2_strided_batched_impl<double>(handle, uplo, n, A, lda, strideA,
                                                                   info, batch_count);
#else
    return rocblas_status_not_implemented;
#endif
}

rocblas_status rocsolver_cpotf2_strided_batched_64(rocblas_handle handle,
                                                   const rocblas_fill uplo,
                                                   const int64_t n,
                                                   rocblas_float_complex* A,
                                                   const int64_t lda,
                                                   const rocblas_stride strideA,
                                                   int64_t* info,
                                                   const int64_t batch_count)
{
#ifdef HAVE_ROCBLAS_64
    return rocsolver::rocsolver_potf2_strided_batched_impl<rocblas_float_complex>(
        handle, uplo, n, A, lda, strideA, info, batch_count);
#else
    return rocblas_status_not_implemented;
#endif
}

rocblas_status rocsolver_zpotf2_strided_batched_64(rocblas_handle handle,
                                                   const rocblas_fill uplo,
                                                   const int64_t n,
                                                   rocblas_double_complex* A,
                                                   const int64_t lda,
                                                   const rocblas_stride strideA,
                                                   int64_t* info,
                                                   const int64_t batch_count)
{
#ifdef HAVE_ROCBLAS_64
    return rocsolver::rocsolver_potf2_strided_batched_impl<rocblas_double_complex>(
        handle, uplo, n, A, lda, strideA, info, batch_count);
#else
    return rocblas_status_not_implemented;
#endif
}
}
