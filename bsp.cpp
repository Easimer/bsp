#include <assert.h>
#include "bsp.h"
#include "util_vector.h"
#include "poly_part.h"

int WhichSide(const Plane& plane, const vector4& point) {
    auto normal = cross(plane[2] - plane[0], plane[1] - plane[0]);

    auto D = dot(plane[0] - point, normal);

    if (D > 0) {
        return SIDE_FRONT;
    } else if (D < 0) {
        return SIDE_BACK;
    } else {
        return SIDE_ON;
    }
}

bool PlaneLineIntersection(vector4* res, const Line& line, const Plane& plane) {
    bool ret = false;

    assert(res);

    auto N = normal(plane);
    auto p0 = plane[0];

    auto l0 = line[0];
    auto l = line[1] - line[0];


    float dotLN = dot(l, N);

    if (abs(dotLN) > 0.001) {
        ret = true;
        float d = dot((p0 - l0), N) / dotLN;
        *res = d * l + l0;
    }

    return ret;
}

// res0 is the line in front of the splitter, res1 is the line back of the splitter
bool SplitLine(Line* res0, Line* res1, vector4* xp, const Line& splitted, const Plane& splitter) {
    bool ret = false;

    assert(res0);
    assert(res1);

    vector4 sp;

    if (PlaneLineIntersection(&sp, splitted, splitter)) {
        Line line0(splitted[0], sp);
        Line line1(splitted[1], sp);

        if (line0.Length() > 0.01 && line1.Length() > 0.01) {
            if (WhichSide(splitter, splitted[0]) == SIDE_FRONT) {
                *res0 = line0;
                *res1 = line1;
            } else {
                *res0 = line1;
                *res1 = line0;

            }
            *xp = sp;
            ret = true;
        } else {
        }
    }

    return ret;
}

int HasPoint(const Polygon& poly, const vector4& point) {
    int ret = -1;
    for (int i = 0; i < poly.Count(); i++) {
        if (poly[i] == point) {
            ret = i;
        }
    }
    return ret;
}

Polygon FromLines(const LineContainer& lc) {
    struct Connection {
        int idx0 = -1, idx1 = -1;
        bool visited = false;
    };
    Polygon ret, temp;
    Connection conns[LINECONT_MAX_POINTS];
    int connCount = 0;
    Connection connsCulled[LINECONT_MAX_POINTS];
    int connCulledCount = 0;


    // 1. build pseudo-polygon from the line set
    for (int i = 0; i < lc.Count() / 2; i++) {
        auto& point0 = lc[i * 2 + 0];
        auto& point1 = lc[i * 2 + 1];
        int idx0 = HasPoint(temp, point0);
        int idx1 = HasPoint(temp, point1);
        if (idx0 == -1) {
            idx0 = temp.Count();
            temp += point0;
        }
        if (idx1 == -1) {
            idx1 = temp.Count();
            temp += point1;
        }
        conns[connCount++] = { idx0, idx1 };
    }

    // 2. Make a cycle around the polygon
    int vertexIdx = 0;

    do {
        ret += temp[vertexIdx];

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

bool SplitPolygon(Polygon* res0, Polygon* res1, const Polygon& splitted, const Plane& splitter) {
    bool ret = false;
    int splitCount = 0;

    assert(res0);
    assert(res1);

    LineContainer front, back;
    // The two intersection points
    vector4 xp[2];

    for (auto pline : splitted) {
        Line l0, l1;

        if (SplitLine(&l0, &l1, &xp[splitCount], pline, splitter)) {
            assert(splitCount < 2);
            front += l0[0];
            front += l0[1];
            back += l1[0];
            back += l1[1];
            ret |= true;
            splitCount++;
        } else {
            int side0 = WhichSide(splitter, pline[0]);
            int side1 = WhichSide(splitter, pline[1]);
            if (side0 != side1) {
                if (side0 == SIDE_ON && side1 != SIDE_ON) {
                    side0 = side1;
                } else if (side0 != SIDE_ON && side1 == SIDE_ON) {
                    side1 = side0;
                } else {
                    assert(0);
                }
            }
            assert(side0 == side1);
            if (side0 >= SIDE_ON) {
                front += pline[0];
                front += pline[1];
            }
            if (side0 <= SIDE_ON) {
                back += pline[0];
                back += pline[1];
            }
        }
    }

    if (ret) {
        front += xp[0];
        front += xp[1];
        back += xp[0];
        back += xp[1];

        *res0 = FromLines(front);
        *res1 = FromLines(back);
    }

    return ret;
}

PolygonContainer FanTriangulate(const Polygon& poly) {
    PolygonContainer ret;

    assert(poly.Count() >= 3);

    for (int vertexIdx = 1; vertexIdx < poly.Count() - 2; vertexIdx++) {
        Polygon tri;
        tri += poly[0];
        tri += poly[vertexIdx];
        tri += poly[vertexIdx + 1];
        ret += tri;
    }

    Polygon last;
    last += poly[0];
    last += poly[poly.Count() - 2];
    last += poly[poly.Count() - 1];
    ret += last;

    return ret;
}

static bsp_node* BuildBSPTree(bsp_node* pNode) {
    bsp_node* pRet = NULL;

    if (pNode) {
        bsp_node* tmp = new bsp_node;
        PolygonContainer*pcFront, *pcBack;
        auto& polyRoot = pNode->list[0];
        auto planeRoot = PlaneFromPolygon(polyRoot);
        pRet = new bsp_node;
        pRet->list += polyRoot;


        pcFront = new PolygonContainer;
        pcBack = new PolygonContainer;

        for (int iPoly = 1; iPoly < pNode->list.Count(); iPoly++) {
            Polygon *polyFront, *polyBack;
            polyFront = new Polygon;
            polyBack = new Polygon;
            auto& splitted = pNode->list[iPoly];
            if (SplitPolygon2(polyFront, polyBack, splitted, planeRoot)) {
                (*pcFront) += *polyFront;
                (*pcBack) += *polyBack;
            } else {
                auto side = WhichSide(planeRoot, pNode->list[iPoly][0]);
                switch (side) {
                case SIDE_FRONT:
                    (*pcFront) += pNode->list[iPoly];
                    break;
                case SIDE_BACK:
                    (*pcBack) += pNode->list[iPoly];
                    break;
                case SIDE_ON:
                    pRet->list += pNode->list[iPoly];
                    break;
                }

            }
            delete polyFront; delete polyBack;
        }

        if (pcFront->Count() > 0) {
            tmp->list = *pcFront;
            pRet->front = BuildBSPTree(tmp);
        }
        if (pcBack->Count() > 0) {
            tmp->list = *pcBack;
            pRet->back = BuildBSPTree(tmp);
        }
        delete tmp;
        delete pcFront; delete pcBack;
    }

    return pRet;
}

bsp_node* BuildBSPTree(const PolygonContainer& pc) {
    bsp_node* ret = NULL;

    bsp_node* pRoot = new bsp_node;
    pRoot->list = pc;

    ret = BuildBSPTree(pRoot);

    delete pRoot;

    return ret;
}