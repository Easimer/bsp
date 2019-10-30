#include "util_intersection.h"

#define EPSILON (0.001f)

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

bool IntersectSegmentByPlane(line* pLineFront, line* pLineBack,
    vector4* pXP,
    const line& line, const float pPlaneCoefficients[4]) {
    bool ret = false;
    vector4 xp, vtxFront, vtxBack;
    float flDist0, flDist1;
    float d;
    auto& P0 = line.p[0];
    auto N = line.p[1] - line.p[0];
    const float& A = pPlaneCoefficients[0];
    const float& B = pPlaneCoefficients[1];
    const float& C = pPlaneCoefficients[2];
    const float& D = pPlaneCoefficients[3];

    flDist0 =
        A * line.p[0][0] +
        B * line.p[0][1] +
        C * line.p[0][2] +
        D;
    flDist1 =
        A * line.p[1][0] +
        B * line.p[1][1] +
        C * line.p[1][2] +
        D;

    printf("\t\tflDist: (%f, %f)\n", flDist0, flDist1);

    if ((flDist0 > EPSILON&& flDist1 < -EPSILON) ||
        (flDist1 > EPSILON&& flDist0 < -EPSILON)) {
        ret = true;
        d = -(D + A * P0[0] + B * P0[1] + C * P0[2]) /
            (A * N[0] + B * N[1] + C * N[2]);
        xp = P0 + d * N;

        if (flDist0 > EPSILON) {
            vtxFront = line.p[0];
            vtxBack = line.p[1];
        } else {
            vtxFront = line.p[1];
            vtxBack = line.p[0];
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

bool IntersectSegmentByPlane(line* pLineFront, line* pLineBack,
    vector4* pXP,
    const line& line, const plane& P) {
    float co[4];
    EquationOfPlane(&co[0], &co[1], &co[2], &co[3], P);

    return IntersectSegmentByPlane(pLineFront, pLineBack, pXP, line, co);
}
