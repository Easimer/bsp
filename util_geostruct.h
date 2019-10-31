#pragma once

#include "util_vector.h"
#include <cstddef>
#include <iterator>
#include <assert.h>

#define POLYGON_MAX_POINTS (16)
#define LINECONT_MAX_POINTS (POLYGON_MAX_POINTS * 2 - 1)
#define POLYCONT_MAX_POLYS ((POLYGON_MAX_POINTS - 2) * 3)

struct Plane {
    Plane() {}
    Plane(vector4 p0, vector4 p1, vector4 p2)
        : p{ p0, p1, p2 } {
    }

    [[deprecated("Direct access is deprecated. Use indexing operator.")]] vector4 p[3];

    int Count() const {
        return 3;
    }

    const vector4& operator[](int iIdx) const {
        return p[iIdx];
    }

    vector4& operator[](int iIdx) {
        return p[iIdx];
    }
};

using plane [[deprecated]] = Plane;

struct Line {
    Line() {}
    Line(const vector4& p0, const vector4& p1) : p{ p0, p1 } {
    }
    [[deprecated("Direct access is deprecated. Use indexing operator.")]] vector4 p[2];

    [[deprecated("Use Length()")]] float length() const {
        return (p[1] - p[0]).length();
    }

    float Length() const {
        return (p[1] - p[0]).length();
    }
};

using line [[deprecated]] = Line;

template<typename T>
struct LinesIterator {
    explicit LinesIterator(const T& poly, int idx = 0)
        : m_poly(poly), m_iLine(idx) {

    }

    typedef Line value_type;
    typedef int difference_type;
    typedef Line* pointer;
    typedef Line& reference;
    typedef std::input_iterator_tag iterator_category;

    const T& m_poly;
    int m_iLine;

    line operator*() const {
        return GetLine(m_iLine);
    }

    bool operator==(const LinesIterator& it) const {
        return m_iLine == it.m_iLine && &m_poly == &it.m_poly;
    }

    bool operator!=(const LinesIterator& it) const {
        return !(*this == it);
    }

    line operator++(int) {
        auto ret = GetLine(m_iLine);
        ++* this;
        return ret;
    }

    LinesIterator& operator++() {
        m_iLine++;
        return *this;
    }

    Line GetLine(int idx) const {
        return Line(m_poly.points[idx], m_poly.points[(idx+ 1) % m_poly.cnt]);
    }
};

template<typename T>
using lines_iterator [[deprecated]] = LinesIterator<T>;

struct Polygon {
    [[deprecated("Use Count()")]] int cnt;
    [[deprecated("Use indexing operator.")]] vector4 points[POLYGON_MAX_POINTS];

    Polygon() : cnt(0) {
    }

    Polygon& operator+=(const vector4& point) {
        if (cnt < POLYGON_MAX_POINTS) {
            points[cnt++] = point;
        }
        return *this;
    }

    vector4& operator[](int iVtx) {
        return points[iVtx % cnt];
    }

    vector4 operator[](int iVtx) const {
        return points[iVtx % cnt];
    }


    LinesIterator<Polygon> begin() const {
        return LinesIterator<Polygon>(*this);
    }

    LinesIterator<Polygon> end() const {
        return LinesIterator<Polygon>(*this, cnt);
    }

    // This is only defined for triangles!!!
    vector4 GetNormal() const {
        assert(cnt == 3);
        return cross(points[1] - points[0], points[1] - points[2]);
    }

    int Count() const {
        return cnt;
    }
};

using polygon [[deprecated]] = Polygon;

struct LineContainer {
    [[deprecated("Use Count()")]] int cnt = 0;
    [[deprecated("Use indexing operator")]] vector4 points[LINECONT_MAX_POINTS];

    LineContainer& operator+=(const vector4& point) {
        if (cnt < LINECONT_MAX_POINTS) {
            points[cnt++] = point;
        }
        return *this;
    }

    LineContainer& operator+=(const Line& line) {
        *this += line.p[0];
        *this += line.p[1];
        return *this;
    }

    int Count() const {
        return cnt;
    }

    const vector4& operator[](int iIdx) const {
        return points[iIdx];
    }

    vector4& operator[](int iIdx) {
        return points[iIdx];
    }
};

using line_container [[deprecated]] = LineContainer;

struct PolygonContainer {
    [[deprecated]] int cnt = 0;
    [[deprecated]] Polygon polygons[POLYCONT_MAX_POLYS];

    PolygonContainer()
        : cnt(0) {
    }

    PolygonContainer& operator+=(const polygon& poly) {
        if (cnt < POLYCONT_MAX_POLYS) {
            polygons[cnt++] = poly;
        }
        return *this;
    }

    int Count() const {
        return cnt;
    }

    const polygon& operator[](int iIdx) const {
        return polygons[iIdx];
    }

    polygon& operator[](int iIdx) {
        return polygons[iIdx];
    }
};

using polygon_container [[deprecated]] = PolygonContainer;

inline Plane PlaneFromPolygon(const Polygon& poly) {
    assert(poly.cnt >= 3);
    return Plane(poly.points[0], poly.points[1], poly.points[2]);
}

inline vector4 normal(const Plane& plane) {
    return cross(plane.p[2] - plane.p[0], plane.p[2] - plane.p[1]);
}
