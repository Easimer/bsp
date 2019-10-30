// === Copyright (c) 2017-2018 easimer.net. All rights reserved. ===
#pragma once

#include <cmath>
#include "util_vector.h"

namespace math {
    class matrix4;

    // Column-major
    class matrix4 {
    public:
        constexpr matrix4() noexcept : m_flValues{} {}
        constexpr matrix4(float v) noexcept : m_flValues{ v, 0, 0, 0, 0, v, 0, 0, 0, 0, v, 0, 0, 0, 0, v } {}

        //matrix4(const float m[16]) {
        //	lens::memcpy(m_flValues, m, 16 * sizeof(float));
        //}

        constexpr matrix4(const float m[16]) :
            m_flValues{
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15] } {
        }

        matrix4& operator=(const matrix4& other);

        const float* ptr() const noexcept {
            return m_flValues;
        }

        float* ptr() noexcept {
            return m_flValues;
        }

        friend matrix4 operator*(const matrix4& lhs, const matrix4& rhs);

        float idx(int row, int col) const {
            row &= 0b11;
            col &= 0b11;
            return m_flValues[col * 4 + row];
        }

        float& idx(int row, int col) {
            row &= 0b11;
            col &= 0b11;
            return m_flValues[col * 4 + row];
        }

        float m_flValues[16];
    };

    /// \brief Invert a matrix
    ///
    /// \returns Whether the matrix has an inverse or not
    bool invert(matrix4& m);
    /// \brief Make a translation matrix
    ///
    matrix4 translate(float x, float y, float z);
    /// \brief Matrix multiplication
    ///
    matrix4 operator*(const matrix4& A, const matrix4& B);
    /// \brief make a perspective projection matrix
    ///
    matrix4 perspective(matrix4& forward, matrix4& inverse, float width, float height, float fov, float near, float far);
    /// \brief Transpose a matrix
    ///
    void transpose(matrix4& m);
    /// \brief Make a transpose of a matrix
    ///
    matrix4 transposed(const matrix4& m);
    /// \brief Lexicographical comparison of two matrices
    ///
    bool operator<(const matrix4& lhs, const matrix4& rhs);
}

inline math::matrix4 MakeRotationZ(float flRot) {
    math::matrix4 ret(1.0f);

    ret.idx(0, 0) = cos(flRot);
    ret.idx(0, 1) = sin(flRot);
    ret.idx(1, 0) = -sin(flRot);
    ret.idx(1, 1) = cos(flRot);

    return ret;
}

inline math::matrix4 MakeRotationY(float flRot) {
    math::matrix4 ret(1.0f);

    ret.idx(0, 0) = cos(flRot);
    ret.idx(0, 2) = sin(flRot);
    ret.idx(2, 0) = -sin(flRot);
    ret.idx(2, 2) = cos(flRot);

    return ret;
}

inline vector4 operator*(const math::matrix4& lhs, const vector4& rhs) {
    const float* pMat = lhs.ptr();
    const float* pVec = rhs.v;
    const float v0 = pVec[0];
    const float v1 = pVec[1];
    const float v2 = pVec[2];
    const float v3 = pVec[3];
    float r[4];
    for (int i = 0; i < 4; i++) {
        float c0 = pMat[i + 0 * 4];
        float c1 = pMat[i + 1 * 4];
        float c2 = pMat[i + 2 * 4];
        float c3 = pMat[i + 3 * 4];

        r[i] = c0 * v0 + c1 * v1 + c2 * v2 + c3 * v3;
    }
    return vector4(r[0], r[1], r[2], r[3]);
}
