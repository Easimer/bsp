#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "bsp.h"
#include "util_vector.h"
#include "poly_part.h"

std::ostream& operator<<(std::ostream& os, const vector4& point) {
    return (os << "(" << point[0] << ", " << point[1] << ", " << point[2] << ")");
}

std::ostream& operator<<(std::ostream& os, const polygon& poly) {
    os << "(poly ";
    for (int i = 0; i < poly.cnt; i++) {
        os << poly.points[i] << ", ";
    }
    os << ")";

    return os;
}

std::ostream& operator<<(std::ostream& os, const plane& plane) {
    os << "(plane ";
    for (int i = 0; i < 3; i++) {
        os << plane.p[i] << ", ";
    }
    os << ")";

    return os;
}

std::ostream& operator<<(std::ostream& os, const line& line) {
    os << "(line " << line.p[0] << ", " << line.p[1] << ")";
    return os;
}

int WhichSide(const plane& plane, const vector4& point) {
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

bool PlaneLineIntersection(vector4* res, const line& line, const plane& plane) {
    bool ret = false;

    assert(res);

    std::cout << "\t\t\t\t\tIntersecting\n" <<
        "\t\t\t\t\t" << line << " by " << plane;

    auto N = normal(plane);
    auto p0 = plane.p[0];

    auto l0 = line.p[0];
    auto l = line.p[1] - line.p[0];


    float dotLN = dot(l, N);

    if (abs(dotLN) > 0.001) {
        ret = true;
        float d = dot((p0 - l0), N) / dotLN;
        *res = d * l + l0;
        std::cout << "\t\t\t\t\t\tFound xp=" << *res << '\n';
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

        if (line0.length() > 0.01 && line1.length() > 0.01) {
            if (WhichSide(splitter, splitted.p[0]) == SIDE_FRONT) {
                *res0 = line0;
                *res1 = line1;
            } else {
                *res0 = line1;
                *res1 = line0;

            }
            *xp = sp;
            ret = true;
        } else {
            std::cout << "\t\t\t\tProduced degenerate line while splitting lines\n";
        }
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

    std::cout << "Splitting " << splitted << " by " << splitter << '\n';

    for (auto pline : splitted) {
        line l0, l1;

        std::cout << "\tSplitting" << pline << '\n';
        if (SplitLine(&l0, &l1, &xp[splitCount], pline, splitter)) {
            std::cout << "\t\tFound intersection front:" << l0 << ", back:" << l1 << '\n';
            assert(splitCount < 2);
            front += l0.p[0];
            front += l0.p[1];
            back += l1.p[0];
            back += l1.p[1];
            ret |= true;
            splitCount++;
        } else {
            std::cout << "\t\tDidn't found intersection\n";
            int side0 = WhichSide(splitter, pline.p[0]);
            int side1 = WhichSide(splitter, pline.p[1]);
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
                std::cout << "\t\t\tIn front \n";
                front += pline.p[0];
                front += pline.p[1];
            }
            if (side0 <= SIDE_ON) {
                std::cout << "\t\t\tIn back\n";
                back += pline.p[0];
                back += pline.p[1];
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

polygon_container FanTriangulate(const polygon& poly) {
    polygon_container ret;

    assert(poly.cnt >= 3);

    for (int vertexIdx = 1; vertexIdx < poly.cnt - 2; vertexIdx++) {
        polygon tri;
        tri += poly.points[0];
        tri += poly.points[vertexIdx];
        tri += poly.points[vertexIdx + 1];
        ret += tri;
    }

    polygon last;
    last += poly.points[0];
    last += poly.points[poly.cnt - 2];
    last += poly.points[poly.cnt - 1];
    ret += last;

    return ret;
}

static bsp_node* BuildBSPTree2(bsp_node* pNode) {
    bsp_node* pRet = NULL;

    if (pNode) {
        bsp_node* tmp = new bsp_node;
        polygon_container *pcFront, *pcBack;
        auto& polyRoot = pNode->list.polygons[0];
        auto planeRoot = PlaneFromPolygon(polyRoot);
        pRet = new bsp_node;
        pRet->list += polyRoot;


        pcFront = new polygon_container;
        pcBack = new polygon_container;

        for (int iPoly = 1; iPoly < pNode->list.cnt; iPoly++) {
            polygon *polyFront, *polyBack;
            polyFront = new polygon;
            polyBack = new polygon;
            auto& splitted = pNode->list.polygons[iPoly];
            if (SplitPolygon2(polyFront, polyBack, splitted, planeRoot)) {
                (*pcFront) += *polyFront;
                (*pcBack) += *polyBack;
            } else {
                auto side = WhichSide(planeRoot, pNode->list.polygons[iPoly].points[0]);
                switch (side) {
                case SIDE_FRONT:
                    (*pcFront) += pNode->list.polygons[iPoly];
                    break;
                case SIDE_BACK:
                    (*pcBack) += pNode->list.polygons[iPoly];
                    break;
                case SIDE_ON:
                    pRet->list += pNode->list.polygons[iPoly];
                    break;
                }

            }
            delete polyFront; delete polyBack;
        }

        if (pcFront->cnt > 0) {
            tmp->list = *pcFront;
            pRet->front = BuildBSPTree2(tmp);
        }
        if (pcBack->cnt > 0) {
            tmp->list = *pcBack;
            pRet->back = BuildBSPTree2(tmp);
        }
        delete tmp;
        delete pcFront; delete pcBack;
    }

    return pRet;
}

static bsp_node* BuildBSPTree(bsp_node* pNode) {
    bsp_node* ret = NULL;
    if (pNode) {
        if (pNode->list.cnt != 0) {
            if (pNode->list.cnt > 1) {
                bsp_node* listFront, * listBack;
                bsp_node* node;
                auto poly = pNode->list.polygons[0];
                plane polyPlane(poly.points[0], poly.points[1], poly.points[2]);

                listFront = new bsp_node;
                listBack = new bsp_node;
                node = new bsp_node;

                for (int iPoly = 1; iPoly < pNode->list.cnt; iPoly++) {
                    polygon front, back;
                    if(SplitPolygon2(&front, &back, pNode->list.polygons[iPoly], polyPlane)) {
                        listFront->list += front;
                        listBack->list += back;
                    } else {
                        // Plane does not split polygon
                        // Determine which side it lies on
                        auto side = WhichSide(polyPlane, pNode->list.polygons[iPoly].points[0]);
                        switch (side) {
                        case SIDE_FRONT:
                            listFront->list += pNode->list.polygons[iPoly];
                            break;
                        case SIDE_BACK:
                            listBack->list += pNode->list.polygons[iPoly];
                            break;
                        case SIDE_ON:
                            node->list += pNode->list.polygons[iPoly];
                            break;
                        }
                    }
                }
                node->list += poly;

                node->front = BuildBSPTree(listFront);
                node->back = BuildBSPTree(listBack);
                ret = node;
            } else {
                auto leaf = new bsp_node;
                leaf->list += pNode->list.polygons[0];
                ret = leaf;
            }
        }
    }
    return ret;
}

bsp_node* BuildBSPTree(const polygon_container& pc) {
    bsp_node* ret = NULL;

    bsp_node* pRoot = new bsp_node;
    pRoot->list = pc;

    ret = BuildBSPTree2(pRoot);

    delete pRoot;

    return ret;
}