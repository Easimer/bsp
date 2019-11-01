// === Copyright (c) 2017-2019 easimer.net. All rights reserved. ===
#include "util_matrix.h"
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

#include <math.h>
#include <stdint.h>
#include <utility>

namespace math {
    matrix4& matrix4::operator=(const matrix4& other) {
        _mm_storeu_ps(m_flValues + 0, _mm_loadu_ps(other.m_flValues + 0));
        _mm_storeu_ps(m_flValues + 4, _mm_loadu_ps(other.m_flValues + 4));
        _mm_storeu_ps(m_flValues + 8, _mm_loadu_ps(other.m_flValues + 8));
        _mm_storeu_ps(m_flValues + 12, _mm_loadu_ps(other.m_flValues + 12));
        return *this;
    }

    bool invert(matrix4& m) {
        float inv[16], det;

        inv[0] = m.m_flValues[5] * m.m_flValues[10] * m.m_flValues[15] -
            m.m_flValues[5] * m.m_flValues[11] * m.m_flValues[14] -
            m.m_flValues[9] * m.m_flValues[6] * m.m_flValues[15] +
            m.m_flValues[9] * m.m_flValues[7] * m.m_flValues[14] +
            m.m_flValues[13] * m.m_flValues[6] * m.m_flValues[11] -
            m.m_flValues[13] * m.m_flValues[7] * m.m_flValues[10];

        inv[4] = -m.m_flValues[4] * m.m_flValues[10] * m.m_flValues[15] +
            m.m_flValues[4] * m.m_flValues[11] * m.m_flValues[14] +
            m.m_flValues[8] * m.m_flValues[6] * m.m_flValues[15] -
            m.m_flValues[8] * m.m_flValues[7] * m.m_flValues[14] -
            m.m_flValues[12] * m.m_flValues[6] * m.m_flValues[11] +
            m.m_flValues[12] * m.m_flValues[7] * m.m_flValues[10];

        inv[8] = m.m_flValues[4] * m.m_flValues[9] * m.m_flValues[15] -
            m.m_flValues[4] * m.m_flValues[11] * m.m_flValues[13] -
            m.m_flValues[8] * m.m_flValues[5] * m.m_flValues[15] +
            m.m_flValues[8] * m.m_flValues[7] * m.m_flValues[13] +
            m.m_flValues[12] * m.m_flValues[5] * m.m_flValues[11] -
            m.m_flValues[12] * m.m_flValues[7] * m.m_flValues[9];

        inv[12] = -m.m_flValues[4] * m.m_flValues[9] * m.m_flValues[14] +
            m.m_flValues[4] * m.m_flValues[10] * m.m_flValues[13] +
            m.m_flValues[8] * m.m_flValues[5] * m.m_flValues[14] -
            m.m_flValues[8] * m.m_flValues[6] * m.m_flValues[13] -
            m.m_flValues[12] * m.m_flValues[5] * m.m_flValues[10] +
            m.m_flValues[12] * m.m_flValues[6] * m.m_flValues[9];

        inv[1] = -m.m_flValues[1] * m.m_flValues[10] * m.m_flValues[15] +
            m.m_flValues[1] * m.m_flValues[11] * m.m_flValues[14] +
            m.m_flValues[9] * m.m_flValues[2] * m.m_flValues[15] -
            m.m_flValues[9] * m.m_flValues[3] * m.m_flValues[14] -
            m.m_flValues[13] * m.m_flValues[2] * m.m_flValues[11] +
            m.m_flValues[13] * m.m_flValues[3] * m.m_flValues[10];

        inv[5] = m.m_flValues[0] * m.m_flValues[10] * m.m_flValues[15] -
            m.m_flValues[0] * m.m_flValues[11] * m.m_flValues[14] -
            m.m_flValues[8] * m.m_flValues[2] * m.m_flValues[15] +
            m.m_flValues[8] * m.m_flValues[3] * m.m_flValues[14] +
            m.m_flValues[12] * m.m_flValues[2] * m.m_flValues[11] -
            m.m_flValues[12] * m.m_flValues[3] * m.m_flValues[10];

        inv[9] = -m.m_flValues[0] * m.m_flValues[9] * m.m_flValues[15] +
            m.m_flValues[0] * m.m_flValues[11] * m.m_flValues[13] +
            m.m_flValues[8] * m.m_flValues[1] * m.m_flValues[15] -
            m.m_flValues[8] * m.m_flValues[3] * m.m_flValues[13] -
            m.m_flValues[12] * m.m_flValues[1] * m.m_flValues[11] +
            m.m_flValues[12] * m.m_flValues[3] * m.m_flValues[9];

        inv[13] = m.m_flValues[0] * m.m_flValues[9] * m.m_flValues[14] -
            m.m_flValues[0] * m.m_flValues[10] * m.m_flValues[13] -
            m.m_flValues[8] * m.m_flValues[1] * m.m_flValues[14] +
            m.m_flValues[8] * m.m_flValues[2] * m.m_flValues[13] +
            m.m_flValues[12] * m.m_flValues[1] * m.m_flValues[10] -
            m.m_flValues[12] * m.m_flValues[2] * m.m_flValues[9];

        inv[2] = m.m_flValues[1] * m.m_flValues[6] * m.m_flValues[15] -
            m.m_flValues[1] * m.m_flValues[7] * m.m_flValues[14] -
            m.m_flValues[5] * m.m_flValues[2] * m.m_flValues[15] +
            m.m_flValues[5] * m.m_flValues[3] * m.m_flValues[14] +
            m.m_flValues[13] * m.m_flValues[2] * m.m_flValues[7] -
            m.m_flValues[13] * m.m_flValues[3] * m.m_flValues[6];

        inv[6] = -m.m_flValues[0] * m.m_flValues[6] * m.m_flValues[15] +
            m.m_flValues[0] * m.m_flValues[7] * m.m_flValues[14] +
            m.m_flValues[4] * m.m_flValues[2] * m.m_flValues[15] -
            m.m_flValues[4] * m.m_flValues[3] * m.m_flValues[14] -
            m.m_flValues[12] * m.m_flValues[2] * m.m_flValues[7] +
            m.m_flValues[12] * m.m_flValues[3] * m.m_flValues[6];

        inv[10] = m.m_flValues[0] * m.m_flValues[5] * m.m_flValues[15] -
            m.m_flValues[0] * m.m_flValues[7] * m.m_flValues[13] -
            m.m_flValues[4] * m.m_flValues[1] * m.m_flValues[15] +
            m.m_flValues[4] * m.m_flValues[3] * m.m_flValues[13] +
            m.m_flValues[12] * m.m_flValues[1] * m.m_flValues[7] -
            m.m_flValues[12] * m.m_flValues[3] * m.m_flValues[5];

        inv[14] = -m.m_flValues[0] * m.m_flValues[5] * m.m_flValues[14] +
            m.m_flValues[0] * m.m_flValues[6] * m.m_flValues[13] +
            m.m_flValues[4] * m.m_flValues[1] * m.m_flValues[14] -
            m.m_flValues[4] * m.m_flValues[2] * m.m_flValues[13] -
            m.m_flValues[12] * m.m_flValues[1] * m.m_flValues[6] +
            m.m_flValues[12] * m.m_flValues[2] * m.m_flValues[5];

        inv[3] = -m.m_flValues[1] * m.m_flValues[6] * m.m_flValues[11] +
            m.m_flValues[1] * m.m_flValues[7] * m.m_flValues[10] +
            m.m_flValues[5] * m.m_flValues[2] * m.m_flValues[11] -
            m.m_flValues[5] * m.m_flValues[3] * m.m_flValues[10] -
            m.m_flValues[9] * m.m_flValues[2] * m.m_flValues[7] +
            m.m_flValues[9] * m.m_flValues[3] * m.m_flValues[6];

        inv[7] = m.m_flValues[0] * m.m_flValues[6] * m.m_flValues[11] -
            m.m_flValues[0] * m.m_flValues[7] * m.m_flValues[10] -
            m.m_flValues[4] * m.m_flValues[2] * m.m_flValues[11] +
            m.m_flValues[4] * m.m_flValues[3] * m.m_flValues[10] +
            m.m_flValues[8] * m.m_flValues[2] * m.m_flValues[7] -
            m.m_flValues[8] * m.m_flValues[3] * m.m_flValues[6];

        inv[11] = -m.m_flValues[0] * m.m_flValues[5] * m.m_flValues[11] +
            m.m_flValues[0] * m.m_flValues[7] * m.m_flValues[9] +
            m.m_flValues[4] * m.m_flValues[1] * m.m_flValues[11] -
            m.m_flValues[4] * m.m_flValues[3] * m.m_flValues[9] -
            m.m_flValues[8] * m.m_flValues[1] * m.m_flValues[7] +
            m.m_flValues[8] * m.m_flValues[3] * m.m_flValues[5];

        inv[15] = m.m_flValues[0] * m.m_flValues[5] * m.m_flValues[10] -
            m.m_flValues[0] * m.m_flValues[6] * m.m_flValues[9] -
            m.m_flValues[4] * m.m_flValues[1] * m.m_flValues[10] +
            m.m_flValues[4] * m.m_flValues[2] * m.m_flValues[9] +
            m.m_flValues[8] * m.m_flValues[1] * m.m_flValues[6] -
            m.m_flValues[8] * m.m_flValues[2] * m.m_flValues[5];

        det = m.m_flValues[0] * inv[0] + m.m_flValues[1] * inv[4] + m.m_flValues[2] * inv[8] + m.m_flValues[3] * inv[12];

        if (det == 0)
            return false;

        det = 1.0f / det;

        for (int i = 0; i < 16; i++) {
            m.m_flValues[i] = (float)(inv[i] * det);
        }
        return true;
    }

    matrix4 translate(float x, float y, float z) {
        float m[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f,
        };
        return matrix4(m);
    }

    inline matrix4 matmul_aligned(const matrix4& A, const matrix4& B) {
        matrix4 m;
        __m128 col[4], sum[4];

        col[0] = _mm_load_ps(B.m_flValues + 0);
        col[1] = _mm_load_ps(B.m_flValues + 4);
        col[2] = _mm_load_ps(B.m_flValues + 8);
        col[3] = _mm_load_ps(B.m_flValues + 12);

        sum[0] = _mm_setzero_ps();
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 0]), col[0], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 1]), col[1], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 2]), col[2], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 3]), col[3], sum[0]);

        sum[1] = _mm_setzero_ps();
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 0]), col[0], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 1]), col[1], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 2]), col[2], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 3]), col[3], sum[1]);

        sum[2] = _mm_setzero_ps();
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 0]), col[0], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 1]), col[1], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 2]), col[2], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 3]), col[3], sum[2]);

        sum[3] = _mm_setzero_ps();
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 0]), col[0], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 1]), col[1], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 2]), col[2], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 3]), col[3], sum[3]);

        _mm_storeu_ps(m.m_flValues + 0, sum[0]);
        _mm_storeu_ps(m.m_flValues + 4, sum[1]);
        _mm_storeu_ps(m.m_flValues + 8, sum[2]);
        _mm_storeu_ps(m.m_flValues + 12, sum[3]);

        return m;
    }

    inline matrix4 matmul_unaligned(const matrix4& A, const matrix4& B) {
        matrix4 m;
        __m128 col[4], sum[4];

        col[0] = _mm_loadu_ps(B.m_flValues + 0);
        col[1] = _mm_loadu_ps(B.m_flValues + 4);
        col[2] = _mm_loadu_ps(B.m_flValues + 8);
        col[3] = _mm_loadu_ps(B.m_flValues + 12);

        sum[0] = _mm_setzero_ps();
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 0]), col[0], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 1]), col[1], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 2]), col[2], sum[0]);
        sum[0] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[0 * 4 + 3]), col[3], sum[0]);

        sum[1] = _mm_setzero_ps();
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 0]), col[0], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 1]), col[1], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 2]), col[2], sum[1]);
        sum[1] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[1 * 4 + 3]), col[3], sum[1]);

        sum[2] = _mm_setzero_ps();
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 0]), col[0], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 1]), col[1], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 2]), col[2], sum[2]);
        sum[2] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[2 * 4 + 3]), col[3], sum[2]);

        sum[3] = _mm_setzero_ps();
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 0]), col[0], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 1]), col[1], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 2]), col[2], sum[3]);
        sum[3] = _mm_fmadd_ps(_mm_set1_ps(A.m_flValues[3 * 4 + 3]), col[3], sum[3]);

        _mm_storeu_ps(m.m_flValues + 0, sum[0]);
        _mm_storeu_ps(m.m_flValues + 4, sum[1]);
        _mm_storeu_ps(m.m_flValues + 8, sum[2]);
        _mm_storeu_ps(m.m_flValues + 12, sum[3]);

        return m;
    }

    matrix4 operator*(const matrix4& A, const matrix4& B) {
        if (((uint64_t)A.ptr() & 15) == 0 && ((uint64_t)B.ptr() & 15) == 0) {
            return matmul_aligned(A, B);
        } else {
            return matmul_unaligned(A, B);
        }
    }

    matrix4 perspective(matrix4& forward, matrix4& inverse, float width, float height, float fov, float near, float far) {
        float aspect = height / width;
        float e = 1.0f / tanf(fov / 2.0f);

        float F = ((far + near) / (near - far));
        float f = (2 * far * near) / (near - far);

        float m[16] = {
            e, 0, 0, 0,
            0, e / aspect, 0, 0,
            0, 0, F, -1.0f,
            0, 0, f, 0,
        };

        float minv[16] = {
            1 / e, 0, 0, 0,
            0, aspect / e, 0, 0,
            0, 0, 0, 1 / f,
            0, 0, -1, F / f
        };
        forward = matrix4(m);
        inverse = matrix4(minv);
        return forward;
    }

    void transpose(matrix4& m) {
        std::swap(m.ptr()[1], m.ptr()[4]);
        std::swap(m.ptr()[2], m.ptr()[8]);
        std::swap(m.ptr()[3], m.ptr()[12]);
        std::swap(m.ptr()[6], m.ptr()[9]);
        std::swap(m.ptr()[7], m.ptr()[13]);
        std::swap(m.ptr()[11], m.ptr()[14]);
    }

    matrix4 transposed(const matrix4& m) {
        matrix4 ret(m);

        std::swap(ret.ptr()[1], ret.ptr()[4]);
        std::swap(ret.ptr()[2], ret.ptr()[8]);
        std::swap(ret.ptr()[3], ret.ptr()[12]);
        std::swap(ret.ptr()[6], ret.ptr()[9]);
        std::swap(ret.ptr()[7], ret.ptr()[13]);
        std::swap(ret.ptr()[11], ret.ptr()[14]);

        return ret;
    }

    bool operator<(const matrix4& lhs, const matrix4& rhs) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (lhs.idx(x, y) < rhs.idx(x, y)) {
                    return true;
                }
            }
        }
        return false;
    }
}