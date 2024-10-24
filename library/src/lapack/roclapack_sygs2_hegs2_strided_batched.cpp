/* **************************************************************************
 * Copyright (C) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include "roclapack_sygs2_hegs2.hpp"

ROCSOLVER_BEGIN_NAMESPACE

template <typename T, typename U>
rocblas_status rocsolver_sygs2_hegs2_strided_batched_impl(rocblas_handle handle,
                                                          const rocblas_eform itype,
                                                          const rocblas_fill uplo,
                                                          const rocblas_int n,
                                                          U A,
                                                          const rocblas_int lda,
                                                          const rocblas_stride strideA,
                                                          U B,
                                                          const rocblas_int ldb,
                                                          const rocblas_stride strideB,
                                                          const rocblas_int batch_count)
{
    const char* name = (!rocblas_is_complex<T> ? "sygs2_strided_batched" : "hegs2_strided_batched");
    ROCSOLVER_ENTER_TOP(name, "--itype", itype, "--uplo", uplo, "-n", n, "--lda", lda, "--strideA",
                        strideA, "--ldb", ldb, "--strideB", strideB, "--batch_count", batch_count);

    if(!handle)
        return rocblas_status_invalid_handle;

    // argument checking
    rocblas_status st
        = rocsolver_sygs2_hegs2_argCheck(handle, itype, uplo, n, lda, ldb, A, B, batch_count);
    if(st != rocblas_status_continue)
        return st;

    // working with unshifted arrays
    rocblas_int shiftA = 0;
    rocblas_int shiftB = 0;

    // memory workspace sizes:
    // size for constants in rocblas calls
    size_t size_scalars;
    // size of reusable workspace (and for calling TRSV or TRMV)
    size_t size_work, size_store_wcs;
    // size of array of pointers (only for batched case)
    size_t size_workArr;
    rocsolver_sygs2_hegs2_getMemorySize<false, T>(itype, n, batch_count, &size_scalars, &size_work,
                                                  &size_store_wcs, &size_workArr);

    if(rocblas_is_device_memory_size_query(handle))
        return rocblas_set_optimal_device_memory_size(handle, size_scalars, size_work,
                                                      size_store_wcs, size_workArr);

    // memory workspace allocation
    void *scalars, *work, *store_wcs, *workArr;
    rocblas_device_malloc mem(handle, size_scalars, size_work, size_store_wcs, size_workArr);

    if(!mem)
        return rocblas_status_memory_error;

    scalars = mem[0];
    work = mem[1];
    store_wcs = mem[2];
    workArr = mem[3];
    if(size_scalars > 0)
        init_scalars(handle, (T*)scalars);

    // execution
    return rocsolver_sygs2_hegs2_template<false, T>(handle, itype, uplo, n, A, shiftA, lda, strideA,
                                                    B, shiftB, ldb, strideB, batch_count,
                                                    (T*)scalars, work, store_wcs, (T**)workArr);
}

ROCSOLVER_END_NAMESPACE

/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

rocblas_status rocsolver_ssygs2_strided_batched(rocblas_handle handle,
                                                const rocblas_eform itype,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                float* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                float* B,
                                                const rocblas_int ldb,
                                                const rocblas_stride strideB,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_sygs2_hegs2_strided_batched_impl<float>(
        handle, itype, uplo, n, A, lda, strideA, B, ldb, strideB, batch_count);
}

rocblas_status rocsolver_dsygs2_strided_batched(rocblas_handle handle,
                                                const rocblas_eform itype,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                double* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                double* B,
                                                const rocblas_int ldb,
                                                const rocblas_stride strideB,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_sygs2_hegs2_strided_batched_impl<double>(
        handle, itype, uplo, n, A, lda, strideA, B, ldb, strideB, batch_count);
}

rocblas_status rocsolver_chegs2_strided_batched(rocblas_handle handle,
                                                const rocblas_eform itype,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                rocblas_float_complex* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_float_complex* B,
                                                const rocblas_int ldb,
                                                const rocblas_stride strideB,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_sygs2_hegs2_strided_batched_impl<rocblas_float_complex>(
        handle, itype, uplo, n, A, lda, strideA, B, ldb, strideB, batch_count);
}

rocblas_status rocsolver_zhegs2_strided_batched(rocblas_handle handle,
                                                const rocblas_eform itype,
                                                const rocblas_fill uplo,
                                                const rocblas_int n,
                                                rocblas_double_complex* A,
                                                const rocblas_int lda,
                                                const rocblas_stride strideA,
                                                rocblas_double_complex* B,
                                                const rocblas_int ldb,
                                                const rocblas_stride strideB,
                                                const rocblas_int batch_count)
{
    return rocsolver::rocsolver_sygs2_hegs2_strided_batched_impl<rocblas_double_complex>(
        handle, itype, uplo, n, A, lda, strideA, B, ldb, strideB, batch_count);
}

} // extern C
