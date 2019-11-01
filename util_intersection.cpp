#include "util_intersection.h"

#define EPSILON (0.001f)

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

bool IntersectSegmentByPlane(Line* pLineFront, Line* pLineBack,
    vector4* pXP,
    const Line& line, const float pPlaneCoefficients[4]) {
    bool ret = false;
    vector4 xp, vtxFront, vtxBack;
    float flDist0, flDist1;
    float d;
    auto& P0 = line[0];
    auto N = line[1] - line[0];
    const float& A = pPlaneCoefficients[0];
    const float& B = pPlaneCoefficients[1];
    const float& C = pPlaneCoefficients[2];
    const float& D = pPlaneCoefficients[3];

    flDist0 =
        A * line[0][0] +
        B * line[0][1] +
        C * line[0][2] +
        D;
    flDist1 =
        A * line[1][0] +
        B * line[1][1] +
        C * line[1][2] +
        D;

    if ((flDist0 > EPSILON&& flDist1 < -EPSILON) ||
        (flDist1 > EPSILON&& flDist0 < -EPSILON)) {
        ret = true;
        d = -(D + A * P0[0] + B * P0[1] + C * P0[2]) /
            (A * N[0] + B * N[1] + C * N[2]);
        xp = P0 + d * N;

        if (flDist0 > EPSILON) {
            vtxFront = line[0];
            vtxBack = line[1];
        } else {
            vtxFront = line[1];
            vtxBack = line[0];
        }

        if (pLineFront) {
            *pLineFront = { vtxFront, xp };
        }
        if (pLineBack) {
            *pLineBack = { xp, vtxBack };
        }

        if (pXP) {
            *pXP = xp;
        }
    }

    return ret;
}

bool IntersectSegmentByPlane(Line* pLineFront, Line* pLineBack,
    vector4* pXP,
    const Line& line, const Plane& P) {
    float co[4];
    EquationOfPlane(&co[0], &co[1], &co[2], &co[3], P);

    return IntersectSegmentByPlane(pLineFront, pLineBack, pXP, line, co);
}
