#include <stdio.h>
#include <assert.h>
#include "bsp.h"
#include "util_vector.h"

static int WhichSide(const plane& plane, const vector4& point) {
    auto normal = cross(plane.p[2] - plane.p[0], plane.p[1] - plane.p[0]);

    auto D = dot(plane.p[0] - point, normal);

    if (D > 0) {
        return SIDE_FRONT;
    } else if (D < 0) {
        return SIDE_BACK;
    } else {
        return SIDE_ON;
    }
}

static bool PlaneLineIntersection(vector4* res, const line& line, const plane& plane) {
    bool ret = false;

    assert(res);

    auto N = normal(plane);
    auto p0 = plane.p[0];

    auto l0 = line.p[0];
    auto l = line.p[1] - line.p[0];


    float dotLN = dot(l, N);

    if (dotLN != 0) {
        ret = true;
        float d = dot((p0 - l0), N) / dotLN;
        *res = d * l + l0;
    }

    return ret;
}

// res0 is the line in front of the splitter, res1 is the line back of the splitter
bool SplitLine(line* res0, line* res1, vector4* xp, const line& splitted, const plane& splitter) {
    bool ret = false;

    assert(res0);
    assert(res1);

    vector4 sp;

    if (PlaneLineIntersection(&sp, splitted, splitter)) {
        line line0(splitted.p[0], sp);
        line line1(splitted.p[1], sp);
        if (WhichSide(splitter, splitted.p[0]) == SIDE_FRONT) {
            *res0 = line0;
            *res1 = line1;
        } else {
            *res0 = line1;
            *res1 = line0;

        }
        *xp = sp;
        ret = true;
    }

    return ret;
}

int HasPoint(const polygon& poly, const vector4& point) {
    int ret = -1;
    for (int i = 0; i < poly.cnt; i++) {
        if (poly.points[i] == point) {
            ret = i;
        }
    }
    return ret;
}

polygon FromLines(const line_container& lc) {
    struct Connection {
        int idx0, idx1;
        bool visited = false;
    };
    polygon ret, temp;
    Connection conns[LINECONT_MAX_POINTS];
    int connCount = 0;
    Connection connsCulled[LINECONT_MAX_POINTS];
    int connCulledCount = 0;


    // 1. build pseudo-polygon from the line set
    for (int i = 0; i < lc.cnt / 2; i++) {
        auto& point0 = lc.points[i * 2 + 0];
        auto& point1 = lc.points[i * 2 + 1];
        int idx0 = HasPoint(temp, point0);
        int idx1 = HasPoint(temp, point1);
        if (idx0 == -1) {
            idx0 = temp.cnt;
            temp += point0;
        }
        if (idx1 == -1) {
            idx1 = temp.cnt;
            temp += point1;
        }
        conns[connCount++] = { idx0, idx1 };
    }

    // 2. Cull redundant connections
    /*
    for (int connIdx0 = 0; connIdx0 < connCount; connIdx0++) {
        bool unique = true;
        auto& conn0 = conns[connIdx0];
        for (int connIdx1 = 0; connIdx1 < connCulledCount && unique; connIdx1++) {
            if (connIdx0 != connIdx1) {
                auto& conn1 = connsCulled[connIdx1];
                if (conn0.idx0 == conn1.idx1 && conn0.idx1 == conn1.idx0) {
                    unique = false;
                }
            }
        }
        if (unique) {
            connsCulled[connCulledCount++] = conn0;
        }
    }
    */

    // 2. Make a cycle around the polygon
    int vertexIdx = 0;

    do {
        ret += temp.points[vertexIdx];

        // What is the next connected vertex
        Connection* conn = NULL;
        for (int connIdx = 0; connIdx < connCount && !conn; connIdx++) {
            if (!conns[connIdx].visited) {
                if (conns[connIdx].idx0 == vertexIdx) {
                    conn = &conns[connIdx];
                    vertexIdx = conn->idx1;
                } else if (conns[connIdx].idx1 == vertexIdx) {
                    conn = &conns[connIdx];
                    vertexIdx = conn->idx0;
                }
            }
        }
        assert(conn);
        conn->visited = true;
    } while (vertexIdx != 0);

    return ret;
}

bool SplitPolygon(polygon* res0, polygon* res1, const polygon& splitted, const plane& splitter) {
    bool ret = false;
    int splitCount = 0;

    assert(res0);
    assert(res1);

    line_container front, back;
    // The two intersection points
    vector4 xp[2];

    for (auto pline : splitted) {
        line l0, l1;
        if (SplitLine(&l0, &l1, &xp[splitCount], pline, splitter)) {
            assert(splitCount < 2);
            front += l0.p[0];
            front += l0.p[1];
            back += l1.p[0];
            back += l1.p[1];
            ret |= true;
            splitCount++;
        } else {
            int side0 = WhichSide(splitter, pline.p[0]);
            int side1 = WhichSide(splitter, pline.p[1]);
            assert(side0 == side1);
            if (side0 >= SIDE_ON) {
                front += pline.p[0];
                front += pline.p[1];
            }
            if (side0 <= SIDE_ON) {
                back += pline.p[0];
                back += pline.p[1];
            }
        }
    }

    front += xp[0];
    front += xp[1];
    back += xp[0];
    back += xp[1];

    *res0 = FromLines(front);
    *res1 = FromLines(back);

    return ret;
}