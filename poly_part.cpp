#include "bsp.h"
#include "poly_part.h"
#include "util_intersection.h"

#define EPSILON (0.01f)

static void EquationOfPlane(float* A, float* B, float* C, float* D, const Plane& P) {
    // Graphics Gems 3 - Newell's Method
    float a = 0, b = 0, c = 0;
    float d = 0;
    vector4 avg, N;

    for (int i = 0; i < 3; i++) {
        a +=
            (P[i][1] - P[(i + 1) % 3][1]) *
            (P[i][2] - P[(i + 1) % 3][2]);
        b +=
            (P[i][2] - P[(i + 1) % 3][2]) *
            (P[i][0] - P[(i + 1) % 3][0]);
        c +=
            (P[i][0] - P[(i + 1) % 3][0]) *
            (P[i][1] - P[(i + 1) % 3][1]);
        avg[0] += P[i][0];
        avg[1] += P[i][1];
        avg[2] += P[i][2];
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

static float SignedDistanceFromPlane(const vector4& P, const Plane& J) {
    const auto Jn = normal(J);
    const auto Jd = -dot(Jn, J[0]);

    return dot(Jn, P) + Jd;
}

#define DISTSIGN(d) \
((d > 0) ? (int)1 : \
((d < 0) ? (int)-1 : (int)0))

bool PartitionPolygonByPlane(Polygon* pFront, Polygon* pBack, const Polygon& poly, const Plane& plane) {
    bool ret = false;
    Polygon front, back;
    vector4 xp[2];
    int xpi = 0;
    LineContainer lc0, lc1;
    Line l0, l1;

    for(auto it = poly.begin(); it != poly.end(); ++it) {
        bool bIntersection = false;
        auto it2 = it;
        auto& edge = *it;
        auto& nextEdge = *(++it2);
        auto& V1 = edge[0];
        auto& V2 = edge[1];
        auto& V3 = nextEdge[1];

        assert(V2 != V3);
        const int dist0 = DISTSIGN(SignedDistanceFromPlane(edge[0], plane));
        const int dist1 = DISTSIGN(SignedDistanceFromPlane(edge[1], plane));

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
                const int dist2 = DISTSIGN(SignedDistanceFromPlane(nextEdge[1], plane));
                if (dist2 != 0) {
                    // Case 8b
                    if (dist0 == 1) {
                        if (dist2 == -1) {
                            bIntersection = true;
                            l0 = { edge[0], edge[1] };
                            l1 = { edge[1], nextEdge[1] };
                            lc0 += l0;
                            lc1 += l1;
                        }
                    }
                    // Case 9b
                    else if (dist0 == -1) {
                        if (dist2 == 1) {
                            bIntersection = true;
                            l0 = { edge[0], edge[1] };
                            l1 = { edge[1], nextEdge[1] };
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

bool SplitPolygon2(Polygon* pFront, Polygon* pBack, const Polygon& poly, const Plane& P) {
    bool ret = false;
    int iVtx;
    int iVtx0Class;
    LineContainer front, back;
    LineContainer* cur = NULL;
    vector4 xp[2];
    int iXP = 0;

    assert(pFront && pBack);

    iVtx0Class = DISTSIGN(SignedDistanceFromPlane(poly[0], P));

    switch (iVtx0Class) {
    case 1:
    case 0:
        cur = &front;
        break;
    case -1:
        cur = &back;
        break;
    }

    for (iVtx = 0; iVtx < poly.Count() + 1; iVtx++) {
        auto P0 = poly[iVtx];
        auto P1 = poly[iVtx + 1];
        //if (PlaneLineIntersection(&xp[iXP], { P0, P1 }, P)) {
        if (iXP < 2 && IntersectSegmentByPlane(NULL, NULL, &xp[iXP], { P0, P1 }, P)) {
            assert(iXP < 2);
            auto l0 = Line { P0, xp[iXP] };
            auto l1 = Line { xp[iXP], P1 };
            (*cur) += P0;
            (*cur) += xp[iXP];
            cur = (cur == &front) ? &back : &front;
            (*cur) += xp[iXP];
            (*cur) += P1;
            iXP++;
            assert(iXP <= 2);
        } else {
            assert(iXP <= 2);
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
