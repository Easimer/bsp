#pragma once

#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

struct alignas(16) vector4 {
    vector4(float x = 0, float y = 0, float z = 0, float w = 0) : v{ x, y, z, w } {}
    float v[4];

    float operator[](int idx) const {
        return v[idx];
    }

    float& operator[](int idx) {
        return v[idx];
    }

    vector4& operator=(const vector4& other) {
        _mm_store_ps(v, _mm_load_ps(other.v));
        return *this;
    }

    float length_sq() const {
        return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
    }

    float length() const {
        return sqrtf(length_sq());
    }
};

template<typename T>
inline T abs(T value) {
    if (value >= 0) {
        return value;
    } else {
        return -value;
    }
}

inline vector4 operator+(const vector4& lhs, const vector4& rhs) {
    vector4 ret;

    _mm_store_ps(ret.v, _mm_add_ps(_mm_load_ps(lhs.v), _mm_load_ps(rhs.v)));

    return ret;
}

inline vector4 operator-(const vector4& lhs, const vector4& rhs) {
    vector4 ret;
    _mm_store_ps(ret.v, _mm_sub_ps(_mm_load_ps(lhs.v), _mm_load_ps(rhs.v)));
    return ret;
}

inline vector4 operator*(float lhs, const vector4& rhs) {
    vector4 ret;
    _mm_store_ps(ret.v, _mm_mul_ps(_mm_load1_ps(&lhs), _mm_load_ps(rhs.v)));
    return ret;
}

inline vector4 operator*(const vector4& lhs, float rhs) {
    vector4 ret;
    _mm_store_ps(ret.v, _mm_mul_ps(_mm_load1_ps(&rhs), _mm_load_ps(lhs.v)));
    return ret;
}

inline vector4 operator/(const vector4& lhs, float rhs) {
    vector4 ret;
    _mm_store_ps(ret.v, _mm_div_ps(_mm_load_ps(lhs.v), _mm_load1_ps(&rhs)));
    return ret;
}

inline bool operator==(const vector4& lhs, const vector4& rhs) {
    vector4 temp;
    _mm_store_ps(temp.v, (_mm_sub_ps(_mm_load_ps(lhs.v), _mm_load_ps(rhs.v))));
    bool ret = true;

    for (int i = 0; i < 4; i++) {
        if (abs(temp[i]) > 0.05) {
            ret = false;
        }
    }

    return ret;
}

inline bool operator!=(const vector4& lhs, const vector4& rhs) {
    return !(lhs == rhs);
}

inline float dot(const vector4& lhs, const vector4& rhs) {
    return
        lhs[0] * rhs[0] +
        lhs[1] * rhs[1] +
        lhs[2] * rhs[2] +
        lhs[3] * rhs[3];
}

inline vector4 cross(const vector4& lhs, const vector4& rhs) {
    vector4 ret;
    auto v0 = _mm_set_ps(0, lhs[0], lhs[2], lhs[1]);
    auto v1 = _mm_set_ps(0, rhs[1], rhs[0], rhs[2]);
    auto v2 = _mm_set_ps(0, rhs[0], rhs[2], rhs[1]);
    auto v3 = _mm_set_ps(0, lhs[1], lhs[0], lhs[2]);
    auto v4 = _mm_mul_ps(v0, v1);
    auto v5 = _mm_mul_ps(v2, v3);
    _mm_storeu_ps(ret.v, _mm_sub_ps(v4, v5));
    return ret;
}
