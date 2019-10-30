#pragma once

#include "util_vector.h"
#include <cstddef>
#include <iterator>
#include <assert.h>

#define POLYGON_MAX_POINTS (16)

struct plane {
    plane() {}
    plane(vector4 p0, vector4 p1, vector4 p2)
        : p{ p0, p1, p2 } {
    }

    vector4 p[3];
};

inline vector4 normal(const plane& plane) {
    return cross(plane.p[2] - plane.p[0], plane.p[2] - plane.p[1]);
}

struct line {
    line() {}
    line(const vector4& p0, const vector4& p1) : p{ p0, p1 } {
    }
    vector4 p[2];

    float length() const {
        return (p[1] - p[0]).length();
    }
};

template<typename T>
struct lines_iterator {
    explicit lines_iterator(const T& poly, int idx = 0)
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
        return line(m_poly.points[idx], m_poly.points[(idx+ 1) % m_poly.cnt]);
    }
};

struct polygon {
    int cnt;
    vector4 points[POLYGON_MAX_POINTS];

    polygon() : cnt(0) {
    }

    polygon& operator+=(const vector4& point) {
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


    lines_iterator<polygon> begin() const {
        return lines_iterator<polygon>(*this);
    }

    lines_iterator<polygon> end() const {
        return lines_iterator<polygon>(*this, cnt);
    }
};

#define LINECONT_MAX_POINTS (POLYGON_MAX_POINTS * 2 - 1)

struct line_container {
    int cnt = 0;
    vector4 points[LINECONT_MAX_POINTS];

    line_container& operator+=(const vector4& point) {
        if (cnt < LINECONT_MAX_POINTS) {
            points[cnt++] = point;
        }
        return *this;
    }

    line_container& operator+=(const line& line) {
        *this += line.p[0];
        *this += line.p[1];
        return *this;
    }
};

#define POLYCONT_MAX_POLYS ((POLYGON_MAX_POINTS - 2) * 3)

struct polygon_container {
    int cnt = 0;
    polygon polygons[POLYCONT_MAX_POLYS];

    polygon_container()
        : cnt(0) {
    }

    polygon_container& operator+=(const polygon& poly) {
        if (cnt < POLYCONT_MAX_POLYS) {
            polygons[cnt++] = poly;
        }
        return *this;
    }
};

struct bsp_node {
public:
    polygon_container list;
    bsp_node* front;
    bsp_node* back;

    bsp_node() :
        front(NULL), back(NULL) {
    }
};

#define SIDE_FRONT (1)
#define SIDE_ON (0)
#define SIDE_BACK (-1)

int WhichSide(const plane& plane, const vector4& point);
//bool SplitPolygon(polygon* res0, polygon* res1, const polygon& splitted, const plane& splitter);
bool SplitPolygon2(polygon* res0, polygon* res1, const polygon& splitted, const plane& splitter);
polygon_container FanTriangulate(const polygon& poly);
bsp_node* BuildBSPTree(const polygon_container& pc);
bool SplitLine(line* res0, line* res1, vector4* xp, const line& splitted, const plane& splitter);
polygon FromLines(const line_container& lc);
bool PlaneLineIntersection(vector4* res, const line& line, const plane& plane);

inline plane PlaneFromPolygon(const polygon& poly) {
    assert(poly.cnt >= 3);
    return plane(poly.points[0], poly.points[1], poly.points[2]);
}