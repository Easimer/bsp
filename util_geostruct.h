#pragma once

#include "util_vector.h"

#define POLYGON_MAX_POINTS (16)
#define LINECONT_MAX_POINTS (POLYGON_MAX_POINTS * 2 - 1)
#define POLYCONT_MAX_POLYS ((POLYGON_MAX_POINTS - 2) * 3)

class Plane {
public:
    Plane() {}
    Plane(vector4 p0, vector4 p1, vector4 p2);
    Plane(float A, float B, float C, float D);

    const vector4& operator[](int iIdx) const {
        return p[iIdx % 3];
    }

    vector4& operator[](int iIdx) {
        return p[iIdx % 3];
    }

    constexpr int Count() const { return 3; }

    int cnt = 3; // Deprecated, use Count()
    vector4 p[3];
    float co[4];
};

using plane = Plane;

inline vector4 Normal(const Plane& plane) {
    return cross(plane[2] - plane[0], plane[2] - plane[1]);
}

class Line {
public:
    Line() {}
    Line(const vector4& p0, const vector4& p1)
        : p{ p0, p1 } {
    }

    float Length() const {
        return (p[1] - p[0]).length();
    }

    const vector4& operator[](int iIdx) const {
        return p[iIdx % 2];
    }

    vector4& operator[](int iIdx) {
        return p[iIdx % 2];
    }

    constexpr int Count() const { return 2; }

    int cnt = 2; // Deprecated, use Count()
    vector4 p[2];
};

using line = Line;

template<typename T>
struct LinesIterator {
    explicit LinesIterator(const T& poly, int idx = 0)
        : m_poly(poly), m_iLine(idx) {

    }

    typedef line value_type;
    typedef int difference_type;
    typedef line* pointer;
    typedef line& reference;
    typedef std::input_iterator_tag iterator_category;

    const T& m_poly;
    int m_iLine;

    line operator*() const {
        return GetLine(m_iLine);
    }

    bool operator==(const lines_iterator& it) const {
        return m_iLine == it.m_iLine && &m_poly == &it.m_poly;
    }

    bool operator!=(const lines_iterator& it) const {
        return !(*this == it);
    }

    line operator++(int) {
        auto ret = GetLine(m_iLine);
        ++* this;
        return ret;
    }

    lines_iterator& operator++() {
        m_iLine++;
        return *this;
    }

    line GetLine(int idx) const {
        return line(m_poly.points[idx], m_poly.points[(idx+ 1) % m_poly.Count()]);
    }
};

template<typename T>
using lines_iterator = LinesIterator;

class BaseGeometricContainer {
public:
    BaseGeometricContainer()
        : cnt(0), points(NULL), m_nCapacity(0) {}

    vector4* points;
    int cnt; // Deprecated, use Count(). Will eventually become private.

    int Count() const {
        return cnt;
    }

    bool IsEmpty() const {
        return cnt == 0;
    }

protected:
    void AppendPoint(const vector4& p) {
        // Allocate buffer if we're empty
        if (!points) {
            points = new vector4[4];
            m_nCapacity = 4;
        }
        
        if (cnt == m_nCapacity) {
            vector4* pNewBuf = new vector4[m_nCapacity * 2];
            m_nCapacity *= 2;
            //memcpy(pNewBuf, points, Count() * sizeof(vector4));
        }
    }

private:
    unsigned m_nCapacity;
};

class Polygon {
    Polygon() : cnt(0) {}

    vector4* points;
    int cnt; // Deprecated, use Count(). Will eventually become private.
};

using polygon = Polygon;
