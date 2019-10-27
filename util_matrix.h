// === Copyright (c) 2017-2018 easimer.net. All rights reserved. ===
#pragma once

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