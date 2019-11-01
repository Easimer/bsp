#pragma once

#include "util_vector.h"
#include <cstddef>
#include <iterator>
#include <assert.h>

#define POLYGON_MAX_POINTS (16)
#define LINECONT_MAX_POINTS (POLYGON_MAX_POINTS * 2 - 1)
#define POLYCONT_MAX_POLYS ((POLYGON_MAX_POINTS - 2) * 3)

class Plane {
public:
    Plane() {}
    Plane(vector4 p0, vector4 p1, vector4 p2)
        : p{ p0, p1, p2 } {
    }

    int Count() const {
        return 3;
    }

    const vector4& operator[](int iIdx) const {
        return p[iIdx];
    }

    vector4& operator[](int iIdx) {
        return p[iIdx];
    }

private:
    vector4 p[3];
};

struct Line {
    Line() {}
    Line(const vector4& p0, const vector4& p1) : p{ p0, p1 } {
    }

    float Length() const {
        return (p[1] - p[0]).length();
    }

    int Count() const {
        return 2;
    }

    const vector4& operator[](int iIdx) const {
        return p[iIdx];
    }

    vector4& operator[](int iIdx) {
        return p[iIdx];
    }

private:
    vector4 p[2];
};

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

    Line operator*() const {
        return GetLine(m_iLine);
    }

    bool operator==(const LinesIterator& it) const {
        return m_iLine == it.m_iLine && &m_poly == &it.m_poly;
    }

    bool operator!=(const LinesIterator& it) const {
        return !(*this == it);
    }

    Line operator++(int) {
        auto ret = GetLine(m_iLine);
        ++* this;
        return ret;
    }

    LinesIterator& operator++() {
        m_iLine++;
        return *this;
    }

    Line GetLine(int idx) const {
        return Line(m_poly[idx], m_poly[(idx+ 1) % m_poly.Count()]);
    }
};

struct Polygon {
public:
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

private:
    int cnt;
    vector4 points[POLYGON_MAX_POINTS];

};

class LineContainer {
public:
    LineContainer& operator+=(const vector4& point) {
        if (cnt < LINECONT_MAX_POINTS) {
            points[cnt++] = point;
        }
        return *this;
    }

    LineContainer& operator+=(const Line& line) {
        *this += line[0];
        *this += line[1];
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
private:
    int cnt = 0;
    vector4 points[LINECONT_MAX_POINTS];

};

class PolygonContainer {
public:
    PolygonContainer()
        : cnt(0) {
    }

    PolygonContainer& operator+=(const Polygon& poly) {
        if (cnt < POLYCONT_MAX_POLYS) {
            polygons[cnt++] = poly;
        }
        return *this;
    }

    int Count() const {
        return cnt;
    }

    const Polygon& operator[](int iIdx) const {
        return polygons[iIdx];
    }

    Polygon& operator[](int iIdx) {
        return polygons[iIdx];
    }

    const Polygon& GetPolygon(int iIdx) const {
        return polygons[iIdx];
    }

    Polygon& GetPolygon(int iIdx) {
        return polygons[iIdx];
    }

private:
    int cnt = 0;
    Polygon polygons[POLYCONT_MAX_POLYS];
};

inline Plane PlaneFromPolygon(const Polygon& poly) {
    assert(poly.Count() >= 3);
    return Plane(poly[0], poly[1], poly[2]);
}

inline vector4 normal(const Plane& plane) {
    return cross(plane[2] - plane[0], plane[2] - plane[1]);
}
