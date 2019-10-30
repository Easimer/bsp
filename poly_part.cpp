#include "bsp.h"
#include "poly_part.h"
#include <iostream>
#include "util_intersection.h"

#define EPSILON (0.01f)

static void PrintLine(const line& l) {
    printf("[(%f, %f, %f) -> (%f, %f, %f)]",
        l.p[0][0], l.p[0][1], l.p[0][2],
        l.p[1][0], l.p[1][1], l.p[1][2]);
}

static void PrintTwoLines(const char* prefix, const line& l0, const line& l1) {
    printf("%s:\n\t", prefix);
    PrintLine(l0);
    printf("\t"); PrintLine(l1);
    printf("\n");
}

static void EquationOfPlane(float* A, float* B, float* C, float* D, const plane& P) {
    // Graphics Gems 3 - Newell's Method
    float a = 0, b = 0, c = 0;
    float d = 0;
    vector4 avg, N;

    for (int i = 0; i < 3; i++) {
        a +=
            (P.p[i][1] - P.p[(i + 1) % 3][1]) *
            (P.p[i][2] - P.p[(i + 1) % 3][2]);
        b +=
            (P.p[i][2] - P.p[(i + 1) % 3][2]) *
            (P.p[i][0] - P.p[(i + 1) % 3][0]);
        c +=
            (P.p[i][0] - P.p[(i + 1) % 3][0]) *
            (P.p[i][1] - P.p[(i + 1) % 3][1]);
        avg[0] += P.p[i][0];
        avg[1] += P.p[i][1];
        avg[2] += P.p[i][2];
    }

    N = { a, b, c };

    avg = avg / 3.0f;

    d = dot(-1 * avg, N);
    float lenN = N.length();

    *A = a / lenN;
    *B = b / lenN;
    *C = c / lenN;
    *D = d / lenN;
}

static float SignedDistanceFromPlane(const vector4& P, const plane& J) {
    const auto Jn = normal(J);
    const auto Jd = -dot(Jn, J.p[0]);

    return dot(Jn, P) + Jd;
}

#define DISTSIGN(d) \
((d > 0) ? (int)1 : \
((d < 0) ? (int)-1 : (int)0))

bool PartitionPolygonByPlane(polygon* pFront, polygon* pBack, const polygon& poly, const plane& plane) {
    bool ret = false;
    polygon front, back;
    vector4 xp[2];
    int xpi = 0;
    line_container lc0, lc1;
    line l0, l1;

    for(auto it = poly.begin(); it != poly.end(); ++it) {
        bool bIntersection = false;
        auto it2 = it;
        auto& edge = *it;
        auto& nextEdge = *(++it2);
        auto& V1 = edge.p[0];
        auto& V2 = edge.p[1];
        auto& V3 = nextEdge.p[1];

        assert(V2 != V3);
        const int dist0 = DISTSIGN(SignedDistanceFromPlane(edge.p[0], plane));
        const int dist1 = DISTSIGN(SignedDistanceFromPlane(edge.p[1], plane));

        // Is there an intersection?
        if (dist0 != dist1) {
            // Case 4
            if (dist0 == -1 && dist1 == 1) {
                bIntersection = true;
                assert(xpi < 2);
                SplitLine(&l0, &l1, &xp[xpi++], edge, plane);
                lc0 += l0;
                lc1 += l1;
            }
            // Case 5
            else if (dist1 == -1 && dist0 == 1) {
                bIntersection = true;
                assert(xpi < 2);
                SplitLine(&l0, &l1, &xp[xpi++], edge, plane);
                lc0 += l0;
                lc1 += l1;
            }
            else if (dist1 == 0) {
                const int dist2 = DISTSIGN(SignedDistanceFromPlane(nextEdge.p[1], plane));
                if (dist2 != 0) {
                    // Case 8b
                    if (dist0 == 1) {
                        if (dist2 == -1) {
                            bIntersection = true;
                            l0 = { edge.p[0], edge.p[1] };
                            l1 = { edge.p[1], nextEdge.p[1] };
                            lc0 += l0;
                            lc1 += l1;
                        }
                    }
                    // Case 9b
                    else if (dist0 == -1) {
                        if (dist2 == 1) {
                            bIntersection = true;
                            l0 = { edge.p[0], edge.p[1] };
                            l1 = { edge.p[1], nextEdge.p[1] };
                            lc0 += l0;
                            lc1 += l1;
                        }
                    }
                } else {
                    // Case 8a
                    if (dist0 == 1) {
                        lc0 += edge;
                    }
                    // Case 9a
                    else if (dist0 == -1) {
                        lc1 += edge;
                    }
                }
            }
        } else {
            lc0 += edge;
            lc1 += edge;
        }
        ret |= bIntersection;
    }

    *pFront = FromLines(lc0);
    *pBack = FromLines(lc1);

    return ret;
}

bool SplitPolygon2(polygon* pFront, polygon* pBack, const polygon& poly, const plane& P) {
    bool ret = false;
    int iVtx;
    int iVtx0Class;
    line_container front, back;
    line_container* cur = NULL;
    vector4 xp[2];
    int iXP = 0;

    assert(pFront && pBack);

    iVtx0Class = DISTSIGN(SignedDistanceFromPlane(poly.points[0], P));

    switch (iVtx0Class) {
    case 1:
    case 0:
        cur = &front;
        break;
    case -1:
        cur = &back;
        break;
    }

    printf("Splitting polygon [\n");
    for (int i = 0; i < poly.cnt; i++) {
        printf("\t(%f, %f, %f), ", poly.points[i][0], poly.points[i][1], poly.points[i][2]);
    }
    printf("\n] by plane [\n");
    for (int i = 0; i < 3; i++) {
        printf("\t(%f, %f, %f), ", P.p[i][0], P.p[i][1], P.p[i][2]);
    }
    printf("\n]\n");

    for (iVtx = 0; iVtx < poly.cnt + 1; iVtx++) {
        auto P0 = poly[iVtx];
        auto P1 = poly[iVtx + 1];
        //if (PlaneLineIntersection(&xp[iXP], { P0, P1 }, P)) {
        if (IntersectSegmentByPlane(NULL, NULL, &xp[iXP], { P0, P1 }, P)) {
            assert(iXP < 2);
            auto l0 = line { P0, xp[iXP] };
            auto l1 = line { xp[iXP], P1 };
            (*cur) += P0;
            (*cur) += xp[iXP];
            cur = (cur == &front) ? &back : &front;
            (*cur) += xp[iXP];
            (*cur) += P1;
            iXP++;
            PrintTwoLines("INTERSECTION! Added two new edges:", l0, l1);
        } else {
            (*cur) += P0;
            (*cur) += P1;
        }
    }

    if (iXP == 2) {
        cur = (cur == &front) ? &back : &front;
        (*cur) += xp[0];
        (*cur) += xp[1];
        cur = (cur == &front) ? &back : &front;
        (*cur) += xp[1];
        (*cur) += xp[0];

        *pFront = FromLines(front);
        *pBack = FromLines(back);

        ret = true;
    }

    return ret;
}
