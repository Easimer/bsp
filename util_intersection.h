#pragma once

#include "bsp.h"

bool IntersectSegmentByPlane(line* pLineFront, line* pLineBack,
    vector4* pXP,
    const line& line, const plane& P);

bool IntersectSegmentByPlane(line* pLineFront, line* pLineBack,
    vector4* pXP,
    const line& line, const float aflPlaneCoefficients[4]);